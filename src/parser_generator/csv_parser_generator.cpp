#include "bagfile_parser_qt/parser_generator/csv_parser_generator.hpp"

CsvParserGenerator::CsvParserGenerator(const std::string &output_path, const std::string &bag_filename, const std::string &data_output_path, const std::string &storage_type)
{
    this->output_path = output_path;
    this->output_package_path = output_path + "/src/rosbag2_parser/";
    this->data_output_path = data_output_path + "/";
    this->bag_filename = bag_filename;
    this->storage_type = storage_type;

    this->loadVectors();
    this->setupFiles();

    this->writeCMakeLists();
    this->writePackageXml();
    this->writeHeader();
    this->writeSource();
    this->writeConfig();
    this->writeLaunch();
    this->writeMain();

    std::cout << "done" << std::endl;
}

CsvParserGenerator::~CsvParserGenerator()
{
}

void CsvParserGenerator::loadVectors()
{
    depends = readFile(output_path + "/files/depends.txt");

    std::vector<std::string> buffer;
    std::string temp;
    buffer.clear();

    msg_header_names.clear();
    msg_vars.clear();
    msg_support_vars.clear();

    // Message Headers and Vars
    buffer = readFile(output_path + "/files/parser_files/message_types.txt");
    for (std::string item : buffer)
    {
        msg_header_names.push_back(item);
        item = slashToUnderscore(item);
        msg_vars.push_back(item + "_msg");
        msg_support_vars.push_back(slashToUnderscore(item) + "_support");
    }

    // Output Filename Vars, Filename Strings, File Vars
    buffer.clear();
    buffer = readFile(output_path + "/files/parser_files/topic_names.txt");
    output_filename_vars.clear();
    output_file_vars.clear();
    output_filename_strings.clear();

    for (std::string item : buffer)
    {
        item.erase(item.begin());
        item = slashToUnderscore(item);
        output_filename_vars.push_back(item + "_filename");
        output_file_vars.push_back(item + "_file");
        output_filename_strings.push_back(item + ".csv");
    }

    // Load Message Variable Types
    buffer.clear();
    msg_types.clear();
    buffer = readFile(output_path + "/files/parser_files/message_names.txt");
    for (std::string item : buffer)
    {
        msg_types.push_back(slashToColon(item));
    }

    if (msg_types.size() != msg_support_vars.size())
    {
        std::cout << "Error: Message config" << std::endl;
    }

    MsgSupportPair msg_support_pair_buffer;
    TopicSortingData topic_sorting_data_buffer;
    for (size_t i = 0; i < msg_types.size(); ++i)
    {
        msg_support_pair_buffer.msg_type = msg_types.at(i);
        msg_support_pair_buffer.msg_support_var = slashToUnderscore(msg_support_vars.at(i));
        msg_support_pairs.push_back(msg_support_pair_buffer);
    }

    buffer.clear();
    buffer = readFile(output_path + "/files/parser_files/topic_data.txt");
    std::vector<std::string> split_string;

    for (std::string item : buffer)
    {
        split_string.clear();
        boost::split(split_string, item, boost::is_any_of("#"));
        topic_sorting_data_buffer.topic_name = split_string.at(0);
        topic_sorting_data_buffer.msg_var = slashToUnderscore(split_string.at(1)) + "_msg";
        topic_sorting_data_buffer.msg_support_var = slashToUnderscore(split_string.at(1)) + "_support";
        topic_sorting_data_buffer.msg_type = slashToColon(split_string.at(2));
        topic_sorting_data.push_back(topic_sorting_data_buffer);
    }
}

