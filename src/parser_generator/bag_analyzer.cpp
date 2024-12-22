#include "bagfile_parser_qt/parser_generator/bag_analyzer.hpp"

BagAnalyzer::BagAnalyzer(const std::string &data_filename, const std::string &output_path)
{
    if(std::filesystem::exists(data_filename.c_str()))
    {
        data_file.open(data_filename);
        data.clear();

        while (!data_file.eof())
        {
            std::string line;
            getline(data_file, line);
            std::vector<std::string> split_string;
            boost::split(split_string, line, boost::is_any_of(","));

            if (split_string.size() > 1)
            {
                std::pair<std::string, std::string> line_data;
                line_data.first = split_string.at(0);
                line_data.second = split_string.at(1);
                data.push_back(line_data);
            }
        }
    }
    else
    {
        std::cout<<"Critical Error - No topic names are saved"<<std::endl;
    }

    this->output_path = output_path;
    generateParserData();
}

BagAnalyzer::~BagAnalyzer()
{

}

void BagAnalyzer::formatMessageTypes(const std::string &message_type, std::string &output)
{
    std::vector<std::string> line_data; // = split(message_type, '/');
    boost::split(line_data, message_type, boost::is_any_of("/"));
    if (checkAllUpper(line_data.at(2)))
    {
        for (size_t i = 0; i < line_data.at(2).length(); i++)
        {
            line_data.at(2)[i] = tolower(line_data.at(2)[i]);
        }
    }

    std::string message_name = line_data.at(2);

    for (size_t i = 0; i < line_data.at(2).length() - 1; i++)
    {
        if (isupper(message_name[i]) && isupper(message_name[i + 1]))
        {
            message_name[i] = tolower(message_name[i]);
        }
    }
    message_name[0] = tolower(message_name[0]);
    camelToSnake(message_name, message_name);
    output = line_data.at(0) + "/" + line_data.at(1) + "/" + message_name;
}

bool BagAnalyzer::checkAllUpper(const std::string &input)
{
    for (size_t i = 0; i < input.length(); i++)
    {
        if (islower(input[i]))
        {
            return false;
        }
    }

    return true;
}

void BagAnalyzer::camelToSnake(const std::string &str, std::string &result)
{
    // Empty String
    result = "";

    // Append first character(in lower case)
    // to result string
    char c = tolower(str[0]);
    result += (char(c));

    // Traverse the string from
    // ist index to last index
    for (size_t i = 1; i < str.length(); i++)
    {

        char ch = str[i];

        // Check if the character is upper case
        // then append '_' and such character
        // (in lower case) to result string
        if (isupper(ch))
        {
            result = result + '_';
            result += char(tolower(ch));
        }

        // If the character is lower case then
        // add such character into result string
        else
        {
            result = result + ch;
        }
    }
}

void BagAnalyzer::generateParserData()
{
    topic_name_file.open(output_path + "/files/parser_files/topic_names.txt", std::ios_base::trunc);
    topic_data_file.open(output_path + "/files/parser_files/topic_data.txt", std::ios_base::trunc);
    message_name_file.open(output_path + "/files/parser_files/message_names.txt", std::ios_base::trunc);
    message_type_file.open(output_path + "/files/parser_files/message_types.txt", std::ios_base::trunc);

    for (std::pair<std::string, std::string> item : data)
    {
        topic_name_file << item.first;
        topic_name_file << "\n";

        std::string topic_data_line;
        std::string message_type;
        camelToSnake(item.second, message_type);
        topic_data_line = item.first + "#" + message_type + "#" + item.second + "\n";
        topic_data_file << topic_data_line;

        message_name_file << item.second;
        message_name_file << "\n";

        message_type_file << message_type;
        message_type_file << "\n";
    }
    topic_name_file.close();
    topic_data_file.clear();
    message_type_file.close();
    message_type_file.close();
}