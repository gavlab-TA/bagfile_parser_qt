#include "bagfile_parser_qt/parser_generator/matlab_parser_generator.hpp"

MatlabParserGenerator::MatlabParserGenerator(const std::string &output_path, const std::string &bag_filename, const std::string &data_output_path, const std::string &storage_type)
{
    this->output_path = output_path + "/";
    this->bag_filename = bag_filename;
    this->data_output_path = data_output_path+"/";
    this->storage_type = storage_type;

    loadVectors();
    setupFiles();

    writeCMakeLists();
    writePackageXml();
    writeHeader();
    writeSource();
    writeLaunch();
    writeConfig();
    writeMain();

    std::cout << "Matlab Parser Generator Finished"<<std::endl;
}

MatlabParserGenerator::~MatlabParserGenerator()
{
}

void MatlabParserGenerator::loadVectors()
{
    depends = readBagDataFile(output_path + "files/depends.txt");

    std::vector<std::string> buffer;
    std::string temp;
    buffer.clear();

    // Build Message Headers, Message Support Vars, Message Vars
    msg_header_names.clear();
    msg_vars.clear();
    msg_support_vars.clear();

    buffer = readBagDataFile(output_path + "files/parser_files/message_types.txt");
    for (size_t i = 0; i < buffer.size(); i++)
    {
        msg_header_names.push_back(buffer.at(i) + ".hpp");
        buffer.at(i) = slashToUnderscore(buffer.at(i));
        msg_vars.push_back(buffer.at(i) + "_msg");
        msg_support_vars.push_back(slashToUnderscore(buffer.at(i)) + "_support");
    }

    // Build Output Filename, Filename String and File Vars
    buffer.clear();
    buffer = readBagDataFile(output_path + "files/parser_files/topic_names.txt");
    output_filenames.clear();
    output_files.clear();
    output_filename_strings.clear();
    for (size_t i = 0; i < buffer.size(); i++)
    {
        buffer.at(i).erase(buffer.at(i).begin());
        buffer.at(i) = slashToUnderscore(buffer.at(i));
        output_filenames.push_back(buffer.at(i) + "_filename");
        output_filename_strings.push_back(buffer.at(i) + ".bin");
        output_files.push_back(buffer.at(i) + "_file");
    }

    // Load Message Variable Types
    buffer.clear();
    msg_types.clear();
    buffer = readBagDataFile(output_path + "files/parser_files/message_names.txt");
    for (size_t i = 0; i < buffer.size(); i++)
    {
        msg_types.push_back(slashToColon(buffer.at(i)));
    }

    if (msg_types.size() != msg_support_vars.size())
    {
        std::cout << "Error: Message Config files not same size" << std::endl;
    }

    MsgSupportPair msg_support_pair_buffer;
    TopicSortingData topic_sorting_data_buffer;
    for (size_t i = 0; i < msg_types.size(); i++)
    {
        msg_support_pair_buffer.msg_type = msg_types.at(i);
        msg_support_pair_buffer.msg_support_var = slashToUnderscore(msg_support_vars.at(i));
        msg_support_pairs.push_back(msg_support_pair_buffer);
    }

    buffer.clear();
    buffer = readBagDataFile(output_path + "files/parser_files/topic_data.txt");
    std::vector<std::string> split_string;

    for (size_t i = 0; i < buffer.size(); i++)
    {
        split_string.clear();
        split_string = split(buffer.at(i), '#');
        topic_sorting_data_buffer.topic_name = split_string.at(0);
        topic_sorting_data_buffer.msg_var = slashToUnderscore(split_string.at(1)) + "_msg";
        topic_sorting_data_buffer.msg_support_var = slashToUnderscore(split_string.at(1)) + "_support";
        topic_sorting_data_buffer.msg_type = slashToColon(split_string.at(2));
        topic_sorting_data.push_back(topic_sorting_data_buffer);
    }
}