void CsvParserGenerator::setupFiles()
{
    if (std::filesystem::exists(output_package_path))
    {
        std::string command = "rm -r " + output_package_path;
        if (system(command.c_str()))
        {
            std::cout << "Somthing went wrong clearing old parser" << std::endl;
            std::cout << "Run: " << command << std::endl;
        }
    }

    // Create Package
    std::string command = "mkdir -p " + output_package_path;
    if (system(command.c_str()))
    {
        std::cout << "Something went wrong creating parser package" << std::endl;
    }

    // Create Directories
    command = "mkdir -p " + output_package_path + "src/";
    if (system(command.c_str()))
    {
        std::cout << "Something went wrong creating src" << std::endl;
    }

    command = "mkdir -p " + output_package_path + "include/rosbag2_parser/";
    if (system(command.c_str()))
    {
        std::cout << "Something went wrong creating include" << std::endl;
    }

    command = "mkdir -p " + output_package_path + "launch/";
    if (system(command.c_str()))
    {
        std::cout << "Something went wrong creating launch" << std::endl;
    }

    command = "mkdir -p " + output_package_path + "config/";
    if (system(command.c_str()))
    {
        std::cout << "Something went wrong creating config" << std::endl;
    }

    package_xml_filename = output_package_path + "package.xml";
    cmake_lists_filename = output_package_path + "CMakeLists.txt";
    source_filename = output_package_path + "src/rosbag2_parser.cpp";
    header_filename = output_package_path + "include/rosbag2_parser/rosbag2_parser.hpp";
    launch_filename = output_package_path + "launch/rosbag2_parser.launch.py";
    config_filename = output_package_path + "config/params.yaml";
    main_filename = output_package_path + "src/main.cpp";

    command = "touch " + package_xml_filename;
    if (system(command.c_str()))
    {
        std::cout << "Something went wrong creating package.xml" << std::endl;
    }

    command = "touch " + cmake_lists_filename;
    if (system(command.c_str()))
    {
        std::cout << "Something went wrong creating CMakeLists.txt" << std::endl;
    }

    command = "touch " + source_filename;
    if (system(command.c_str()))
    {
        std::cout << "Something went wrong creating class.cpp file" << std::endl;
    }

    command = "touch " + header_filename;
    if (system(command.c_str()))
    {
        std::cout << "Something went wrong creating class.hpp file" << std::endl;
    }

    command = "touch " + launch_filename;
    if (system(command.c_str()))
    {
        std::cout << "Something went wrong creating launch file" << std::endl;
    }

    command = "touch " + config_filename;
    if (system(command.c_str()))
    {
        std::cout << "Something went wrong creating config file" << std::endl;
    }

    command = "touch " + main_filename;
    if (system(command.c_str()))
    {
        std::cout << "Something went wrong creating main.cpp" << std::endl;
    }
}

void CsvParserGenerator::writeCMakeLists()
{
    std::stringstream output;
    std::string package_name = "rosbag2_parser";

    output << "cmake_minimum_required(VERSION 3.8)\n";
    output << "project(" + package_name + ")\n\n";
    output << "if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES \"Clang\")\n  add_compile_options(-Wall -Wextra -Wpedantic)\nendif()\n\n";
    output << "find_package(ament_cmake REQUIRED)\n";

    output << "find_package(rclcpp)\n";
    output << "find_package(rosbag2_cpp)\n";
    std::string depends_list;
    for (size_t i = 0; i < depends.size(); i++)
    {
        output << "find_package(" + depends.at(i) + ")\n";
        depends_list.append(" " + depends.at(i));
    }

    output << "\ninclude_directories(include)\n\n";
    output << "add_executable(" + package_name + " src/main.cpp src/" + package_name + ".cpp)\n";
    output << "ament_target_dependencies(" + package_name + " rclcpp rosbag2_cpp" + depends_list + ")\n\n";

    output << "install(\n  TARGETS " + package_name + "\n  DESTINATION lib/${PROJECT_NAME}/\n)\n\n";
    output << "install(\n  DIRECTORY launch\n  DESTINATION share/${PROJECT_NAME}/\n)\n\n";
    output << "install(\n  DIRECTORY config\n  DESTINATION share/${PROJECT_NAME}/\n)\n\n";
    output << "if(BUILD_TESTING)\n  find_package(ament_lint_auto REQUIRED)\n\n  ament_lint_auto_find_test_dependencies()\nendif()\n\n";
    output << "ament_package()\n\n";

    cmake_lists_file.open(cmake_lists_filename, std::ios_base::trunc);
    cmake_lists_file << output.str();
    cmake_lists_file.close();
}

void CsvParserGenerator::writePackageXml()
{
    std::string package_name = "rosbag2_parser";
    std::stringstream output;
    output << "<?xml version=\"1.0\"?>\n";
    output << "<?xml-model href=\"http://download.ros.org/schema/package_format3.xsd\" schematypens=\"http://www.w3.org/2001/XMLSchema\"?>\n";
    output << "<package format=\"3\">\n";
    output << "  <name>" + package_name + "</name>\n  <version>0.0.0</version>\n  <description>TODO: Package description</description>\n  <maintainer email=\"user@todo.todo\">TODO</maintainer>\n  <license>TODO: License declaration</license>\n\n";
    output << "  <buildtool_depend>ament_cmake</buildtool_depend>\n\n";

    output << "  <depend>rclcpp</depend>\n";
    output << "  <depend>rosbag2_cpp</depend>\n";

    for (size_t i = 0; i < depends.size(); i++)
    {
        output << "  <depend>" + depends.at(i) + "</depend>\n";
    }

    output << "\n  <test_depend>ament_lint_auto</test_depend>\n  <test_depend>ament_lint_common</test_depend>\n\n";
    output << "  <export>\n    <build_type>ament_cmake</build_type>\n  </export>\n";
    output << "</package>";

    package_xml_file.open(package_xml_filename, std::ios_base::trunc);
    package_xml_file << output.str();
    package_xml_file.close();
}

