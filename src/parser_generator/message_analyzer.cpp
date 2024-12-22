#include "bagfile_parser_qt/parser_generator/message_analyzer.hpp"

MessageAnalyzer::MessageAnalyzer(const std::string &parser_files_path, const std::string &output_path)
{
    this->parser_files_path = parser_files_path;
    this->output_path = output_path;

    std::vector<std::string> message_types;
    std::ifstream message_names_file;
    std::string filename = this->parser_files_path + "message_names.txt";
    message_names_file.open(filename);

    std::string temp;
    while (!message_names_file.eof())
    {
        getline(message_names_file, temp);
        if (temp != "")
        {
            message_types.push_back(temp);
        }
    }

    std::cout<<"Retrieved Messages"<<std::endl;

    makeLogs(message_types);
    std::cout<<"Logs Created"<<std::endl;

    scanForUniqueFields(message_types);
    std::cout<<"Unique Fields"<<std::endl;

    decomment(message_types);
    std::cout<<"Remove Comment"<<std::endl;

    removeOptions(message_types);
    std::cout<<"Remove Options"<<std::endl;

    deblank(message_types);
    std::cout<<"Remove Blanks"<<std::endl;

    cleanupExtraSpaces(message_types);
    std::cout<<"Remove Extra Spaces"<<std::endl;

    collapseTabs(message_types);
    std::cout<<"Collapse Tabs"<<std::endl;

    prepForSplit(message_types);
    std::cout<<"Prep For Splits"<<std::endl;

    applyArraysToSubFields(message_types);
    std::cout<<"Handle Arrays"<<std::endl;

    setupFieldNamespaces(message_types);
    std::cout<<"Setup Namespaces"<<std::endl;

    clearArrays(message_types);
    std::cout<<"Clear Arrays"<<std::endl;

    addArrayInfo(message_types);
    std::cout<<"Add Array Info"<<std::endl;

    removeFieldNesting(message_types);
    std::cout<<"Remove Field Nesting"<<std::endl;

    removeExtraSpaces(message_types);
    std::cout<<"Remove Extra Spaces"<<std::endl;
}

MessageAnalyzer::~MessageAnalyzer()
{

}

std::string MessageAnalyzer::messageTypesToSnake(std::string str)
{
    for (size_t i = 0; i < str.length(); i++)
    {
        if (str.at(i) == '/')
        {
            if (i < str.length() - 1)
            {
                str.at(i + 1) = tolower(str.at(i + 1));
            }
            str.at(i) = '_';
        }
        else if (i < str.length() - 1)
        {
            if (isupper(str.at(i)) && islower(str.at(i + 1)))
            {
                str.insert(i, "_");
                str.at(i + 1) = tolower(str.at(i + 1));
            }
        }

        if (isupper(str.at(i)))
        {
            str.at(i) = tolower(str.at(i));
        }
    }
    return str;
}

void MessageAnalyzer::makeLogs(const std::vector<std::string> &message_types)
{
    std::stringstream output;
    int system_status;
    std::string command;

    for (size_t i = 0; i < message_types.size(); i++)
    {
        command = "touch " + parser_files_path + "msg_data/" + messageTypesToSnake(message_types.at(i)) + ".log";
        system_status = system(command.c_str());

        command = "bash -c 'source " + output_path + "/install/setup.bash && " + "ros2 interface show " + message_types.at(i) + " > " + parser_files_path + "msg_data/" + messageTypesToSnake(message_types.at(i)) + ".log'";
        std::cout << "Pulling ROS Message Data: " << std::to_string(i + 1) << "/" << std::to_string(message_types.size()) << std::endl;
        system_status = system(command.c_str());
        if (system_status)
        {
            std::cout<<"Failed to pull: " << message_types.at(i) << std::endl;
        }
    }
}

