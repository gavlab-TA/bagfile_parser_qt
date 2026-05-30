#ifdef HAS_LZ4
#include <lz4.h>
#include <lz4frame.h>
#endif

#ifdef HAS_ZSTD
#include <zstd.h>
#endif

#define MCAP_IMPLEMENTATION
#include <mcap/reader.hpp>

#include "bagfile_parser_qt/converter.hpp"
#include "bagfile_parser_qt/schema_parser.hpp"
#include "bagfile_parser_qt/cdr_reader.hpp"
#include "bagfile_parser_qt/mat_writer.hpp"
#include "bagfile_parser_qt/csv_writer.hpp"

#include <algorithm>
#include <atomic>
#include <chrono>
#include <condition_variable>
#include <filesystem>
#include <functional>
#include <map>
#include <mutex>
#include <optional>
#include <set>
#include <sstream>
#include <thread>
#include <unordered_map>
#include <utility>

#if defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#elif defined(__APPLE__)
#include <sys/sysctl.h>
#else
#include <unistd.h>
#endif

#if defined(__GLIBC__)
#include <malloc.h>
#endif

namespace fs = std::filesystem;

static std::vector<std::string> findMcapFiles(const std::string& path)
{
    std::vector<std::string> files;
    if (fs::is_regular_file(path) && fs::path(path).extension() == ".mcap")
    {
        files.push_back(path);
    }
    else if (fs::is_directory(path))
    {
        for (const fs::directory_entry& entry : fs::directory_iterator(path))
        {
            if (entry.path().extension() == ".mcap")
            {
                files.push_back(entry.path().string());
            }
        }
        std::sort(files.begin(), files.end());
    }
    return files;
}

std::vector<std::string> findMissingSummaries(const std::string& bag_path)
{
    std::vector<std::string> mcap_files = findMcapFiles(bag_path);
    std::vector<std::string> missing;
    for (const std::string& file : mcap_files)
    {
        mcap::McapReader reader;
        if (!reader.open(file).ok())
        {
            continue;
        }
        mcap::Status status = reader.readSummary(mcap::ReadSummaryMethod::NoFallbackScan);
        if (!status.ok())
        {
            missing.push_back(file);
        }
        reader.close();
    }
    return missing;
}

std::vector<TopicSummary> listTopics(const std::string& bag_path)
{
    std::vector<std::string> mcap_files = findMcapFiles(bag_path);
    std::map<std::string, TopicSummary> topics;

    for (const std::string& file : mcap_files)
    {
        mcap::McapReader reader;
        if (!reader.open(file).ok())
        {
            continue;
        }
        (void)reader.readSummary(mcap::ReadSummaryMethod::AllowFallbackScan);

        const std::optional<mcap::Statistics>& stats = reader.statistics();
        const std::unordered_map<mcap::ChannelId, mcap::ChannelPtr>& channels = reader.channels();
        const std::unordered_map<mcap::SchemaId, mcap::SchemaPtr>& schemas = reader.schemas();

        for (const std::pair<const mcap::ChannelId, mcap::ChannelPtr>& kv : channels)
        {
            const mcap::ChannelId& id = kv.first;
            const mcap::ChannelPtr& ch = kv.second;
            const mcap::SchemaPtr& schema = schemas.at(ch->schemaId);
            size_t count = 0;
            if (stats)
            {
                std::unordered_map<mcap::ChannelId, uint64_t>::const_iterator it =
                    stats->channelMessageCounts.find(id);
                if (it != stats->channelMessageCounts.end())
                {
                    count = it->second;
                }
            }
            TopicSummary& t = topics[ch->topic];
            t.topic = ch->topic;
            t.msgtype = schema->name;
            t.count += count;
        }
        reader.close();
    }

    std::vector<TopicSummary> result;
    result.reserve(topics.size());
    for (std::pair<const std::string, TopicSummary>& kv : topics)
    {
        result.push_back(std::move(kv.second));
    }
    return result;
}

struct TopicData
{
    std::string name;
    std::string msgtype;
    FieldDesc schema;
    std::vector<uint8_t> blob;
    std::vector<size_t> offsets;
    std::vector<uint32_t> lengths;
    std::vector<double> timestamps;
    bool large_skip = false;