void CsvParserGenerator::writeHeader()
{
    std::string package_name = "rosbag2_parser";
    std::string class_name = "Rosbag2Parser";
    std::stringstream output;
    output << "#ifndef " + package_name + "_hpp\n";
    output << "#define " + package_name + "_hpp\n\n";

    output << "#include \"rclcpp/rclcpp.hpp\"\n";
    output << "#include <rosbag2_cpp/readers/sequential_reader.hpp>\n\n";

    for (size_t i = 0; i < msg_header_names.size(); i++)
    {
        output << "#include \"" + msg_header_names.at(i) + ".hpp\"\n";
    }

    output << "\n#include <fstream>\n";
    output << "#include <string>\n";
    output << "#include <iostream>\n";

    // output << "\n#include \"TinyMAT/tinymatwriter.h\"\n";

    output << "\n\nclass " + class_name + " : public rclcpp::Node\n";
    output << "{\n\tpublic:\n";
    output << "\t" + class_name + "();\n";
    output << "\t~" + class_name + "();\n";

    output << "\n\tprivate:\n";
    output << "\n\t//Bag Reader and Serializer\n";
    output << "\trosbag2_cpp::readers::SequentialReader* reader;\n";
    output << "\trosbag2_storage::StorageOptions storage_options{};\n";
    output << "\trosbag2_cpp::ConverterOptions converter_options{};\n";
    output << "\trosbag2_cpp::SerializationFormatConverterFactory factory;\n";
    output << "\tstd::unique_ptr<rosbag2_cpp::converter_interfaces::SerializationFormatDeserializer> cdr_deserializer;\n\n";

    output << "\t//Output File Names\n";
    output << "\tstd::string path;\n";

    std::string temp;
    for (TopicSortingData data : topic_sorting_data)
    {
        temp = data.topic_name;
        temp.erase(temp.begin());
        output << "\tstd::ofstream " + slashToUnderscore(temp) + ";\n";
    }

    output << "\n\t//Message Support\n";
    for (size_t i = 0; i < msg_support_vars.size(); i++)
    {
        output << "\tconst rosidl_message_type_support_t * " + msg_support_vars.at(i) + ";\n";
    }

    output << "\n\t//Message Structures\n";
    for (size_t i = 0; i < msg_types.size(); i++)
    {
        output << "\t" + msg_types.at(i) + " " + msg_vars.at(i) + ";\n";
    }

    output << "\n\t//Functions\n";

    output << "\tvoid parseBag();\n\n";

    for (size_t i = 0; i < topic_sorting_data.size(); i++)
    {
        temp = topic_sorting_data.at(i).topic_name;
        temp.erase(temp.begin());

        output << snakeToCamel("\tvoid parse_" + slashToUnderscore(temp)) + "(std::shared_ptr<rosbag2_storage::SerializedBagMessage> serialized_message);\n";
    }

    output << "};\n\n#endif";

    header_file.open(header_filename, std::ios_base::trunc);
    header_file << output.str();
    header_file.close();
}