bool MessageAnalyzer::checkNativeMessageTypes(const std::string &str)
{
    if (str == "bool" || str == "bool[]")
    {
        return true;
    }
    else if (str == "byte" || str == "byte[]")
    {
        return true;
    }
    else if (str == "char" || str == "char[]")
    {
        return true;
    }
    else if (str == "float32" || str == "float32[]")
    {
        return true;
    }
    else if (str == "float64" || str == "float64[]")
    {
        return true;
    }
    else if (str == "int8" || str == "int8[]")
    {
        return true;
    }
    else if (str == "uint8" || str == "uint8[]")
    {
        return true;
    }
    else if (str == "int16" || str == "int16[]")
    {
        return true;
    }
    else if (str == "uint16" || str == "uint16[]")
    {
        return true;
    }
    else if (str == "int32" || str == "int32")
    {
        return true;
    }
    else if (str == "uint32" || str == "uint32[]")
    {
        return true;
    }
    else if (str == "int64" || str == "int64[]")
    {
        return true;
    }
    else if (str == "uint64" || str == "uint64[]")
    {
        return true;
    }
    else if (str == "string" || str == "string[]")
    {
        return true;
    }
    else if (str == "wstring" || str == "wstring[]")
    {
        return true;
    }
    else
    {
        return false;
    }
}

std::stringstream MessageAnalyzer::loadOutput(const std::vector<std::string> &output_line)
{
std::stringstream output("");
    for (size_t i = 0; i < output_line.size(); i++)
    {
        if (i == output_line.size() - 1)
        {
            output << output_line.at(i);
        }
        else
        {
            output << output_line.at(i) + "\n";
        }
    }
    return output;}

void MessageAnalyzer::decomment(const std::vector<std::string> &message_types)
{
    std::string filename;
    std::fstream file;
    std::stringstream output;
    std::string buffer;
    std::vector<std::string> output_line;

    for (size_t i = 0; i < message_types.size(); i++)
    {
        output_line.clear();

        filename = parser_files_path + "msg_data/" + messageTypesToSnake(message_types.at(i)) + ".log";
        file.open(filename);
        while (!file.eof())
        {
            getline(file, buffer);

            for (size_t i = 0; i < buffer.length(); i++)
            {
                if (buffer.at(i) == '#')
                {
                    buffer.erase(buffer.begin() + i, buffer.end());
                }
            }
            output_line.push_back(buffer);
        }
        file.close();

        output = loadOutput(output_line);

        file.open(filename, std::ofstream::out | std::ofstream::trunc);
        file << output.str();
        file.close();
    }
}

bool MessageAnalyzer::checkBlankString(const std::string &line)
{
for (size_t i = 0; i < line.length(); i++)
    {
        if (line.at(i) != ' ' && line.at(i) != '\t')
        {
            return false;
        }
    }
    return true;}

void MessageAnalyzer::deblank(const std::vector<std::string> &message_types)
{
    std::string filename;
    std::fstream file;
    std::stringstream output("");
    std::vector<std::string> output_lines;
    std::string buffer;

    for (size_t i = 0; i < message_types.size(); i++)
    {
        output_lines.clear();

        filename = parser_files_path + "msg_data/" + messageTypesToSnake(message_types.at(i)) + ".log";
        file.open(filename);
        while (!file.eof())
        {
            getline(file, buffer);
            if (!checkBlankString(buffer))
            {
                output_lines.push_back(buffer);
            }
        }
        file.close();

        output = loadOutput(output_lines);

        file.open(filename, std::ofstream::out | std::ofstream::trunc);
        file << output.str();
        file.close();
    }
}

bool MessageAnalyzer::isCapitalOrUnderscore(char input)
{
    if (isupper(input) || input == '_' || isdigit(input))
    {
        return true;
    }
    else
    {
        return false;
    }
}