void MatlabParserGenerator::setupFiles()
{
    package_name = "matlab_parser";
    class_name = package_name;
    class_name[0] = toupper(class_name[0]);
    class_name = snakeToCamel(class_name);

    source_path = output_path + "src/";
    package_path = source_path + package_name;

    package_xml_filename = package_path + "/package.xml";
    cmake_lists_filename = package_path + "/CMakeLists.txt";
    header_filename = package_path + "/include/" + package_name + "/" + package_name + ".hpp";
    source_filename = package_path + "/src/" + package_name + ".cpp";
    config_filename = package_path + "/config/params.yaml";
    launch_filename = package_path + "/launch/" + package_name + ".launch.py";
    main_filename = package_path + "/src/main.cpp";

    std::string create_package_xml_file_command = "touch " + package_xml_filename;
    std::string create_cmake_lists_file_command = "touch " + cmake_lists_filename;
    std::string create_header_file_command = "touch " + header_filename;
    std::string create_source_file_command = "touch " + source_filename;
    std::string create_config_file_command = "touch " + config_filename;
    std::string create_launch_file_command = "touch " + launch_filename;
    std::string create_main_file_command = "touch " + main_filename;
    std::string create_files_command = create_package_xml_file_command + " && " + create_cmake_lists_file_command + " && " + create_header_file_command + " && " + create_source_file_command + " && " + create_config_file_command + " && " + create_launch_file_command + " && " + create_main_file_command;

    std::string temp = "mkdir -p " + package_path;
    system_status = system(temp.c_str());
    if (system_status != 0)
    {
        std::cout << "Error creating package file" << std::endl;
    }

    temp = "mkdir -p " + package_path + "/src && mkdir -p " + package_path + "/include/" + package_name + " && mkdir -p " + package_path + "/config && mkdir -p " + package_path + "/launch";

    system_status = system(temp.c_str());

    system_status = system(create_files_command.c_str());
    if (system_status != 0)
    {
        std::cout << "Error creating package file" << std::endl;
    }
}

void MatlabParserGenerator::writeCMakeLists()
{
    std::stringstream output;

    output << "cmake_minimum_required(VERSION 3.8)\n";
    output << "project(" + package_name + ")\n\n";
    output << "if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES \"Clang\")\n  add_compile_options(-Wall -Wextra -Wpedantic)\nendif()\n\n";
    output << "find_package(ament_cmake REQUIRED)\n";
    output << "find_package(rclcpp REQUIRED)\n";
    output << "find_package(rosbag2_cpp REQUIRED)\n";
    output << "find_package(TinyMAT REQUIRED)\n";

    std::string depends_list;
    for (size_t i = 0; i < depends.size(); i++)
    {
        output << "find_package(" + depends.at(i) + ")\n";
        depends_list.append(" " + depends.at(i));
    }

    output << "\ninclude_directories(include)\n\n";
    output << "add_executable(" + package_name + " src/main.cpp src/" + package_name + ".cpp)\n";
    output << "ament_target_dependencies(" + package_name + depends_list + " rclcpp rosbag2_cpp TinyMAT)\n\n";

    // output << "add_executable(matlab_parser src/matlab_parser.cpp)\n";
    // output << "ament_target_dependencies(matlab_parser" + depends_list + ")\n\n";

    output << "install(\n  TARGETS " + package_name + "\n  DESTINATION lib/${PROJECT_NAME}/\n)\n\n";
    output << "install(\n  DIRECTORY launch\n  DESTINATION share/${PROJECT_NAME}/\n)\n\n";
    output << "install(\n  DIRECTORY config\n  DESTINATION share/${PROJECT_NAME}/\n)\n\n";
    output << "if(BUILD_TESTING)\n  find_package(ament_lint_auto REQUIRED)\n\n  ament_lint_auto_find_test_dependencies()\nendif()\n\n";
    output << "ament_package()\n\n";

    cmake_lists_file.open(cmake_lists_filename);
    cmake_lists_file << output.str();
    cmake_lists_file.close();
}