    size_t count() const { return this->lengths.size(); }

    void append(double t, const uint8_t* bytes, size_t len)
    {
        this->offsets.push_back(this->blob.size());
        this->lengths.push_back(static_cast<uint32_t>(len));
        this->timestamps.push_back(t);
        this->blob.insert(this->blob.end(), bytes, bytes + len);
    }

    void release()
    {
        std::vector<uint8_t>().swap(this->blob);
        std::vector<size_t>().swap(this->offsets);
        std::vector<uint32_t>().swap(this->lengths);
    }

    void releaseAll()
    {
        this->release();
        std::vector<double>().swap(this->timestamps);
    }
};

static bool isLargeSensorType(const std::string& msgtype)
{
    std::string leaf = msgtype;
    std::string::size_type slash = msgtype.rfind('/');
    if (slash != std::string::npos)
    {
        leaf = msgtype.substr(slash + 1);
    }
    static const char* kSkip[] = {
        "Image", "CompressedImage", "CompressedVideo", "PointCloud", "PointCloud2"
    };
    for (const char* s : kSkip)
    {
        if (leaf == s) return true;
    }
    return false;
}

static void logMessage(const ConvertCallbacks& cbs, const std::string& msg)
{
    if (cbs.log)
    {
        cbs.log(msg);
    }
}

static size_t doublesPerMessage(const FieldDesc& desc)
{
    size_t total = 0;
    for (const FieldDesc& f : desc.fields)
    {
        if (f.skip)
        {
            continue;
        }
        if (f.is_primitive_type)
        {
            bool is_str = (f.primitive == PrimitiveType::STRING ||
                           f.primitive == PrimitiveType::WSTRING);
            if (is_str)
            {
                continue;
            }
            if (f.array_size > 0)
            {
                total += static_cast<size_t>(f.array_size);
            }
            else if (f.array_size == 0)
            {
                total += 1;
            }
            else
            {
                total += 1;
            }
        }
        else
        {
            total += doublesPerMessage(f);
        }
    }
    return total;
}

static const size_t kStringFootprint = 128;

static size_t estimateFootprint(const FieldDesc& desc, size_t n)
{
    size_t total = 0;
    for (const FieldDesc& f : desc.fields)
    {
        if (f.skip)
        {
            continue;
        }
        if (f.padded_max > 0 && !f.is_primitive_type)
        {
            FieldDesc elem;
            elem.fields = f.fields;
            total += static_cast<size_t>(f.padded_max) * estimateFootprint(elem, n);
            continue;
        }
        if (f.is_primitive_type)
        {
            bool is_str = (f.primitive == PrimitiveType::STRING ||
                           f.primitive == PrimitiveType::WSTRING);
            if (is_str)
            {
                size_t width = (f.array_size > 0) ? static_cast<size_t>(f.array_size) : 1;
                total += n * width * kStringFootprint;
            }
            else if (f.array_size == 0)
            {
                total += n * sizeof(double);
            }
            else if (f.array_size > 0)
            {
                total += n * static_cast<size_t>(f.array_size) * sizeof(double);
            }
            else
            {
                total += n * 4 * sizeof(double);
            }
        }
        else
        {
            total += estimateFootprint(f, n);
        }
    }
    return total;
}

static size_t physicalRamBytes()
{
#if defined(_WIN32)
    MEMORYSTATUSEX mem_info;
    mem_info.dwLength = sizeof(mem_info);
    if (GlobalMemoryStatusEx(&mem_info))
    {
        return static_cast<size_t>(mem_info.ullTotalPhys);
    }
    return 0;
#elif defined(__APPLE__)
    int mib[2] = {CTL_HW, HW_MEMSIZE};
    uint64_t mem_size = 0;
    size_t len = sizeof(mem_size);
    if (sysctl(mib, 2, &mem_size, &len, nullptr, 0) == 0)
    {
        return static_cast<size_t>(mem_size);
    }
    return 0;
#else
    long pages = sysconf(_SC_PHYS_PAGES);
    long page_size = sysconf(_SC_PAGE_SIZE);
    if (pages > 0 && page_size > 0)
    {
        return static_cast<size_t>(pages) * static_cast<size_t>(page_size);
    }
    return 0;
#endif
}

