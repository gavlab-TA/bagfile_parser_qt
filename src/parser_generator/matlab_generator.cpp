#include "bagfile_parser_qt/parser_generator/matlab_generator.hpp"

MatlabGenerator::MatlabGenerator(const std::string &output_file_path, const std::string &parser_path)
{
    this->output_file_path = output_file_path;
    this->parser_path = parser_path;

    std::vector<std::string> split_string;
    boost::split(split_string, output_file_path, boost::is_any_of("/"));
    this->output_file_path = "";
    for (size_t i = 0; i < split_string.size()-1; ++i)
    {
        this->output_file_path += split_string.at(i) + "/";
    }

    loadTopicData();
}

MatlabGenerator::~MatlabGenerator()
{

}

void MatlabGenerator::loadTopicData()
{
    std::ifstream file;
    file.open(parser_path + "/files/parser_files/topic_data.txt");
    while (!file.eof())
    {
        std::string line;
        getline(file, line);
        std::vector<std::string> split_string;
        boost::split(split_string, line, boost::is_any_of("#"));

        if (split_string.size() > 1)
        {
            std::string temp = slashToUnderscore(split_string.at(1));
            MessageData msg_data = loadFieldNames(parser_path + "/files/parser_files/msg_data/" + temp + ".log");

            TopicData buffer;
            buffer.topic_name = split_string.front();
            buffer.msg_data = msg_data;

            topic_data.push_back(buffer);
        }
    }
}

MatlabGenerator::MessageData MatlabGenerator::loadFieldNames(const std::string &msg_data_filename)
{
    MessageData data;

    std::ifstream file;
    file.open(msg_data_filename);
    while (!file.eof())
    {
        std::string line;
        std::vector<std::string> split_string;
        getline(file, line);
        
        bool is_array = false;
        for (char c : line)
        {
            if (c == '[' || c == ']')
            {
                is_array = true;
                break;
            }
        }

        boost::split(split_string, line, boost::is_any_of("#"));
        if (split_string.size() > 1)
        {
            std::string temp = split_string.back();
            boost::split(split_string, temp, boost::is_any_of("!"));
            data.field_names.push_back(split_string.back());
            data.is_array_field.push_back(is_array);
        }
    }

    return data;
}

std::string MatlabGenerator::slashToUnderscore(std::string str)
{
    for (size_t i = 0; i < str.length(); i++)
    {
        if (str.at(i) == '/')
        {
            str.at(i) = '_';
        }
    }
    return str;
}