void MatlabParserGenerator::writePackageXml()
{
    std::stringstream output;
    output << "<?xml version=\"1.0\"?>\n";
    output << "<?xml-model href=\"http://download.ros.org/schema/package_format3.xsd\" schematypens=\"http://www.w3.org/2001/XMLSchema\"?>\n";
    output << "<package format=\"3\">\n";
    output << "  <name>" + package_name + "</name>\n  <version>0.0.0</version>\n  <description>TODO: Package description</description>\n  <maintainer email=\"user@todo.todo\">TODO</maintainer>\n  <license>TODO: License declaration</license>\n\n";
    output << "  <buildtool_depend>ament_cmake</buildtool_depend>\n\n";

    output << "  <depend>rclcpp</depend>\n";
    output << "  <depend>rosbag2_cpp</depend>\n";
    output << "  <depend>TinyMAT</depend>\n";

    for (size_t i = 0; i < depends.size(); i++)
    {
        output << "  <depend>" + depends.at(i) + "</depend>\n";
    }

    output << "\n  <test_depend>ament_lint_auto</test_depend>\n  <test_depend>ament_lint_common</test_depend>\n\n";
    output << "  <export>\n    <build_type>ament_cmake</build_type>\n  </export>\n";
    output << "</package>";

    package_xml_file.open(package_xml_filename);
    package_xml_file << output.str();
    package_xml_file.close();
}

void MatlabParserGenerator::writeHeader()
{
    std::stringstream output;
    output << "#ifndef " + package_name + "_hpp\n";
    output << "#define " + package_name + "_hpp\n\n";

    output << "#include \"rclcpp/rclcpp.hpp\"\n";
    output << "#include <rosbag2_cpp/readers/sequential_reader.hpp>\n\n";

    for (size_t i = 0; i < msg_header_names.size(); i++)
    {
        output << "#include \"" + msg_header_names.at(i) + "\"\n";
    }

    output << "\n#include <fstream>\n";
    output << "#include <string>\n";
    output << "#include <iostream>\n";

    output << "\n#include \"TinyMAT/tinymatwriter.h\"\n";

    output << "\n\nclass " + class_name + "\n";
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
    /*for (size_t i = 0; i < output_filenames.size(); i++)
    {
        output << "\tstd::string " + output_filenames.at(i) + ";\n";
    }*/
    output << "\tstd::string filename;\n";
    output << "\tstd::string path;\n";

    // output << "\n\t//Output File Variables\n";
    /*for (size_t i = 0; i < output_files.size(); i++)
    {
        output << "\tstd::ofstream " + output_files.at(i) + ";\n";
    }*/

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
    // output << "\tbool processMessage(const std::shared_ptr<rosbag2_storage::SerializedBagMessage> serialized_msg, auto ros_message);\n";
    // output << "\tvoid read_bag();\n\n";

    std::string temp;
    for (size_t i = 0; i < topic_sorting_data.size(); i++)
    {
        temp = topic_sorting_data.at(i).topic_name;
        temp.erase(temp.begin());

        output << "\tvoid parse_" + slashToUnderscore(temp) + "();\n";
    }

    output << "};\n\n#endif";

    header_file.open(header_filename);
    header_file << output.str();
    header_file.close();
}