bool MessageAnalyzer::checkForCapitalWord(const std::string &line)
{
bool on_capital_word = false;
    bool looking_for_word = true;
    for (size_t i = 0; i < line.length(); i++)
    {
        if ((line.at(i) == ' ' || line.at(i) == '\t' || line.at(i) == '=') && on_capital_word)
        {
            return true;
        }

        if (line.at(i) == ' ' || line.at(i) == '\t')
        {
            looking_for_word = true;
        }

        if (looking_for_word && isCapitalOrUnderscore(line.at(i)))
        {
            on_capital_word = true;
            looking_for_word = false;
        }

        if (on_capital_word && !isCapitalOrUnderscore(line.at(i)))
        {
            on_capital_word = false;
        }

        if (line.at(i) == ' ' || line.at(i) == '\t')
        {
            looking_for_word = true;
        }
        else
        {
            looking_for_word = false;
        }
    }

    return false;}

void MessageAnalyzer::removeOptions(const std::vector<std::string> &message_types)
{
    std::string filename;
    std::fstream file;
    std::stringstream output;
    std::string buffer;
    std::vector<std::string> output_line;
    std::vector<std::string> split_string;

    for (size_t i = 0; i < message_types.size(); i++)
    {
        output = std::stringstream("");
        output_line.clear();
        filename = parser_files_path + "msg_data/" + messageTypesToSnake(message_types.at(i)) + ".log";
        file.open(filename);
        while (!file.eof())
        {
            getline(file, buffer);
            //split_string = split(buffer, ' ');
            boost::split(split_string, buffer, boost::is_any_of(" "));
            if (split_string.size()>0)
            {
                for (size_t j = 0; j < split_string.at(0).length(); j++)
                {
                    split_string.at(0).at(j) = tolower(split_string.at(0).at(j));
                }
                buffer = "";
                for (size_t j = 0; j < split_string.size(); j++)
                {
                    buffer += split_string.at(j) + " ";
                }
                buffer.erase(buffer.end()-1);
            }
            bool flag = checkForCapitalWord(buffer);
            if (!flag)
            {
                output_line.push_back(buffer);
            }
        }
        file.close();

        output = loadOutput(output_line);

        file.open(filename, std::ofstream::out | std::ofstream::trunc);
        file << output.str();
        file.close();
    }
}

void MessageAnalyzer::cleanupExtraSpaces(const std::vector<std::string> &message_types)
{
    std::string filename;
    std::fstream file;
    std::stringstream output;
    std::string buffer;
    std::vector<std::string> output_lines;

    for (size_t i = 0; i < message_types.size(); i++)
    {
        output = std::stringstream("");
        output_lines.clear();
        filename = parser_files_path + "msg_data/" + messageTypesToSnake(message_types.at(i)) + ".log";
        std::cout<<message_types.at(i)<<std::endl;
        std::cout<<i<<std::endl;
        file.open(filename);
        while (!file.eof())
        {
            getline(file, buffer);
            std::cout<<buffer<<std::endl;
            bool passed_starting_spaces = false;
            bool passed_field_type = false;
            for (size_t i = 0; i < buffer.length(); i++)
            {
                if ((buffer.at(i) != ' ' || buffer.at(i) != '\t') && !passed_starting_spaces)
                {
                    passed_starting_spaces = true;
                }
                if (passed_starting_spaces && buffer.at(i) == ' ')
                {
                    passed_field_type = true;
                }
                if (passed_field_type && buffer.at(i) == ' ')
                {
                    while (buffer.at(i) == ' ' || buffer.at(i) == '\t')
                    {
                        buffer.erase(buffer.begin() + i);
                    }
                }
                if (passed_field_type && buffer.at(i) != ' ')
                {
                    buffer.insert(i, "#");
                    break;
                }
            }

            output_lines.push_back(buffer);
        }
        file.close();

        output = loadOutput(output_lines);
        file.open(filename, std::ofstream::out | std::ofstream::trunc);
        file << output.str();
        file.close();
    }
}

