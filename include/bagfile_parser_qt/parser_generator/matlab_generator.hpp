#ifndef MATLAB_GENERATOR_HPP
#define MATLAB_GENERATOR_HPP

#include <string>
#include <vector>
#include <fstream>
#include <boost/algorithm/string.hpp>
#include <iostream>

class MatlabGenerator
{
    public: 
    MatlabGenerator(const std::string &output_file_path, const std::string &parser_path);
    ~MatlabGenerator();

    struct MessageData{
        std::vector<std::string> field_names;
        std::vector<bool> is_array_field;
    };

    struct TopicData{
        std::string topic_name;
        MessageData msg_data;
    };

    private:
    std::string output_file_path;
    std::string parser_path;
    std::ofstream matlab_parser;

    std::vector<TopicData> topic_data;

    void loadTopicData();
    std::string slashToUnderscore(std::string str);
    MessageData loadFieldNames(const std::string &msg_data_filename);
};

#endif