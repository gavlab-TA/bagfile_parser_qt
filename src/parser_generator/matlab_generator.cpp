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
    this->output_file_path += "parse_csv2mat.m";

    loadTopicData();
    generateMatlabParser();
}

MatlabGenerator::~MatlabGenerator()
{

}

void MatlabGenerator::generateMatlabParser()
{
    matlab_parser.open(output_file_path, std::ios_base::trunc);

    std::stringstream output;
    output << "clc, clear;\n";

    for (size_t i = 0; i < topic_data.size(); ++i)
    {
        output << "fprintf(\"" + std::to_string(i+1) + "/" + std::to_string(topic_data.size()) + " - " + topic_data.at(i).topic_name + "\\n\")\n";
        output << topic_data.at(i).topic_name + "_table = readtable(\"" + topic_data.at(i).topic_name + ".csv\", VariableNamingRule='preserve');\n";
        output << topic_data.at(i).topic_name + "_array = table2cell(" + topic_data.at(i).topic_name + "_table);\n";

        output << "\n";
        output << "[r, ~] = size(" + topic_data.at(i).topic_name + "_array);\n";
        output << "for i = 1:r\n";
        output << "\tcolumn_index = 1;\n";
        for (size_t j = 0; j < topic_data.at(i).msg_data.field_names.size(); ++j)
        {
            if (topic_data.at(i).msg_data.is_array_field.at(j))
            {
                std::string temp = topic_data.at(i).msg_data.field_names.at(j);
                temp.pop_back();
                output << "\tval = " + topic_data.at(i).topic_name + "_array{i, column_index};\n";
                output << "\tif isnumeric(val)\n";
                output << "\t\tarr_size = val;\n";
                output << "\telse\n";
                output << "\t\tarr_size = str2double(val);\n";
                output << "\t\tif isnan(arr_size)\n";
                output << "\t\t\tarr_size = 0;\n";
                output << "\t\tend\n";
                output << "\tend\n";
                output << "\tcolumn_index = column_index + 1;\n";
                output << "\tfor j = 1:arr_size\n";
                output << "\t\tval = ";
                output << topic_data.at(i).topic_name + "_array{i, column_index+j-1};\n";
                output << "\t\tif isnumeric(val)\n";
                output << "\t\t\tif class(val) == \"cell\"\n";
                output << "\t\t\t\t" + topic_data.at(i).topic_name + "." + temp + "(i).data{j} = val;\n";
                output << "\t\t\telse\n";
                output << "\t\t\t\t" + topic_data.at(i).topic_name + "." + temp + "(i).data(j) = val;\n";
                output << "\t\t\tend\n";
                output << "\t\telse\n";
                output << "\t\t\tnum = str2double(val);\n";
                output << "\t\t\tif isnan(num)\n";
                output << "\t\t\t\t" + topic_data.at(i).topic_name + "." + temp + "(i).data{j} = val;\n";
                output << "\t\t\telse\n";
                output << "\t\t\t\t" + topic_data.at(i).topic_name + "." + temp + "(i).data(j) = num;\n";
                output << "\t\t\tend\n";
                output << "\t\tend\n";
                output << "\tend\n";
                output << "\tcolumn_index = column_index + arr_size;\n";
            }
            else
            {
                output << "\tval = ";
                output << topic_data.at(i).topic_name + "_array{i, column_index};\n";
                output << "\tif isnumeric(val)\n";
                output << "\t\tif exist(\"" + topic_data.at(i).topic_name + "." + topic_data.at(i).msg_data.field_names.at(j) + "\", \"var\")\n";
                output << "\t\t\tif class(" + topic_data.at(i).topic_name + "." + topic_data.at(i).msg_data.field_names.at(j) + ") == \"cell\"\n";
                output << "\t\t\t\t" + topic_data.at(i).topic_name + "." + topic_data.at(i).msg_data.field_names.at(j) + "{i} = val;\n";
                output << "\t\t\telse\n";
                output << "\t\t\t\t" + topic_data.at(i).topic_name + "." + topic_data.at(i).msg_data.field_names.at(j) + "(i) = val;\n";
                output << "\t\t\tend\n";
                output << "\t\telse\n";
                output << "\t\t\tif class(val) == \"cell\"\n";
                output << "\t\t\t\t" + topic_data.at(i).topic_name + "." + topic_data.at(i).msg_data.field_names.at(j) + "{i} = val;\n";
                output << "\t\t\telse\n";
                output << "\t\t\t\t" + topic_data.at(i).topic_name + "." + topic_data.at(i).msg_data.field_names.at(j) + "(i) = val;\n";
                output << "\t\t\tend\n";
                output << "\t\tend\n";
                output << "\telse\n";
                output << "\t\tnum = str2double(val);\n";
                output << "\t\tif isnan(num)\n";
                output << "\t\t\t" + topic_data.at(i).topic_name + "." + topic_data.at(i).msg_data.field_names.at(j) + "{i} = val;\n";
                output << "\t\telse\n";
                output << "\t\t\t" + topic_data.at(i).topic_name + "." + topic_data.at(i).msg_data.field_names.at(j) + "(i) = num;\n";
                output << "\t\tend\n";
                output << "\tend\n";
                output << "\n";
                output << "\tcolumn_index = column_index + 1;\n";

            }
        }
        output << "end\n";
        output << "save(\"" + topic_data.at(i).topic_name + ".mat\", \"" + topic_data.at(i).topic_name + "\")\n";
        output << "clear\n";
        output << "\n";
    }
    matlab_parser << output.str();
    matlab_parser.close();
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
            temp = slashToUnderscore(split_string.front());
            temp.erase(temp.begin());
            buffer.topic_name = temp;
            buffer.msg_data = msg_data;

            topic_data.push_back(buffer);
        }
    }
    file.close();
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
            temp = bangToUnderscore(temp);
            //boost::split(split_string, temp, boost::is_any_of("!"));
            data.field_names.push_back(removeSpecialChars(temp));
            data.is_array_field.push_back(is_array);
        }
    }
    file.close();
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

std::string MatlabGenerator::bangToUnderscore(std::string str)
{
    for (size_t i = 0; i < str.length(); i++)
    {
        if (str.at(i) == '!')
        {
            str.at(i) = '_';
        }
    }

    return str;
}

std::string MatlabGenerator::removeSpecialChars(std::string str)
{
    std::string output_string = "";
    for (size_t i = 0; i < str.length(); ++i)
    {
        if (str.at(i) != '@')
        {
            output_string += str.at(i);
        }
    }

    return output_string;
}