void MessageAnalyzer::collapseTabs(const std::vector<std::string> &message_types)
{
    std::string filename;
    std::fstream file;
    std::stringstream output;
    std::string buffer;
    std::vector<std::string> output_lines;

    for (size_t i = 0; i < message_types.size(); i++)
    {
        filename = parser_files_path + "msg_data/" + messageTypesToSnake(message_types.at(i)) + ".log";
        file.open(filename);
        output = std::stringstream("");
        output_lines.clear();

        while (!file.eof())
        {
            getline(file, buffer);
            for (size_t i = 0; i < buffer.length(); i++)
            {
                while (buffer.at(i) == '\t')
                {
                    buffer.erase(buffer.begin() + i);
                    buffer.insert(i, "!");
                }
            }
            output_lines.push_back(buffer);
        }
        file.close();

        output = loadOutput(output_lines);
        file.open(filename, std::ofstream::out | std::ofstream::trunc);
        file << output.str();
        file.close();
    }
}

void MessageAnalyzer::prepForSplit(const std::vector<std::string> &message_types)
{
    std::string filename;
    std::fstream file;
    std::stringstream output;
    std::string buffer;
    std::vector<std::string> output_lines;

    for (size_t i = 0; i < message_types.size(); i++)
    {
        filename = parser_files_path + "msg_data/" + messageTypesToSnake(message_types.at(i)) + ".log";
        file.open(filename);
        output = std::stringstream("");
        output_lines.clear();

        while (!file.eof())
        {
            getline(file, buffer);
            for (size_t i = 0; i < buffer.length(); i++)
            {
                if (buffer.at(i) != '!')
                {
                    buffer.insert(i, "#");
                    break;
                }
            }
            if (buffer != "")
            {
                output_lines.push_back(buffer);
            }
        }
        file.close();
        output = loadOutput(output_lines);
        file.open(filename, std::ofstream::out | std::ofstream::trunc);
        file << output.str();
        file.close();
    }
}

bool MessageAnalyzer::checkForArray(const std::string &str)
{
for (size_t i = 0; i < str.length(); i++)
    {
        if (str.at(i) == '[')
        {
            return true;
        }
    }
    return false;}

void MessageAnalyzer::setupFieldNamespaces(const std::vector<std::string> &message_types)
{
    std::string filename;
    std::fstream file;
    std::stringstream output;
    std::string buffer;
    std::vector<std::string> output_lines;
    std::vector<std::string> current_namespace;
    std::vector<std::string> namespace_buffer;
    std::vector<std::string> split_string;
    std::vector<std::string> array_symbols;

    std::string temp;

    for (size_t i = 0; i < message_types.size(); i++)
    {
        filename = parser_files_path + "msg_data/" + messageTypesToSnake(message_types.at(i)) + ".log";
        file.open(filename);
        output = std::stringstream("");
        output_lines.clear();
        current_namespace.clear();

        std::cout<<filename<<std::endl;

        while (!file.eof())
        {
            getline(file, buffer);

            //split_string = split(buffer, '#');
            boost::split(split_string, buffer, boost::is_any_of("#"));
            if (buffer != "")
            {
                if (split_string.at(0).length() >= current_namespace.size())
                {
                    current_namespace.push_back(split_string.at(2));
                }
                current_namespace.at(split_string.at(0).length()) = split_string.at(2);
                temp = "";
                for (size_t i = 0; i < split_string.at(0).length(); i++)
                {
                    temp += current_namespace.at(i) + "!";
                }
                buffer = split_string.at(1) + "#" + temp + split_string.at(2);
                if (split_string.size() == 4)
                {
                    buffer += "#" + split_string.at(3);
                }
            }
            output_lines.push_back(buffer);
        }
        file.close();
        output = loadOutput(output_lines);
        file.open(filename, std::ofstream::out | std::ofstream::trunc);
        file << output.str();
        file.close();
    }
}