void CsvParserGenerator::writeSource()
{
    std::string package_name = "rosbag2_parser";
    std::string class_name = "Rosbag2Parser";
    std::stringstream output;
    std::string temp;
    std::vector<std::string> split_string;

    output << "#include \"" + package_name + "/" + package_name + ".hpp\"\n\n";

    output << class_name + "::" + class_name + "() : Node(\"rosbag2_parser\")\n{\n";

    std::ifstream bag_file_path;
    bag_file_path.open("../config/path.txt");
    getline(bag_file_path, temp);
    boost::split(split_string, temp, boost::is_any_of("/"));
    temp.erase(temp.end() - split_string.at(split_string.size() - 1).length(), temp.end());
    std::string path = temp;

    output << "\tthis->declare_parameter(\"output_file_path\", \"output\");\n";
    output << "\tthis->declare_parameter(\"bagfile\", \"bag\");\n\n";

    output << "\tpath = this->get_parameter(\"output_file_path\").as_string();\n";
    output << "\tstd::string bag_filename = this->get_parameter(\"bagfile\").as_string();\n\n";

    output << "\tthis->reader = new rosbag2_cpp::readers::SequentialReader();\n";
    output << "\tthis->storage_options.uri = bag_filename;\n";
    output << "\tthis->storage_options.storage_id = \"" + storage_type + "\";\n";
    output << "\tthis->converter_options.input_serialization_format = \"cdr\";\n";
    output << "\tthis->converter_options.output_serialization_format = \"cdr\";\n";
    output << "\tthis->cdr_deserializer = factory.load_deserializer(\"cdr\");\n";

    output << "\n\t//Setup Message Support\n";
    for (size_t i = 0; i < msg_support_pairs.size(); i++)
    {
        output << "\tthis->" + msg_support_pairs.at(i).msg_support_var + " = rosidl_typesupport_cpp::get_message_type_support_handle<" + msg_support_pairs.at(i).msg_type + ">();\n";
    }
    output << "\n";

    for (size_t i = 0; i < topic_sorting_data.size(); ++i)
    {
        temp = topic_sorting_data.at(i).topic_name;
        temp.erase(temp.begin());
        output << "\tthis->" + slashToUnderscore(temp) + ".open(path + \"/" + output_filename_strings.at(i) + "\", std::ios_base::trunc);\n";
    }

    output << "\n\tthis->parseBag();";

    output << "\n}\n\n";

    // Destructor
    output << class_name + "::~" + class_name + "()\n";
    output << "{\n\tdelete reader;\n\n";
    for (size_t i = 0; i < topic_sorting_data.size(); ++i)
    {
        temp = topic_sorting_data.at(i).topic_name;
        temp.erase(temp.begin());
        output << "\t" + slashToUnderscore(temp) + ".close();\n";
    }
    output << "}\n\n";

    output << "void Rosbag2Parser::parseBag()\n{\n";

    output << "\treader->open(storage_options, converter_options);\n";
    output << "\n\twhile(reader->has_next())\n\t{\n";

    output << "\t\tstd::shared_ptr<rosbag2_storage::SerializedBagMessage> serialized_message = reader->read_next();\n\n";
    for (size_t i = 0; i < topic_sorting_data.size(); ++i)
    {
        output << "\t\tif (serialized_message->topic_name == \"" + topic_sorting_data.at(i).topic_name + "\")\n";
        output << "\t\t{\n";
        temp = topic_sorting_data.at(i).topic_name;
        temp.erase(temp.begin());

        output << snakeToCamel("\t\t\tparse_" + slashToUnderscore(temp)) + "(serialized_message);\n";
        output << "\t\t}\n";
    }

    output << "\t}\n";

    output << "\tstd::cout<<\"Parser Finished\"<<std::endl;\n";
    output << "}\n\n";

    std::string message_data_filename;
    std::string buffer;
    std::ifstream message_data_file;
    std::vector<std::string> types;
    std::vector<std::string> field_names;
    std::vector<std::string> field_array;
    std::vector<std::string> message_structures;

    for (size_t i = 0; i < topic_sorting_data.size(); ++i)
    {
        temp = topic_sorting_data.at(i).topic_name;
        temp.erase(temp.begin());
        output << "void Rosbag2Parser::" + snakeToCamel("parse_" + slashToUnderscore(temp)) + "(std::shared_ptr<rosbag2_storage::SerializedBagMessage> serialized_message)\n";
        output << "{\n";

        temp = topic_sorting_data.at(i).msg_var;
        temp.erase(temp.end() - 4, temp.end());
        message_data_filename = output_path + "/files/parser_files/msg_data/" + temp + ".log";
        message_data_file.open(message_data_filename);

        types.clear();
        field_names.clear();
        split_string.clear();
        field_array.clear();
        message_structures.clear();
        while (!message_data_file.eof())
        {
            getline(message_data_file, buffer);
            boost::split(split_string, buffer, boost::is_any_of("#"));
            if (split_string.size() > 1)
            {
                types.push_back(convertFieldTypes(split_string.at(0)));
                field_names.push_back(split_string.at(1));
                if (split_string.size() == 3)
                {
                    temp = split_string.at(2);
                    field_array.push_back(bangToDot(temp));
                }
                else
                {
                    field_array.push_back("");
                }
                message_structures.push_back(bangToDot(split_string.at(1)));
            }
        }
        message_data_file.close();

        temp = topic_sorting_data.at(i).topic_name;
        temp.erase(temp.begin());
        
        output << "\tauto ros_message = std::make_shared<rosbag2_cpp::rosbag2_introspection_message_t>();\n\n";
        output << "\tros_message->message = &" + topic_sorting_data.at(i).msg_var + ";\n";
        output << "\tcdr_deserializer->deserialize(serialized_message, " + topic_sorting_data.at(i).msg_support_var + ", ros_message);\n";
        output << "\tstd::string output_string = \"\";\n\n";

        for (size_t j = 0; j < field_names.size(); j++)
        {
            if (field_array.at(j) == "")
            {
                //output << "\t" + bangToUnderscore(field_names.at(j)) + ".push_back(" + topic_sorting_data.at(i).msg_var + "." + message_structures.at(j) + ");\n";
                if (types.at(j) == "string")
                {
                    output << "\toutput_string += " + topic_sorting_data.at(i).msg_var + "." + message_structures.at(j) + ";\n";
                }
                else 
                {
                    output << "\toutput_string += std::to_string(" + topic_sorting_data.at(i).msg_var + "." + message_structures.at(j) + ");\n";
                }

                output << "\toutput_string += \",\";\n";
            }
            else
            {
                //boost::split(split_string, field_array.at(j), boost::is_any_of("@"));
                split_string = split(field_array.at(j), '@');
                if (split_string.size() == 1)
                {
                    split_string.at(0).erase(split_string.at(0).end() - 6, split_string.at(0).end());
                }


                /*    
                if (types.at(j) == "string")
                {
                    //output << "\t\t" + bangToUnderscore(field_names.at(j)) + ".push_back(std::to_string(" + topic_sorting_data.at(i).msg_var + "." + split_string.at(0) + ".size()));\n";
                    output << "\toutput_string += " + topic_sorting_data.at(i).msg_var + "." + split_string.at(0) + ".size();\n";
                }
                else
                {
                    //output << "\t\t" + bangToUnderscore(field_names.at(j)) + ".push_back(" + topic_sorting_data.at(i).msg_var + "." + split_string.at(0) + ".size());\n";
                    output << "\toutput_string += std::to_string(" + topic_sorting_data.at(i).msg_var + "." + split_string.at(0) + ".size());\n";
                }
                */

                output << "\toutput_string += std::to_string(" + topic_sorting_data.at(i).msg_var + "." + split_string.at(0) + ".size());\n";
                output << "\toutput_string += \",\";\n";

                output << "\tfor (size_t j = 0; j < " + topic_sorting_data.at(i).msg_var + "." + split_string.at(0) + ".size(); j++)\n\t{\n";

                //boost::split(split_string, field_array.at(j), boost::is_any_of("@"));
                split_string = split(field_array.at(j), '@');
                if (split_string.size() > 1)
                {
                    temp = split_string.at(0) + ".at(j)" + split_string.at(1);
                    temp.erase(temp.end() - 6, temp.end());
                }
                else
                {
                    temp = split_string.at(0);
                    temp.erase(temp.end() - 6, temp.end());
                    temp.append(".at(j)");
                }

                //output << "\t\t" + bangToUnderscore(field_names.at(j)) + ".push_back(" + topic_sorting_data.at(i).msg_var + "." + temp + ");\n";
                if (types.at(j) == "string")
                {
                    output << "\t\toutput_string += " + topic_sorting_data.at(i).msg_var + "." + temp + ";\n";
                }
                else
                {
                    output << "\t\toutput_string += std::to_string(" + topic_sorting_data.at(i).msg_var + "." + temp + ");\n";
                }
                output << "\t\toutput_string += \",\";\n";
                output << "\t}\n";
            }
        }

        output << "\toutput_string += \"\\n\";\n";

        temp = topic_sorting_data.at(i).topic_name;
        temp.erase(temp.begin());
        output << "\n\t" + slashToUnderscore(temp) + " << output_string;\n";

        output << "}\n\n";
    }

    source_file.open(source_filename, std::ios_base::trunc);
    source_file << output.str();
    source_file.close();
}

