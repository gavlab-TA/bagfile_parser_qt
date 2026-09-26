#pragma once
#include <atomic>
#include <cstdint>
#include <functional>
#include <string>
#include <vector>

struct TopicSummary
{
    std::string topic;
    std::string msgtype;
    size_t count = 0;
};

// .mcap files of a bag: the file itself, or every .mcap in the directory, sorted.
std::vector<std::string> findMcapFiles(const std::string& path);

// Returns list of files in bag_path that are missing their MCAP summary section.
std::vector<std::string> findMissingSummaries(const std::string& bag_path);

std::vector<TopicSummary> listTopics(const std::string& bag_path);

// Camera / lidar message types (Image, PointCloud2, ...) that are skipped unless
// skip_large_topics is off (CLI --keep-large; GUI "Skip large topics" unchecked).
bool isLargeSensorType(const std::string& msgtype);

enum class OutputFormat { MAT, CSV, BOTH };

struct ConvertOptions
{
    std::string bag_path;
    std::vector<std::string> topics; // empty = all
    std::string output_dir;
    int threads = 0;
    OutputFormat format = OutputFormat::MAT;
    int byte_array_max = 256;
    int msg_array_max = 20;
    // Cap on a message array's padded size (messages x entries x enclosing entries).
    uint64_t max_pad_elems = 2000000;
    int mem_budget_mb = 0;
    bool skip_large_topics = true;
    int large_msg_kb = 1024;
};

struct ConvertCallbacks
{
    std::function<void(const std::string&)> log;
    std::function<void(const std::string&)> topic_done;
    std::atomic<bool>* cancel = nullptr;
};

void convert(const ConvertOptions& opts, const ConvertCallbacks& cbs);