void MessageAnalyzer::clearArrays(const std::vector<std::string> &message_types)
{
    std::string filename;
    std::fstream file;
    std::stringstream output;
    std::string buffer;
    std::vector<std::string> output_lines;
    std::vector<std::string> split_string;

    for (size_t i = 0; i < message_types.size(); i++)
    {
        filename = parser_files_path + "msg_data/" + messageTypesToSnake(message_types.at(i)) + ".log";
        file.open(filename);
        output = std::stringstream("");
        std::vector<std::string> split_string;
        bool has_array;

        output_lines.clear();

        while (!file.eof())
        {
            getline(file, buffer);
            has_array = false;

            for (size_t i = 0; i < buffer.length(); i++)
            {
                if (buffer.at(i) == '[')
                {
                    has_array = true;
                    while (buffer.at(i + 1) != ']')
                    {
                        buffer.erase(buffer.begin() + i + 1);
                    }
                    // split_string = split(buffer, '#');
                    // buffer.append("_array");
                    // buffer = split_string.at(0) + "#" + split_string.at(1).append("_array") + "#" + split_string.at(2);
                }
            }

            if (has_array)
            {
                //split_string = split(buffer, '#');
                boost::split(split_string, buffer, boost::is_any_of("#"));
                buffer = split_string.at(0) + "#" + split_string.at(1).append("_array") + "#" + split_string.at(2);
            }

            output_lines.push_back(buffer);
        }
        file.close();
        output = loadOutput(output_lines);
        file.open(filename, std::ofstream::out | std::ofstream::trunc);
        file << output.str();
        file.close();
    }
}

void MessageAnalyzer::applyArraysToSubFields(const std::vector<std::string> &message_types)
{
    std::string filename;
    std::fstream file;
    std::stringstream output;
    std::string buffer;
    std::vector<std::string> output_lines;
    std::vector<std::string> split_string;
    std::vector<int> levels;
    std::string adjusted_output;
    int level;
    int line;
    int array_level;

    //int lines;
    int max_level;

    for (size_t i = 0; i < message_types.size(); i++)
    {
        output = std::stringstream("");
        filename = parser_files_path + "msg_data/" + messageTypesToSnake(message_types.at(i)) + ".log";
        max_level = 0;

        std::cout<<filename<<std::endl;

        // 1st pass - getting file size vars
        file.open(filename);
        levels.clear();
        while (!file.eof())
        {
            getline(file, buffer);
            //split_string = split(buffer, '#');
            boost::split(split_string, buffer, boost::is_any_of("#"));
            levels.push_back(split_string.at(0).length());
            if ((int) split_string.at(0).length() > max_level)
            {
                max_level = split_string.at(0).length();
            }
        }
        file.close();
        //lines = levels.size();

        // 2nd pass - get array layout data
        std::vector<std::vector<int>>layout_data;
        file.open(filename);
        levels.clear();
        line = 0;

        while (!file.eof())
        {
            getline(file, buffer);
            //split_string = split(buffer, '#');
            boost::split(split_string, buffer, boost::is_any_of("#"));
            level = split_string.at(0).length();
            if (line == 0)
            {
                for (int j = 0; j <= max_level; j++)
                {
                    layout_data[line][j] = 0;
                }
            }
            else
            {
                for (int j = 0; j <= max_level; j++)
                {
                    layout_data[line][j] = layout_data[line - 1][j];
                }
            }
            for (int j = 0; j <= max_level; j++)
            {
                if (j >= level)
                {
                    layout_data[line][j] = 0;
                }
            }
            if (split_string.at(1).at(split_string.at(1).length() - 1) == ']')
            {
                layout_data[line][level] = 1;
            }
            line++;
        }
        file.close();

        // 3rd pass - add vector symbols
        file.open(filename);
        level = 0;
        output_lines.clear();

        int line_sum;
        line = 0;
        while (!file.eof())
        {
            getline(file, buffer);
            //split_string = split(buffer, '#');
            boost::split(split_string, buffer, boost::is_any_of("#"));
            level = split_string.at(0).length();

            line_sum = 0;

            for (int j = 0; j <= max_level; j++)
            {
                line_sum += layout_data[line][j];
            }

            if (line_sum == 0)
            {
                output_lines.push_back(buffer);
            }
            else if (line_sum == 1)
            {
                if (layout_data[line][level] == 1)
                {
                    array_level = level;
                    adjusted_output = split_string.at(0) + "#" + split_string.at(1) + "#" + split_string.at(2) + "#" + std::to_string(array_level);
                    output_lines.push_back(adjusted_output);
                }
                else
                {
                    for (int j = 0; j <= max_level; j++)
                    {
                        if (layout_data[line][level] == 1)
                        {
                            array_level = level;
                        }
                    }
                    adjusted_output = split_string.at(0) + "#" + split_string.at(1).append("[]") + "#" + split_string.at(2) + "#" + std::to_string(array_level);
                    output_lines.push_back(adjusted_output);
                }
            }
            else
            {
                adjusted_output = split_string.at(0) + "#" + split_string.at(1).append("@") + "#" + split_string.at(2);
                //output_lines.push_back(adjusted_output);
            }

            line++;
        }
        file.close();

        output = loadOutput(output_lines);
        file.open(filename, std::ofstream::out | std::ofstream::trunc);
        file << output.str();
        file.close();
    }
}

