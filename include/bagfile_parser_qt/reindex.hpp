#pragma once
#include <cstdint>
#include <string>

#include "bagfile_parser_qt/converter.hpp"

struct ReindexResult
{
    bool ok = false;
    std::string error;
    std::string output_dir;    // directory holding the indexed copy
    uint64_t messages = 0;     // messages written across rewritten files
    int rewritten = 0;         // files that lacked an index and were rewritten
    int truncated = 0;         // of those, files that ended mid-record (recording cut off)
    bool reused = false;       // a complete indexed copy already existed; nothing was written
};

// Default location for the indexed copy of a bag: a sibling "<bag>_reindexed" directory.
std::string defaultReindexDir(const std::string& bag_path);

// Writes an indexed copy of a bag into out_dir. Every .mcap without a summary/index
// (usually a recording that was cut off) is rewritten record by record up to the last
// complete record, zstd-compressed, and closed properly so it gets a summary; .mcap
// files that already have one are hard-linked (copied if that fails). metadata.yaml is
// copied along. The source bag is never modified. Progress goes to cbs.log;
// cbs.cancel aborts.
ReindexResult reindexBag(const std::string& bag_path, const std::string& out_dir, const ConvertCallbacks& cbs);
