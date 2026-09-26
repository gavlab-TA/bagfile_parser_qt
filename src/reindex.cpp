#include "bagfile_parser_qt/reindex.hpp"

#include <mcap/reader.hpp>
#include <mcap/writer.hpp>

#include <algorithm>
#include <cstdio>
#include <filesystem>
#include <set>
#include <sstream>
#include <unordered_map>
#include <vector>

namespace fs = std::filesystem;

static const uint64_t kMcapMagicSize = 8;

static void logMessage(const ConvertCallbacks& cbs, const std::string& msg)
{
    if (cbs.log)
    {
        cbs.log(msg);
    }
}

static std::string gigabytes(uint64_t bytes)
{
    char buf[32];
    std::snprintf(buf, sizeof(buf), "%.1f GB", static_cast<double>(bytes) / 1e9);
    return buf;
}

// The copy is always compressed, whatever the source used: rosbag2 often writes
// uncompressed chunks, and a zstd copy of those is several times smaller (a 14 GB
// recording came out at 2.9 GB).
static mcap::Compression copyCompression()
{
#ifndef MCAP_COMPRESSION_NO_ZSTD
    return mcap::Compression::Zstd;
#elif !defined(MCAP_COMPRESSION_NO_LZ4)
    return mcap::Compression::Lz4;
#else
    return mcap::Compression::None;
#endif
}

std::string defaultReindexDir(const std::string& bag_path)
{
    fs::path bag = fs::path(bag_path).lexically_normal();
    if (bag.filename().empty()) bag = bag.parent_path();   // trailing slash
    std::string name = fs::is_regular_file(bag) ? bag.stem().string() : bag.filename().string();
    return (bag.parent_path() / (name + "_reindexed")).string();
}

// Copies every readable record of `in` into a new, properly closed MCAP at `out`.
// Stops at the first unreadable record (normally the cut-off end of an interrupted
// recording) and keeps everything before it.
static bool rewriteFile(const fs::path& in, const fs::path& out, const ConvertCallbacks& cbs,
                        uint64_t& messages, bool& truncated, std::string& error)
{
    mcap::McapReader reader;
    mcap::Status status = reader.open(in.string());
    if (!status.ok())
    {
        error = "cannot open " + in.string() + ": " + status.message;
        return false;
    }
    mcap::IReadable& source = *reader.dataSource();
    const uint64_t size = source.size();
    const std::string name = in.filename().string();

    mcap::McapWriter writer;
    bool writer_open = false;
    mcap::Status write_status;
    std::string profile;
    std::unordered_map<mcap::SchemaId, mcap::SchemaId> schema_ids;
    std::unordered_map<mcap::ChannelId, mcap::ChannelId> channel_ids;
    uint64_t orphans = 0;
    uint64_t written = 0;

    // Opened on the first record that needs it, once the header's profile is known.
    auto ensureOpen = [&]() -> bool
    {
        if (writer_open) return true;
        if (!write_status.ok()) return false;
        mcap::McapWriterOptions opts(profile);
        opts.compression = copyCompression();
        write_status = writer.open(out.string(), opts);
        writer_open = write_status.ok();
        return writer_open;
    };

    mcap::TypedRecordReader records(source, kMcapMagicSize);
    records.onHeader = [&](const mcap::Header& h, mcap::ByteOffset)
    {
        profile = h.profile;
    };
    records.onSchema = [&](const mcap::SchemaPtr s, mcap::ByteOffset, std::optional<mcap::ByteOffset>)
    {
        if (schema_ids.count(s->id) || !ensureOpen()) return;
        mcap::Schema copy = *s;
        writer.addSchema(copy);
        schema_ids[s->id] = copy.id;
    };
    records.onChannel = [&](const mcap::ChannelPtr c, mcap::ByteOffset, std::optional<mcap::ByteOffset>)
    {
        if (channel_ids.count(c->id) || !ensureOpen()) return;
        mcap::Channel copy = *c;
        std::unordered_map<mcap::SchemaId, mcap::SchemaId>::const_iterator it = schema_ids.find(c->schemaId);
        copy.schemaId = (it != schema_ids.end()) ? it->second : 0;   // 0 = schemaless
        writer.addChannel(copy);
        channel_ids[c->id] = copy.id;
    };
    records.onMessage = [&](const mcap::Message& m, mcap::ByteOffset, std::optional<mcap::ByteOffset>)
    {
        std::unordered_map<mcap::ChannelId, mcap::ChannelId>::const_iterator it = channel_ids.find(m.channelId);
        if (it == channel_ids.end())
        {
            orphans++;
            return;
        }
        mcap::Message copy = m;
        copy.channelId = it->second;
        mcap::Status s = writer.write(copy);
        if (!s.ok())
        {
            if (write_status.ok()) write_status = s;
            return;
        }
        written++;
    };
    records.onMetadata = [&](const mcap::Metadata& md, mcap::ByteOffset)
    {
        if (!ensureOpen()) return;
        mcap::Status s = writer.write(md);
        if (!s.ok() && write_status.ok()) write_status = s;
    };
    records.onAttachment = [&](const mcap::Attachment& a, mcap::ByteOffset)
    {
        if (!ensureOpen()) return;
        mcap::Attachment copy = a;
        mcap::Status s = writer.write(copy);
        if (!s.ok() && write_status.ok()) write_status = s;
    };

    auto abandon = [&](const std::string& why)
    {
        error = name + ": " + why;
        if (writer_open) writer.close();
        std::error_code ec;
        fs::remove(out, ec);
        return false;
    };

    const uint64_t step = std::max<uint64_t>(size / 20, 1);
    uint64_t next_report = step;
    uint64_t good_offset = kMcapMagicSize;   // end of the last record read successfully
    while (records.next())
    {
        good_offset = records.offset();
        if (cbs.cancel && cbs.cancel->load()) return abandon("cancelled");
        if (!write_status.ok()) return abandon("write failed: " + write_status.message);
        if (good_offset >= next_report)
        {
            std::ostringstream os;
            os << name << ": " << (100 * good_offset / size) << "% ("
               << gigabytes(good_offset) << " / " << gigabytes(size) << ")";
            logMessage(cbs, os.str());
            next_report += step;
        }
    }
    if (!write_status.ok()) return abandon("write failed: " + write_status.message);
    if (!writer_open) return abandon("no readable records");

    const mcap::Status& end = records.status();
    truncated = !end.ok();
    if (truncated)
    {
        std::ostringstream os;
        os << name << ": readable up to " << gigabytes(good_offset) << " of " << gigabytes(size) << " ("
           << (100 * good_offset / size) << "%); kept everything before that. The rest is unreadable, "
           << "usually a recording that was cut off (" << end.message << ")";
        logMessage(cbs, os.str());
    }
    if (orphans > 0)
    {
        logMessage(cbs, name + ": skipped " + std::to_string(orphans) +
                        " message(s) whose channel record was never seen");
    }
    writer.close();
    messages += written;
    return true;
}