void CsvParserGenerator::writeConfig()
{
    // Get Path from Bagfile
    std::vector<std::string> split_string;
    boost::split(split_string, bag_filename, boost::is_any_of("/"));
    std::string filepath = "";
    for (size_t i = 0; i < split_string.size()-1; ++i)
    {
        filepath += split_string.at(i) + "/";
    }

    std::stringstream output;
    output << "rosbag2_parser:\n";
    output << "  ros__parameters:\n";
    output << "    output_file_path: \"" + data_output_path + "\"\n";
    output << "    bagfile: \"" + bag_filename + "\"\n";

    config_file.open(config_filename, std::ios_base::trunc);
    config_file << output.str();
    config_file.close();
}

void CsvParserGenerator::writeLaunch()
{
    std::stringstream output;
    std::string package_name = "rosbag2_parser";
    output << "import os\n";
    output << "from launch import LaunchDescription\n";
    output << "from launch_ros.actions import Node\n";
    output << "from launch.actions import IncludeLaunchDescription\n";
    output << "from ament_index_python.packages import get_package_share_directory\n\n\n";

    output << "def generate_launch_description():\n\n";
    output << "\tconfig_path = os.path.join(\n";
    output << "\t\tget_package_share_directory(\'" + package_name + "\'),\n";
    output << "\t\t\'config\'\n\t\t)\n\n";

    output << "\ttemplate_node_description = Node(\n";
    output << "\t\t\tpackage=\'" + package_name + "\',\n";
    output << "\t\t\texecutable=\'" + package_name + "\',\n";
    output << "\t\t\tname=\'" + package_name + "\',\n";
    output << "\t\t\toutput=\'screen\', \n";
    output << "\t\t\tparameters=[os.path.join(config_path, \'params.yaml\')]\n\t\t)\n\n";

    output << "\treturn LaunchDescription([\n";
    output << "\t\ttemplate_node_description,\n";
    output << "\t])";

    launch_file.open(launch_filename);
    launch_file << output.str();
    launch_file.close();
}