void MessageAnalyzer::addArrayInfo(const std::vector<std::string> &message_types)
{
    std::string filename;
    std::fstream file;
    std::stringstream output;
    std::string buffer;
    std::vector<std::string> output_lines;
    std::vector<std::string> split_string;
    std::vector<std::string> message_stucture;
    std::string temp;
    int level;

    for (size_t i = 0; i < message_types.size(); i++)
    {
        output = std::stringstream("");
        filename = parser_files_path + "msg_data/" + messageTypesToSnake(message_types.at(i)) + ".log";
        file.open(filename);
        output_lines.clear();
        while (!file.eof())
        {
            getline(file, buffer);
            //split_string = split(buffer, '#');
            boost::split(split_string, buffer, boost::is_any_of("#"));
            if (split_string.size() == 3)
            {
                level = stoi(split_string.at(2));
                //message_stucture = split(split_string.at(1), '!');
                boost::split(message_stucture, split_string.at(1), boost::is_any_of("!"));
                message_stucture.at(level).append("@");
                if (message_stucture.size() == 1)
                {
                    temp = message_stucture.at(0);
                }
                else
                {
                    temp = message_stucture.at(0);
                    for (size_t j = 1; j < message_stucture.size(); j++)
                    {
                        temp += "!" + message_stucture.at(j);
                    }
                }
                output_lines.push_back(split_string.at(0) + "#" + split_string.at(1) + "#" + temp);
            }
            else
            {
                output_lines.push_back(buffer);
            }
        }
        file.close();

        output = loadOutput(output_lines);
        file.open(filename, std::ofstream::out | std::ofstream::trunc);
        file << output.str();
        file.close();
    }
}

void MessageAnalyzer::removeFieldNesting(const std::vector<std::string> &message_types)
{
    std::string filename;
    std::fstream file;
    std::stringstream output;
    std::string buffer;
    std::vector<std::string> output_lines;
    std::vector<std::string> split_string;

    for (size_t i = 0; i < message_types.size(); i++)
    {
        output = std::stringstream("");
        filename = parser_files_path + "msg_data/" + messageTypesToSnake(message_types.at(i)) + ".log";
        file.open(filename);
        output_lines.clear();
        while (!file.eof())
        {
            getline(file, buffer);
            //split_string = split(buffer, '#');
            boost::split(split_string, buffer, boost::is_any_of("#"));
            if (checkNativeMessageTypes(split_string.at(0)))
            {
                output_lines.push_back(buffer);
            }
        }
        file.close();
        output = loadOutput(output_lines);
        file.open(filename, std::ofstream::out | std::ofstream::trunc);
        file << output.str();
        file.close();
    }
}

