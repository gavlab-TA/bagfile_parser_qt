#ifndef DEPENDENCY_MANAGER_WINDOW_HPP
#define DEPENDENCY_MANAGER_WINDOW_HPP

#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QLayout>

#include "rclcpp/rclcpp.hpp"
#include <rosbag2_cpp/readers/sequential_reader.hpp>
#include <boost/algorithm/string.hpp>

#include "bagfile_parser_qt/recommended_depends_window.hpp"

class DependencyManagerWindow : QWidget
{
    Q_OBJECT

    public:
    DependencyManagerWindow(const int &width, const int &height, const std::string &output_path, const rosbag2_storage::BagMetadata &data);
    ~DependencyManagerWindow();

    private:
    QVBoxLayout* main_layout;

    std::vector<std::string> packages;
    std::vector<std::string> bag_packages;
    std::string output_path;
    rosbag2_storage::BagMetadata data;

    void getBagPackages();
    bool vectorContains(const std::vector<std::string> &vec, const std::string &val);
    void cleanupDependsFile();
};

#endif