ReindexResult reindexBag(const std::string& bag_path, const std::string& out_dir, const ConvertCallbacks& cbs)
{
    ReindexResult result;
    result.output_dir = out_dir;

    std::vector<std::string> files = findMcapFiles(bag_path);
    if (files.empty())
    {
        result.error = "no .mcap files in " + bag_path;
        return result;
    }
    std::vector<std::string> missing_list = findMissingSummaries(bag_path);
    std::set<std::string> missing(missing_list.begin(), missing_list.end());

    std::error_code ec;
    fs::path out(out_dir);
    fs::path src_dir = fs::is_directory(bag_path) ? fs::path(bag_path) : fs::path(bag_path).parent_path();
    if (fs::exists(out) && fs::equivalent(out, src_dir, ec))
    {
        result.error = "the reindexed copy can't be written into the bag's own directory";
        return result;
    }
    fs::create_directories(out, ec);
    if (ec)
    {
        result.error = "cannot create " + out_dir + ": " + ec.message();
        return result;
    }

    // An earlier reindex already produced a complete, indexed copy: use it.
    bool complete = findMcapFiles(out_dir).size() == files.size() && findMissingSummaries(out_dir).empty();
    for (const std::string& f : files)
    {
        complete = complete && fs::exists(out / fs::path(f).filename());
    }
    if (complete)
    {
        logMessage(cbs, "Using the existing indexed copy in " + out_dir);
        result.ok = true;
        result.reused = true;
        return result;
    }

    uint64_t needed = 0;
    for (const std::string& f : missing) needed += fs::file_size(f, ec);
    uint64_t available = fs::space(out, ec).available;
    if (!ec && available < needed + (1ull << 30))
    {
        result.error = "not enough space in " + out_dir + ": the indexed copy needs up to " +
                       gigabytes(needed) + ", " + gigabytes(available) + " available";
        return result;
    }

    for (const std::string& f : files)
    {
        fs::path target = out / fs::path(f).filename();
        if (!missing.count(f))
        {
            // Already indexed: link rather than copy so it costs no space.
            fs::remove(target, ec);
            fs::create_hard_link(f, target, ec);
            if (ec)
            {
                fs::copy_file(f, target, fs::copy_options::overwrite_existing, ec);
            }
            if (ec)
            {
                result.error = "cannot copy " + f + ": " + ec.message();
                return result;
            }
            continue;
        }

        logMessage(cbs, "Reindexing " + f + " -> " + target.string());
        fs::path partial = target;
        partial += ".partial";   // not *.mcap, so a half-written file is never picked up as part of the bag
        bool truncated = false;
        if (!rewriteFile(f, partial, cbs, result.messages, truncated, result.error))
        {
            return result;
        }
        fs::rename(partial, target, ec);
        if (ec)
        {
            result.error = "cannot rename " + partial.string() + ": " + ec.message();
            return result;
        }
        result.rewritten++;
        if (truncated) result.truncated++;
    }

    fs::path yaml = src_dir / "metadata.yaml";
    if (fs::is_directory(bag_path) && fs::exists(yaml))
    {
        fs::copy_file(yaml, out / "metadata.yaml", fs::copy_options::overwrite_existing, ec);
    }

    std::ostringstream os;
    os << "Reindexed " << result.rewritten << " file(s), " << result.messages << " messages -> " << out_dir;
    logMessage(cbs, os.str());
    result.ok = true;
    return result;
}