void MatlabParserGenerator::writeSource()
{
    std::stringstream output;
    std::string temp;
    std::vector<std::string> split_string;

    output << "#include \"" + package_name + "/" + package_name + ".hpp\"\n\n";

    output << class_name + "::" + class_name + "()\n{\n";

    // output << "\tthis->declare_parameter(\"path\", \"~/\");\n\tthis->declare_parameter(\"bagfile\", \"data\");\n\n";

    // output << "\tstd::string bag_filename = this->get_parameter(\"path\").as_string() + this->get_parameter(\"bagfile\").as_string();\n\n";
    
    boost::split(split_string, bag_filename, boost::is_any_of("/"));
    temp = "";
    for (size_t i = 0; i < split_string.size()-1; ++i)
    {
        temp += split_string.at(i) + "/";
    }

    output << "\tstd::string bag_filename = \"" + bag_filename + "\";\n";
    output << "\tthis->path = \"" + data_output_path + "\";\n";

    output << "\tthis->reader = new rosbag2_cpp::readers::SequentialReader();\n";
    output << "\tthis->storage_options.uri = bag_filename;\n";
    output << "\tthis->storage_options.storage_id = \"" + storage_type + "\";\n";
    output << "\tthis->converter_options.input_serialization_format = \"cdr\";\n";
    output << "\tthis->converter_options.output_serialization_format = \"cdr\";\n";
    output << "\tthis->cdr_deserializer = factory.load_deserializer(\"cdr\");\n";

    /*output << "\n\t//Output File Names\n";
    for (size_t i = 0; i < output_filenames.size(); i++)
    {
        output << "\t" + output_filenames.at(i) + " = path + \"" + output_filename_strings.at(i) + "\";\n";
    }*/

    /*output << "\n\t//Output File Open\n";
    for (size_t i = 0; i < output_files.size(); i++)
    {
        output << "\t" + output_files.at(i) + ".open(" + output_filenames.at(i) + ", std::ofstream::out | std::ofstream::binary);\n";
    }*/

    output << "\n\t//Setup Message Support\n";
    for (size_t i = 0; i < msg_support_pairs.size(); i++)
    {
        output << "\tthis->" + msg_support_pairs.at(i).msg_support_var + " = rosidl_typesupport_cpp::get_message_type_support_handle<" + msg_support_pairs.at(i).msg_type + ">();\n";
    }

    output << "\n\tstd::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();\n";
    output << "\tstd::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();\n";

    // output << "\n\tthis->read_bag();\n\n";
    for (size_t i = 0; i < topic_sorting_data.size(); i++)
    {
        temp = topic_sorting_data.at(i).topic_name;
        temp.erase(temp.begin());
        output << "\n\tparse_" + slashToUnderscore(temp) + "();";
        output << "\n\tend = std::chrono::steady_clock::now();";
        output << "\n\tstd::cout << \"Topic (" + std::to_string(i + 1) + "/" + std::to_string(topic_sorting_data.size()) + ") Runtime: \" << std::chrono::duration_cast<std::chrono::seconds>(end-begin).count() << \" seconds\" << std::endl;";
        output << "\n\tbegin = std::chrono::steady_clock::now();";
    }

    output << "\n}\n\n";

    // Destructor
    output << class_name + "::~" + class_name + "()\n";
    output << "{\n\tdelete reader;\n}\n\n";

    // Process Messages
    // output << "bool " + class_name + "::processMessage(const std::shared_ptr<rosbag2_storage::SerializedBagMessage> serialized_msg, auto ros_message)\n";
    // output << "{\n";

    // output << "\tros_message->time_stamp = 0;\n\tros_message->message = nullptr;\n\tros_message->allocator = rcutils_get_default_allocator();\n";

    std::string message_data_filename;
    std::string buffer;
    std::ifstream message_data_file;
    std::vector<std::string> types;
    std::vector<std::string> field_names;
    std::vector<std::string> field_array;
    std::vector<std::string> message_structures;

    for (size_t i = 0; i < topic_sorting_data.size(); i++)
    {
        temp = topic_sorting_data.at(i).msg_var;
        temp.erase(temp.end() - 4, temp.end());
        message_data_filename = output_path + "files/parser_files/msg_data/" + temp + ".log";
        message_data_file.open(message_data_filename);

        types.clear();
        field_names.clear();
        split_string.clear();
        field_array.clear();
        message_structures.clear();
        while (!message_data_file.eof())
        {
            getline(message_data_file, buffer);
            split_string = split(buffer, '#');
            if (split_string.size() > 1)
            {
                types.push_back(convertFieldTypes(split_string.at(0)));
                /*for (size_t j = 0; j < split_string.at(1).length(); j++)
                {
                    if (split_string.at(1).at(j) == '!')
                    {
                        split_string.at(1).at(j) = '.';
                    }
                }*/
                field_names.push_back(split_string.at(1));
                if (split_string.size() == 3)
                {
                    temp = split_string.at(2);
                    //for (size_t j = 0; j < temp.length(); j++)
                    //{
                    //    if (temp.at(j) == '@')
                    //    {
                    //       temp.erase(temp.begin() + j);
                    //        temp.insert(j, ".at(j)");
                    //    }
                    //}
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
        output << "void " + class_name + "::" + "parse_" + slashToUnderscore(temp) + "()\n{\n";

        for (size_t j = 0; j < field_names.size(); j++)
        {
            output << "\tstd::vector<";
            if (types.at(j) == "string")
            {
                output << "std::";
            }
            output << types.at(j) + "> " + bangToUnderscore(field_names.at(j)) + ";\n\t" + bangToUnderscore(field_names.at(j)) + ".clear();\n";
        }
        output << "\n";

        output << "\tfilename = path + \"" + slashToUnderscore(temp) + ".mat\";\n";

        output << "\tthis->reader->open(this->storage_options, this->converter_options);\n";
        output << "\tstd::vector<rosbag2_storage::TopicMetadata> topics = this->reader->get_all_topics_and_types();\n";
        output << "\tstd::map<std::string, std::string> topics_map;\n";
        output << "\tfor (rosbag2_storage::TopicMetadata topic : topics)\n\t{\n";
        output << "\t\ttopics_map.insert(std::pair<std::string, std::string>(topic.name, topic.type));\n\t}\n";
        output << "\tauto ros_message = std::make_shared<rosbag2_cpp::rosbag2_introspection_message_t>();\n\n";

        output << "\twhile (reader->has_next())\n\t{\n";
        output << "\t\tstd::shared_ptr<rosbag2_storage::SerializedBagMessage> serialized_message = reader->read_next();\n";
        output << "\t\tstd::string topic_type = topics_map[serialized_message->topic_name];\n";
        output << "\t\tif (serialized_message->topic_name == \"" + topic_sorting_data.at(i).topic_name + "\")\n\t\t{\n";
        output << "\t\t\tros_message->message = &" + topic_sorting_data.at(i).msg_var + ";\n";
        output << "\t\t\tcdr_deserializer->deserialize(serialized_message, " + topic_sorting_data.at(i).msg_support_var + ", ros_message);\n";

        for (size_t j = 0; j < field_names.size(); j++)
        {
            if (field_array.at(j) == "")
            {
                output << "\t\t\t" + bangToUnderscore(field_names.at(j)) + ".push_back(" + topic_sorting_data.at(i).msg_var + "." + message_structures.at(j) + ");\n";
            }
            else
            {
                split_string = split(field_array.at(j), '@');
                // std::cout<<field_array.at(j)<<"\t"<<split_string.size()<<std::endl;
                if (split_string.size() == 1)
                {
                    split_string.at(0).erase(split_string.at(0).end() - 6, split_string.at(0).end());
                }

                output << "\t\t\tfor (size_t j = 0; j < " + topic_sorting_data.at(i).msg_var + "." + split_string.at(0) + ".size(); j++)\n\t\t\t{\n";

                if (types.at(j) == "string")
                {
                    output << "\t\t\t\t" + bangToUnderscore(field_names.at(j)) + ".push_back(std::to_string(" + topic_sorting_data.at(i).msg_var + "." + split_string.at(0) + ".size()));\n";
                }
                else
                {
                    output << "\t\t\t\t" + bangToUnderscore(field_names.at(j)) + ".push_back(" + topic_sorting_data.at(i).msg_var + "." + split_string.at(0) + ".size());\n";
                }

                //std::cout<<field_array.at(j)<<std::endl;
                split_string = split(field_array.at(j), '@');
                if (split_string.size() > 1)
                {
                    temp = split_string.at(0) + ".at(j)" + split_string.at(1);
                    temp.erase(temp.end()-6, temp.end());
                }
                else
                {
                    temp = split_string.at(0);
                    temp.erase(temp.end()-6, temp.end());
                    temp.append(".at(j)");
                }

                output << "\t\t\t\t" + bangToUnderscore(field_names.at(j)) + ".push_back(" + topic_sorting_data.at(i).msg_var + "." + temp + ");\n";
                output << "\t\t\t}\n";
            }
        }

        output << "\t\t}\n";
        output << "\n\t}\n";

        output << "\n\tTinyMATWriterFile *mat_file = TinyMATWriter_open(filename.c_str());\n";
        temp = topic_sorting_data.at(i).topic_name;
        temp.erase(temp.begin());
        output << "\tTinyMATWriter_startStruct(mat_file, \"" + slashToUnderscore(temp) + "\");\n\n";    

        for (size_t j = 0; j < field_names.size(); j++)
        {
            if (types.at(j) == "string")
            {
                //output << "\tTinyMATWriter_writeStringVector(mat_file, \"" + bangToUnderscore(field_names.at(j)) + "\", " + bangToUnderscore(field_names.at(j)) + ");\n";
            }
            else
            {
                output << "\tTinyMATWriter_writeDoubleVector(mat_file, \"" + bangToUnderscore(field_names.at(j)) + "\", " + bangToUnderscore(field_names.at(j)) + ", false);\n";
            }
        }  

        output << "\n\tTinyMATWriter_endStruct(mat_file);\n";
        output << "\tTinyMATWriter_close(mat_file);\n";  

        

        output << "\tthis->reader->close();\n}\n\n";
    }

    /*for (size_t i = 0; i < topic_sorting_data.size(); i++)
    {
        temp = topic_sorting_data.at(i).msg_var;
        temp.erase(temp.end()-4, temp.end());
        message_data_filename = "../msg_data/" + temp + ".log";
        message_data_file.open(message_data_filename);

        types.clear();
        field_names.clear();
        split_string.clear();
        while (!message_data_file.eof())
        {
            getline(message_data_file, buffer);
            split_string = split(buffer, '#');
            if (split_string.size()>1)
            {
                types.push_back(split_string.at(0));
                for (size_t j = 0; j < split_string.at(1).length(); j++)
                {
                    if (split_string.at(1).at(j) == '!')
                    {
                        split_string.at(1).at(j) = '.';
                    }
                }
                field_names.push_back(split_string.at(1));
            }
        }
        message_data_file.close();

        if (i==0)
        {
            output << "\tif (serialized_msg->topic_name == \"" + topic_sorting_data.at(i).topic_name + "\")\n";
        }
        else
        {
            output << "\telse if (serialized_msg->topic_name == \"" + topic_sorting_data.at(i).topic_name + "\")\n";
        }
        output << "\t{\n";
        output << "\t\tros_message->message = &" + topic_sorting_data.at(i).msg_var + ";\n";
        output << "\t\tcdr_deserializer->deserialize(serialized_msg, " + topic_sorting_data.at(i).msg_support_var + ", ros_message);\n\n";

        for (size_t j = 0; j < field_names.size(); j++)
        {
            if (types.at(j) == "string" || types.at(j) == "string[]")
            {
                output << "\t\t" + topic_sorting_data.at(i).msg_var + "." + field_names.at(j) + ".clear();\n";
            }
        }

        output << "\t\t" + output_files.at(i) + ".write((char *)&" + topic_sorting_data.at(i).msg_var + ", sizeof(" + topic_sorting_data.at(i).msg_type + "));\n\t}\n\n";
    }*/

    // output << "\treturn true;\n}\n\n";

    // read bag
    /*output << "void " + class_name + "::read_bag()\n{\n";
    output << "\tthis->reader->open(this->storage_options, this->converter_options);\n\n";
    output << "\tstd::vector<rosbag2_storage::TopicMetadata> topics = this->reader->get_all_topics_and_types();\n";
    output << "\tstd::map<std::string, std::string> topics_map;\n";
    output << "\tfor (rosbag2_storage::TopicMetadata topic : topics)\n\t{\n\n\t\ttopics_map.insert(std::pair<std::string, std::string>(topic.name, topic.type));\n\t}\n\n";
    output << "\tauto ros_message = std::make_shared<rosbag2_cpp::rosbag2_introspection_message_t>();\n\n";
    output << "\twhile (reader->has_next())\n\t{\n";
    output << "\t\tstd::shared_ptr<rosbag2_storage::SerializedBagMessage> serialized_message = reader->read_next();\n";
    output << "\t\tstd::string topic_type = topics_map[serialized_message->topic_name];\n";
    output << "\t\tbool pass = this->processMessage(serialized_message, ros_message);\n";
    output << "\t\tif(!pass)\n\t\t{\n";
    output << "\t\t\tstd::cout<<\"Skipped\"<<std::endl;\n\t\t}\n\t}\n";*/

    // output << "}\n";

    source_file.open(source_filename);
    source_file << output.str();
    source_file.close();
}

void MatlabParserGenerator::writeConfig()
{
    std::stringstream output;

    output << package_name + ":\n";
    output << "  ros__parameters:\n";
    output << "    path: \"/home/kyle/Data/test/\"\n";
    output << "    bagfile: \"2023-01-03_VEGAS_run1_vehicle_1726\"\n";

    config_file.open(config_filename);
    config_file << output.str();
    config_file.close();
}

void MatlabParserGenerator::writeLaunch()
{
    std::stringstream output;
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

void MatlabParserGenerator::writeMain()
{
    std::stringstream output;

    output << "#include \"" + package_name + "/" + package_name + ".hpp\"\n\n";
    output << "int main()\n{\n";
    output << "\tstd::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();\n";
    // output << "\trclcpp::init(argc, argv);\n";
    // output << "\tstd::make_shared<" + class_name + ">();\n";
    output << "\t" << class_name + " parser;\n";
    // output << "\tparser = new " + class_name <<"();\n";
    output << "\tstd::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();\n";
    output << "\tstd::cout << \"Total Runtime: \" << std::chrono::duration_cast<std::chrono::seconds>(end - begin).count() << \" seconds\" << std::endl;\n";
    // output << "\trclcpp::shutdown();\n\n";
    output << "\treturn 0;\n}";

    main_file.open(main_filename);
    main_file << output.str();
    main_file.close();
}

std::vector<std::string> MatlabParserGenerator::readBagDataFile(std::string filename)
{
    std::ifstream file;
    file.open(filename);
    std::vector<std::string> output;
    std::string temp;
    output.clear();

    while (!file.eof())
    {
        getline(file, temp);
        if (temp != "")
        {
            output.push_back(temp);
        }
    }

    file.close();
    return output;
}

std::string MatlabParserGenerator::convertFieldTypes(std::string str)
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

void MatlabParserGenerator::printStringVector(std::vector<std::string> input)
{
    for (size_t i = 0; i < input.size(); i++)
    {
        std::cout << input.at(i) << std::endl;
    }
}

std::string MatlabParserGenerator::slashToUnderscore(std::string str)
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

std::string MatlabParserGenerator::slashToColon(std::string str)
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

std::string MatlabParserGenerator::bangToUnderscore(std::string str)
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

std::string MatlabParserGenerator::bangToDot(std::string str)
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

std::vector<std::string> MatlabParserGenerator::split(std::string s, char delim)
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

std::string MatlabParserGenerator::snakeToCamel(std::string str)
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