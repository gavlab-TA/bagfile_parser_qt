#ifndef CSV_PARSER_GENERATOR_HPP
#define CSV_PARSER_GENERATOR_HPP

#include <fstream>
#include <iostream>
#include <string>  
#include <vector>
#include <sstream>
#include <boost/algorithm/string.hpp>
#include <filesystem>

class CsvParserGenerator
{
    public:
    CsvParserGenerator(const std::string &output_path, const std::string &bag_filename);
    ~CsvParserGenerator();

    struct MsgSupportPair{
        std::string msg_support_var;
        std::string msg_type;
    };

    struct TopicSortingData
    {
        std::string topic_name;
        std::string msg_var;
        std::string msg_type;
        std::string msg_support_var;
    };

    std::string slashToUnderscore(std::string str);
    std::string slashToColon(std::string str);
    std::string snakeToCamel(std::string str);
    std::vector<std::string> readFile(const std::string &filename);
    std::string bangToDot(std::string str);
    std::string bangToUnderscore(std::string str);
    std::string convertFieldTypes(std::string str);

    private:
    std::string output_path;
    std::string output_package_path;
    std::string bag_filename;

    std::ofstream package_xml_file;
    std::ofstream cmake_lists_file;
    std::ofstream header_file;
    std::ofstream source_file;
    std::ofstream config_file;
    std::ofstream launch_file;
    std::ofstream main_file;

    std::string package_xml_filename;
    std::string cmake_lists_filename;
    std::string header_filename;
    std::string source_filename;
    std::string config_filename;
    std::string launch_filename;
    std::string main_filename;

    std::vector<std::string> depends;
    std::vector<std::string> msg_header_names;
    std::vector<std::string> output_filename_vars;
    std::vector<std::string> msg_support_vars;
    std::vector<std::string> msg_types;
    std::vector<std::string> msg_vars;
    std::vector<std::string> output_file_vars;
    std::vector<std::string> output_filename_strings;
    std::vector<MsgSupportPair> msg_support_pairs;
    std::vector<TopicSortingData> topic_sorting_data;

    void writeCMakeLists();
    void writePackageXml();
    void writeHeader();
    void writeSource();
    void writeConfig();
    void writeLaunch();
    void writeMain();

    void loadVectors();
    void setupFiles();
    std::vector<std::string> split(std::string s, char delim);
};

#endif