void CsvParserGenerator::writeMain()
{
    std::stringstream output;
    std::string package_name = "rosbag2_parser";
    std::string class_name = "Rosbag2Parser";

    output << "#include \"" + package_name + "/" + package_name + ".hpp\"\n\n";
    output << "int main(int argc, char *argv[])\n{\n";
    output << "\tstd::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();\n";
    output << "\trclcpp::init(argc, argv);\n";
    output << "\tstd::make_shared<Rosbag2Parser>();\n";
    output << "\trclcpp::shutdown();\n";
    output << "\tstd::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();\n";
    output << "\tstd::cout << \"Total Runtime: \" << std::chrono::duration_cast<std::chrono::seconds>(end - begin).count() << \" seconds\" << std::endl;\n";
    output << "\treturn 0;\n}";

    main_file.open(main_filename);
    main_file << output.str();
    main_file.close();
}


std::vector<std::string> CsvParserGenerator::readFile(const std::string &filename)
{
    std::vector<std::string> output;
    if (std::filesystem::exists(filename))
    {
        std::ifstream file;
        file.open(filename);

        while (!file.eof())
        {
            std::string line;
            getline(file, line);
            if (!line.empty())
            {
                output.push_back(line);
            }
        }
        file.close();
    }

    return output;
}

std::string CsvParserGenerator::slashToUnderscore(std::string str)
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

std::string CsvParserGenerator::slashToColon(std::string str)
{
    for (size_t i = 0; i < str.length(); i++)
    {
        if (str.at(i) == '/')
        {
            str.at(i) = ':';
            str.insert(i, ":");
        }
    }
    return str;
}

std::string CsvParserGenerator::bangToUnderscore(std::string str)
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

std::string CsvParserGenerator::bangToDot(std::string str)
{
    for (size_t i = 0; i < str.length(); i++)
    {
        if (str.at(i) == '!')
        {
            str.at(i) = '.';
        }
    }

    return str;
}

std::string CsvParserGenerator::snakeToCamel(std::string str)
{
    for (size_t i = 0; i < str.length(); i++)
    {
        if (str[i] == '_')
        {
            str[i + 1] = toupper(str[i + 1]);
            str.erase(str.begin() + i);
        }
    }

    return str;
}

std::string CsvParserGenerator::convertFieldTypes(std::string str)
{
    if (str == "string" || str == "string[]")
    {
        return "string";
    }
    else if (str == "char" || str == "char[]")
    {
        return "string";
    }

    else
    {
        return "double";
    }
}

std::vector<std::string> CsvParserGenerator::split(std::string s, char delim)
{
    std::vector<std::string> result;
    std::stringstream ss(s);
    std::string item;

    while (getline(ss, item, delim))
    {
        result.push_back(item);
    }

    return result;
}