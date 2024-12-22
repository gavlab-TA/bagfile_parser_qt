#ifndef BAG_ANALYZER_HPP
#define BAG_ANALYZER_HPP

#include <string>
#include <vector>
#include <fstream>
#include <filesystem>
#include <iostream>
#include <boost/algorithm/string.hpp>

class BagAnalyzer
{
    public:
    BagAnalyzer(const std::string &data_filename, const std::string &output_path);
    ~BagAnalyzer();
    void generateParserData();

    private:
    std::vector<std::pair<std::string, std::string>> data;

    std::string output_path;
    std::ifstream data_file;
    std::ofstream topic_name_file;
    std::ofstream topic_data_file;
    std::ofstream message_type_file;
    std::ofstream message_name_file;

    std::string formatMessageTypes(std::string input);
    std::string camelToSnake(std::string input);
    bool checkAllUpper(const std::string &input);
};

#endif