void convert(const ConvertOptions& opts, const ConvertCallbacks& cbs)
{
    std::vector<std::string> mcap_files = findMcapFiles(opts.bag_path);
    if (mcap_files.empty())
    {
        logMessage(cbs, "No .mcap files found in " + opts.bag_path);
        return;
    }

    std::map<std::string, TopicData> topics;
    std::map<std::string, std::string> skipped_topics;
    std::chrono::steady_clock::time_point t_start = std::chrono::steady_clock::now();

    // --- Phase 1: read MCAP files, buffer raw messages per topic ---
    for (const std::string& mcap_file : mcap_files)
    {
        if (cbs.cancel && cbs.cancel->load())
        {
            return;
        }

        mcap::McapReader reader;
        mcap::Status status = reader.open(mcap_file);
        if (!status.ok())
        {
            logMessage(cbs, "Failed to open " + mcap_file + ": " + status.message);
            continue;
        }

        std::function<void(const mcap::Status&)> on_problem =
            [&](const mcap::Status& s)
        {
            logMessage(cbs, "mcap warning: " + s.message);
        };
        (void)reader.readSummary(mcap::ReadSummaryMethod::AllowFallbackScan, on_problem);

        std::map<mcap::SchemaId, FieldDesc> schemas;
        const std::unordered_map<mcap::SchemaId, mcap::SchemaPtr>& mcap_schemas = reader.schemas();
        for (const std::pair<const mcap::SchemaId, mcap::SchemaPtr>& kv : mcap_schemas)
        {
            const mcap::SchemaId& id = kv.first;
            const mcap::SchemaPtr& schema_ptr = kv.second;
            std::string def(reinterpret_cast<const char*>(schema_ptr->data.data()), schema_ptr->data.size());
            try
            {
                schemas[id] = parseSchema(def, schema_ptr->name);
            }
            catch (const std::exception& e)
            {
                logMessage(cbs, "schema parse failed for " + schema_ptr->name + ": " + e.what());
            }
        }

        std::set<std::string> filter(opts.topics.begin(), opts.topics.end());
        size_t large_msg_bytes = opts.large_msg_kb > 0
            ? static_cast<size_t>(opts.large_msg_kb) * 1024
            : 0;

        mcap::LinearMessageView view = reader.readMessages(on_problem);
        size_t msg_count = 0;
        for (mcap::LinearMessageView::Iterator it = view.begin(); it != view.end(); ++it)
        {
            if (cbs.cancel && cbs.cancel->load())
            {
                reader.close();
                return;
            }

            const mcap::Channel& channel = *it->channel;
            if (!filter.empty() && filter.count(channel.topic) == 0)
            {
                continue;
            }
            if (schemas.find(channel.schemaId) == schemas.end())
            {
                continue;
            }

            const std::string& msgtype = it->schema->name;
            if (opts.skip_large_topics && isLargeSensorType(msgtype))
            {
                if (skipped_topics.emplace(channel.topic, msgtype).second)
                {
                    logMessage(cbs, "Skipping " + channel.topic + " (" + msgtype +
                             "): high-volume sensor type, not exported (use --keep-large to include)");
                }
                continue;
            }

            TopicData& info = topics[channel.topic];
            if (info.large_skip)
            {
                continue;
            }
            if (info.count() == 0)
            {
                info.name = channel.topic;
                info.msgtype = msgtype;
                info.schema = schemas.at(channel.schemaId);
            }

            if (opts.skip_large_topics && large_msg_bytes > 0 && it->message.dataSize > large_msg_bytes)
            {
                info.large_skip = true;
                info.releaseAll();
                if (skipped_topics.emplace(channel.topic, msgtype).second)
                {
                    std::ostringstream os;
                    os << "Skipping " << channel.topic << " (" << msgtype << "): message of "
                       << (it->message.dataSize / 1024) << " KB exceeds " << opts.large_msg_kb
                       << " KB limit, not exported (use --keep-large or raise --large-msg-kb)";
                    logMessage(cbs, os.str());
                }
                continue;
            }

            double ts = static_cast<double>(it->message.logTime) * 1e-9;
            const uint8_t* msg_bytes = reinterpret_cast<const uint8_t*>(it->message.data);
            info.append(ts, msg_bytes, it->message.dataSize);

            if (++msg_count % 50000 == 0)
            {
                std::ostringstream os;
                os << "read " << msg_count << " messages...";
                logMessage(cbs, os.str());
            }
        }
        reader.close();
    }

    if (topics.empty())
    {
        logMessage(cbs, "No messages found.");
        return;
    }

    std::chrono::steady_clock::time_point t_read = std::chrono::steady_clock::now();
    double read_sec = std::chrono::duration<double>(t_read - t_start).count();
    size_t total_msgs = 0;
    for (const std::pair<const std::string, TopicData>& kv : topics)
    {
        total_msgs += kv.second.count();
    }
    {
        std::ostringstream os;
        os << "Read " << total_msgs << " messages across " << topics.size()
           << " topics in " << read_sec << "s";
        logMessage(cbs, os.str());
    }

    // --- Phase 2 & 3: deserialize + write, parallelized by topic ---
    fs::path output_dir = opts.output_dir.empty() ? fs::current_path() : fs::path(opts.output_dir);
    fs::create_directories(output_dir);

    const char* ext = (opts.format == OutputFormat::CSV) ? ".csv" : ".mat";
    struct WorkItem
    {
        TopicData* info;
        std::string mat_name;
        std::string filepath;
        size_t cost;
    };
    std::vector<WorkItem> work;
    work.reserve(topics.size());
    for (std::pair<const std::string, TopicData>& kv : topics)
    {
        if (kv.second.large_skip || kv.second.count() == 0)
        {
            continue;
        }
        WorkItem w;
        w.info = &kv.second;
        w.mat_name = sanitizeName(kv.first);
        w.filepath = (output_dir / (w.mat_name + ext)).string();
        w.cost = kv.second.blob.size()
               + kv.second.count() * doublesPerMessage(kv.second.schema) * sizeof(double);
        work.push_back(std::move(w));
    }
    std::sort(work.begin(), work.end(),
              [](const WorkItem& a, const WorkItem& b) { return a.cost > b.cost; });

    size_t mem_budget = opts.mem_budget_mb > 0
        ? static_cast<size_t>(opts.mem_budget_mb) * 1024ull * 1024ull
        : 0;
    if (mem_budget == 0)
    {
        const size_t MiB = static_cast<size_t>(1024) * 1024;
        size_t ram = physicalRamBytes();
        size_t auto_budget = ram ? ram / 6 : 1536 * MiB;
        auto_budget = std::max(auto_budget, 1024 * MiB);
        auto_budget = std::min(auto_budget, 3072 * MiB);
        mem_budget = auto_budget;
    }

    std::atomic<size_t> next_item{0};
    std::mutex log_mutex;
    std::mutex budget_mutex;
    std::condition_variable budget_cv;
    size_t in_flight_bytes = 0;

    std::function<void()> worker = [&]()
    {
        while (true)
        {
            if (cbs.cancel && cbs.cancel->load())
            {
                return;
            }
            size_t idx = next_item.fetch_add(1);
            if (idx >= work.size())
            {
                return;
            }

            WorkItem& w = work[idx];
            TopicData& info = *w.info;
            size_t n = info.count();

            std::unordered_map<std::string, uint32_t> stats;
            for (size_t i = 0; i < n; i++)
            {
                try
                {
                    CdrReader reader(info.blob.data() + info.offsets[i], info.lengths[i]);
                    scanMessage(info.schema, reader, stats);
                }
                catch (const std::exception&) { /* counted as 0 */ }
            }
            FieldDesc adjusted = adjustSchema(info.schema, stats, opts.byte_array_max, opts.msg_array_max, n);

            size_t topic_cost = estimateFootprint(adjusted, n) * 2 + info.blob.size();

            {
                std::unique_lock<std::mutex> lock(budget_mutex);
                budget_cv.wait(lock, [&]()
                {
                    if (cbs.cancel && cbs.cancel->load()) return true;
                    return in_flight_bytes == 0 || in_flight_bytes + topic_cost <= mem_budget;
                });
                if (cbs.cancel && cbs.cancel->load()) return;
                in_flight_bytes += topic_cost;
            }
            struct BudgetGuard
            {
                std::mutex& m;
                std::condition_variable& cv;
                size_t& in_flight;
                size_t cost;
                bool released = false;
                void release()
                {
                    if (released) return;
                    released = true;
                    {
                        std::lock_guard<std::mutex> lock(m);
                        in_flight -= cost;
                    }
                    cv.notify_all();
                }
                ~BudgetGuard() { release(); }
            } budget_guard{budget_mutex, budget_cv, in_flight_bytes, topic_cost};

            std::function<void(const FieldDesc&, const std::string&, std::vector<std::string>&)> collect_skipped =
                [&](const FieldDesc& f, const std::string& path, std::vector<std::string>& out)
            {
                if (f.skip)
                {
                    std::unordered_map<std::string, uint32_t>::const_iterator it = stats.find(path);
                    uint32_t mx = (it != stats.end()) ? it->second : 0;
                    std::ostringstream s;
                    s << path << " (max=" << mx << ")";
                    out.push_back(s.str());
                    return;
                }
                for (const FieldDesc& sub : f.fields)
                {
                    std::string sp = path.empty() ? sub.name : path + "." + sub.name;
                    collect_skipped(sub, sp, out);
                }
            };
            std::vector<std::string> skipped_paths;
            for (const FieldDesc& sub : adjusted.fields)
            {
                collect_skipped(sub, sub.name, skipped_paths);
            }
            if (!skipped_paths.empty())
            {
                std::ostringstream s;
                s << info.name << ": dropped " << skipped_paths.size() << " field(s):";
                for (const std::string& p : skipped_paths) s << " " << p;
                std::lock_guard<std::mutex> lock(log_mutex);
                logMessage(cbs, s.str());
            }

            CollectedData collected;
            allocateCollectors(collected, adjusted, n);

            for (size_t i = 0; i < n; i++)
            {
                if (cbs.cancel && cbs.cancel->load())
                {
                    return;
                }
                try
                {
                    CdrReader reader(info.blob.data() + info.offsets[i], info.lengths[i]);
                    fillMessage(collected, adjusted, reader, i);
                }
                catch (const std::exception& e)
                {
                    std::lock_guard<std::mutex> lock(log_mutex);
                    std::ostringstream os;
                    os << "deserialize failed at " << info.name << " #" << i << ": " << e.what();
                    logMessage(cbs, os.str());
                }
            }

            std::vector<double> timestamps = std::move(info.timestamps);
            info.release();

            if (opts.format == OutputFormat::CSV)
            {
                writeCsv(w.filepath, collected, timestamps);
            }
            else
            {
                writeMat(w.filepath, w.mat_name, collected, timestamps);
            }

            collected = CollectedData();
#if defined(__GLIBC__)
            malloc_trim(0);
#endif
            budget_guard.release();

            {
                std::lock_guard<std::mutex> lock(log_mutex);
                std::ostringstream os;
                os << info.name << " -> " << w.mat_name << ext << " (" << n << " msgs)";
                logMessage(cbs, os.str());
            }
            if (cbs.topic_done)
            {
                cbs.topic_done(info.name);
            }
        }
    };

    int n_threads = opts.threads > 0
        ? opts.threads
        : static_cast<int>(std::thread::hardware_concurrency());
    if (n_threads <= 0)
    {
        n_threads = 4;
    }
    n_threads = std::min(n_threads, static_cast<int>(work.size()));

    std::vector<std::thread> pool;
    pool.reserve(n_threads);
    for (int i = 0; i < n_threads; i++)
    {
        pool.emplace_back(worker);
    }
    for (std::thread& t : pool)
    {
        t.join();
    }

    std::chrono::steady_clock::time_point t_done = std::chrono::steady_clock::now();
    double total_sec = std::chrono::duration<double>(t_done - t_start).count();
    {
        std::ostringstream os;
        os << "Done. " << work.size() << " topic(s) -> " << output_dir.string()
           << " (total " << total_sec << "s)";
        logMessage(cbs, os.str());
    }
}