void MessageAnalyzer::removeExtraSpaces(const std::vector<std::string> &message_types)
{
    std::string filename;
    std::stringstream output;
    std::string buffer;
    std::vector<std::string> output_lines;
    std::vector<std::string> split_string;
    std::vector<std::string> message_structure;
    std::vector<std::string> space_delim;
    std::fstream file;
    std::string temp;

    for (size_t i = 0; i < message_types.size(); i++)
    {
        output = std::stringstream("");
        filename = parser_files_path + "msg_data/" + messageTypesToSnake(message_types.at(i)) + ".log";
        file.open(filename);
        output_lines.clear();
        while (!file.eof())
        {
            getline(file, buffer);
            //split_string = split(buffer, '#');
            boost::split(split_string, buffer, boost::is_any_of("#"));
            for (size_t j = 0; j < split_string.at(1).length(); j++)
            {
                message_structure.clear();
                space_delim.clear();
                //message_structure = split(split_string.at(1), '!');
                boost::split(message_structure, split_string.at(1), boost::is_any_of("!"));
                //space_delim = split(message_structure.at(message_structure.size()-1), ' ');
                boost::split(space_delim, message_structure.at(message_structure.size()-1), boost::is_any_of(" "));

                if (space_delim.size() != 1)
                {
                    temp = "";
                    for (size_t k = 0; k < space_delim.size() - 1; k++)
                    {
                        temp += space_delim.at(k);
                    }
                    message_structure.at(message_structure.size()-1) = temp;
                    temp = "";
                    for (size_t k = 0; k < message_structure.size(); k++)
                    {
                        temp += message_structure.at(k) + "!";
                    }
                    temp.erase(temp.end()-1);
                    split_string.at(1) = temp;
                }

                if (split_string.at(1).at(j) == ' ' || split_string.at(1).at(j) == '\t')
                {
                    split_string.at(1).erase(split_string.at(1).begin() + j);
                    j = 0;
                }
            }
            if (split_string.size() == 3)
            {
                for (size_t j = 0; j < split_string.at(2).length(); j++)
                {
                    if (split_string.at(2).at(j) == ' ' || split_string.at(2).at(j) == '\t')
                    {
                        split_string.at(2).erase(split_string.at(2).begin() + j);
                        j = 0;
                    }
                }
            }

            if (split_string.size() == 3)
            {
                output_lines.push_back(split_string.at(0) + "#" + split_string.at(1) + "#" + split_string.at(2));
            }
            else
            {
                output_lines.push_back(split_string.at(0) + "#" + split_string.at(1));
            }
        }
        file.close();

        output = loadOutput(output_lines);
        file.open(filename, std::ofstream::out | std::ofstream::trunc);
        file << output.str();
        file.close();
    }
}

bool MessageAnalyzer::checkForFieldType(const std::string &key, const std::string &line)
{
bool beginning_blanks = true;
    for (size_t i=0; i < line.length(); i++)
    {
        if ((line.at(i) == ' ' || line.at(i) == '\t') && beginning_blanks)
        {
            // continue
        }
        else
        {
            beginning_blanks = false;

            if(key == line.substr(i, key.length()))
            {
                return true;
            }
            else
            {
                return false;
            }        
        }
    }
    return false;}

void MessageAnalyzer::scanForUniqueFields(const std::vector<std::string> &message_types)
{
    std::string filename;
    std::fstream file;
    std::stringstream output;
    std::string buffer;
    std::vector<std::string> output_line;

    for (size_t i = 0; i < message_types.size(); i++)
    {
        output_line.clear();

        filename = parser_files_path + "msg_data/" + messageTypesToSnake(message_types.at(i)) + ".log";
        file.open(filename);
        while (!file.eof())
        {
            getline(file, buffer);

            if (checkForFieldType("Quaternion", buffer))
            {
                output_line.push_back(buffer);
                for (size_t i = 0; i < 4; i++)
                {
                    getline(file, buffer);
                    buffer.erase(buffer.end()-1);
                    output_line.push_back(buffer);
                }
            }
            else
            {
                output_line.push_back(buffer);
            }
        }
        file.close();

        output = loadOutput(output_line);

        file.open(filename, std::ofstream::out | std::ofstream::trunc);
        file << output.str();
        file.close();
    }
}
