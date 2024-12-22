#ifndef MESSAGE_ANALYZER_HPP
#define MESSAGE_ANALYZER_HPP

#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <stdio.h>
#include <stdlib.h>
#include <filesystem>
#include <boost/algorithm/string.hpp>

class MessageAnalyzer
{
    public:
    MessageAnalyzer(const std::string &parser_files_path, const std::string &output_path);
    ~MessageAnalyzer();

    private:
    std::string parser_files_path;
    std::string output_path;

    std::string messageTypesToSnake(std::string str);
    void makeLogs(const std::vector<std::string> &message_types);
    bool checkNativeMessageTypes(const std::string &str);
    std::stringstream loadOutput(const std::vector<std::string> &output_line);

    void decomment(const std::vector<std::string> &message_types);
    bool checkBlankString(const std::string &line);
    void deblank(const std::vector<std::string> &message_types);
    bool isCapitalOrUnderscore(char input);
    bool checkForCapitalWord(const std::string &line);
    void removeOptions(const std::vector<std::string> &message_types);
    void cleanupExtraSpaces(const std::vector<std::string> &message_types);
    void collapseTabs(const std::vector<std::string> &message_types);
    void prepForSplit(const std::vector<std::string> &message_types);
    bool checkForArray(const std::string &str);
    void setupFieldNamespaces(const std::vector<std::string> &message_types);
    void clearArrays(const std::vector<std::string> &message_types);
    void applyArraysToSubFields(const std::vector<std::string> &message_types);
    void addArrayInfo(const std::vector<std::string> &message_types);
    void removeFieldNesting(const std::vector<std::string> &message_types);
    void removeExtraSpaces(const std::vector<std::string> &message_types);
    bool checkForFieldType(const std::string &key, const std::string &line);
    void scanForUniqueFields(const std::vector<std::string> &message_types);
};

#endif