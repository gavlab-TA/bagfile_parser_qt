#include "bagfile_parser_qt/dependency_manager_window.hpp"

DependencyManagerWindow::DependencyManagerWindow(const int &width, const int &height, const std::string &output_path, const rosbag2_storage::BagMetadata &data)
{
    this->main_layout = new QVBoxLayout();
    this->resize(width, height);
    this->setWindowTitle("Configure Dependency Packages");

    this->bag_packages.clear();
    this->packages.clear();
    this->output_path = output_path;
    this->data = data;
    this->getBagPackages();

    cleanupDependsFile();

    this->setLayout(main_layout);
    this->show();

    RecommendedDependsWindow recommended_depends_window(bag_packages, output_path, this);
    recommended_depends_window.exec();
}

DependencyManagerWindow::~DependencyManagerWindow()
{

}

void DependencyManagerWindow::getBagPackages()
{
    for (size_t i = 0; i < data.topics_with_message_count.size(); ++i)
    {
        std::vector<std::string> split_string;
        std::string topic_type = data.topics_with_message_count.at(i).topic_metadata.type;
        
        boost::split(split_string, topic_type, boost::is_any_of("/"));
        if (split_string.size() > 1)
        {
            std::string package_name = split_string.front();
            if (!vectorContains(bag_packages, package_name))
            {
                bag_packages.push_back(package_name);
            }
        }
    }
}

bool DependencyManagerWindow::vectorContains(const std::vector<std::string> &vec, const std::string &val)
{
    for (size_t i = 0; i < vec.size(); ++i)
    {
        if (vec.at(i) == val)
        {
            return true;
        }
    }
    return false;
}

void DependencyManagerWindow::cleanupDependsFile()
{
    std::string filename = output_path + "/files/depends.txt";
    std::ifstream in_file;
    in_file.open(filename.c_str());

    std::vector<std::string> in_file_list;
    while (!in_file.eof())
    {
        std::string line;
        getline(in_file, line);
        if (line.length() > 1)
        {
            in_file_list.push_back(line);
        }
    }
    in_file.close();

    std::vector<std::string> out_file_list;
    for (size_t i = 0; i < in_file_list.size(); ++i)
    {
        if (!vectorContains(out_file_list, in_file_list.at(i)))
        {    
            out_file_list.push_back(in_file_list.at(i));
        }
    }

    std::ofstream out_file;
    out_file.open(filename.c_str(), std::ios_base::trunc);
    for (const std::string &item : out_file_list)
    {
        out_file << item;
        out_file << "\n";
    }
    out_file.close();
}