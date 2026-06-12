#pragma once
#include <atomic>
#include <functional>
#include <string>
#include <vector>

struct TopicSummary
{
    std::string topic;
    std::string msgtype;
    size_t count = 0;
};

// Returns list of files in bag_path that are missing their MCAP summary section.
std::vector<std::string> findMissingSummaries(const std::string& bag_path);

std::vector<TopicSummary> listTopics(const std::string& bag_path);

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
