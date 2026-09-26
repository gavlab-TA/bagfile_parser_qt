#include "bagfile_parser_qt/converter.hpp"
#include "bagfile_parser_qt/gui.hpp"

#include <QtWidgets/QApplication>

#include <cstdio>
#include <iostream>
#include <string>
#include <vector>

static void printUsage()
{
    std::cerr <<
        "Usage:\n"
        "  bagfile_parser_qt                              Launch GUI\n"
        "  bagfile_parser_qt <bag_path> [options]         CLI mode\n"
        "\n"
        "Outputs land in <output>/mat/ and/or <output>/csv/ depending on --format.\n"
        "\n"
        "CLI options:\n"
        "  -l, --list-topics         List topics and exit\n"
        "  -t, --topics T1 T2 ...    Topics to convert (default: all)\n"
        "  -o, --output DIR          Output directory (default: bag folder)\n"
        "  -j, --threads N           Worker threads (default: hw cores)\n"
        "  -f, --format mat|csv|both Output format (default: mat)\n"
        "  --byte-max N              Max dynamic byte-array length to keep (default: 256)\n"
        "  --msg-max N               Max dynamic message-array count to keep (default: 20)\n"
        "  --max-pad-elems N         Max padded size of a message array, messages x entries\n"
        "                            (default: 2000000)\n"
        "  --mem-budget-mb N         Cap aggregate in-flight data (default: auto, ~20% RAM)\n"
        "  --keep-large              Don't auto-skip camera/lidar/radar (large) topics\n"
        "  --large-msg-kb N          Retire a topic if any message exceeds N KB (default: 1024)\n";
}

static int runCli(int argc, char** argv)
{
    std::string bag_path = argv[1];
    std::string output_dir;
    std::vector<std::string> topics;
    int threads = 0;
    bool list_topics_flag = false;
    OutputFormat format = OutputFormat::MAT;
    int opts_byte_max = 256;
    int opts_msg_max = 20;
    uint64_t opts_max_pad = 2000000;
    int opts_mem_budget_mb = 0;
    bool opts_skip_large = true;
    int opts_large_msg_kb = 1024;

    for (int i = 2; i < argc; i++)
    {
        std::string a = argv[i];
        if (a == "-l" || a == "--list-topics")
        {
            list_topics_flag = true;
        }
        else if (a == "-o" || a == "--output")
        {
            if (++i < argc)
            {
                output_dir = argv[i];
            }
        }
        else if (a == "-j" || a == "--threads")
        {
            if (++i < argc)
            {
                threads = std::stoi(argv[i]);
            }
        }
        else if (a == "-f" || a == "--format")
        {
            if (++i < argc)
            {
                std::string f = argv[i];
                if (f == "csv") format = OutputFormat::CSV;
                else if (f == "mat") format = OutputFormat::MAT;
                else if (f == "both") format = OutputFormat::BOTH;
                else
                {
                    std::cerr << "Unknown format: " << f << " (expected mat|csv|both)\n";
                    return 1;
                }
            }
        }
        else if (a == "-t" || a == "--topics")
        {
            while (i + 1 < argc && argv[i + 1][0] != '-')
            {
                topics.push_back(argv[++i]);
            }
        }
        else if (a == "--byte-max")
        {
            if (++i < argc) opts_byte_max = std::stoi(argv[i]);
        }
        else if (a == "--msg-max")
        {
            if (++i < argc) opts_msg_max = std::stoi(argv[i]);
        }
        else if (a == "--max-pad-elems")
        {
            if (++i < argc) opts_max_pad = std::stoull(argv[i]);
        }
        else if (a == "--mem-budget-mb")
        {
            if (++i < argc) opts_mem_budget_mb = std::stoi(argv[i]);
        }
        else if (a == "--keep-large")
        {
            opts_skip_large = false;
        }
        else if (a == "--large-msg-kb")
        {
            if (++i < argc) opts_large_msg_kb = std::stoi(argv[i]);
        }
        else if (a == "-h" || a == "--help")
        {
            printUsage();
            return 0;
        }
        else
        {
            std::cerr << "Unknown option: " << a << "\n";
            printUsage();
            return 1;
        }
    }

    if (list_topics_flag)
    {
        std::vector<TopicSummary> ts = listTopics(bag_path);
        for (const TopicSummary& t : ts)
        {
            std::printf("  %-48s %-40s %8zu\n", t.topic.c_str(), t.msgtype.c_str(), t.count);
        }
        return 0;
    }

    ConvertOptions opts;
    opts.bag_path = bag_path;
    opts.output_dir = output_dir;
    opts.topics = topics;
    opts.threads = threads;
    opts.format = format;
    opts.byte_array_max = opts_byte_max;
    opts.msg_array_max = opts_msg_max;
    opts.max_pad_elems = opts_max_pad;
    opts.mem_budget_mb = opts_mem_budget_mb;
    opts.skip_large_topics = opts_skip_large;
    opts.large_msg_kb = opts_large_msg_kb;

    ConvertCallbacks cbs;
    cbs.log = [](const std::string& m)
    {
        std::cerr << m << "\n";
    };

    try
    {
        convert(opts, cbs);
    }
    catch (const std::exception& e)
    {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }
    return 0;
}

int main(int argc, char** argv)
{
    if (argc > 1 && (std::string(argv[1]) == "-h" || std::string(argv[1]) == "--help"))
    {
        printUsage();
        return 0;
    }

    if (argc > 1)
    {
        return runCli(argc, argv);
    }

    QApplication app(argc, argv);
    MainWindow main_window;
    main_window.show();
    return app.exec();
}
