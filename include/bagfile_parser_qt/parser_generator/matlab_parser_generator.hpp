#ifndef MATLAB_PARSER_GENERATOR_HPP
#define MATLAB_PARSER_GENERATOR_HPP

#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include <sstream>

class MatlabParserGenerator
{
    public: 
    MatlabParserGenerator(const std::string &output_path, const std::string &bag_filename);
    ~MatlabParserGenerator();

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

    std::vector<std::string> readBagDataFile(std::string filename);
    std::vector<std::string> split(std::string s, char delim);
    std::string slashToUnderscore(std::string str);
    std::string slashToColon(std::string str);
    void printStringVector(std::vector<std::string> input);
    std::string snakeToCamel(std::string str);

    private: 
    int system_status;
    std::string output_path;
    std::string bag_filename;

    std::ofstream package_xml_file;
    std::ofstream cmake_lists_file;
    std::ofstream header_file;
    std::ofstream source_file;
    std::ofstream config_file;
    std::ofstream launch_file;
    std::ofstream main_file;

    std::string package_name;
    std::string class_name;
    std::string source_path;
    std::string package_path;
    
    std::string package_xml_filename;
    std::string cmake_lists_filename;
    std::string header_filename;
    std::string source_filename;
    std::string config_filename;
    std::string launch_filename;
    std::string main_filename; 

    std::vector<std::string> depends;

    std::vector<std::string> msg_header_names;
    std::vector<std::string> output_filenames;
    std::vector<std::string> output_files;
    std::vector<std::string> msg_support_vars;
    std::vector<std::string> msg_types;
    std::vector<std::string> msg_vars;
    std::vector<std::string> output_filename_strings;
    std::vector<MsgSupportPair> msg_support_pairs;
    std::vector<TopicSortingData> topic_sorting_data;
    
    // Writing Functions
    void writeCMakeLists();
    void writePackageXml();
    void writeHeader();
    void writeSource();
    void writeConfig();
    void writeLaunch();
    void writeMain();

    // Utility Functions
    void loadVectors();
    void setupFiles();

    std::string bangToDot(std::string str);
    std::string bangToUnderscore(std::string str);
    std::string convertFieldTypes(std::string str);
    
};  

#endif