#ifndef DEPENDENCY_MANAGER_WINDOW_HPP
#define DEPENDENCY_MANAGER_WINDOW_HPP

#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QLayout>

#include "rclcpp/rclcpp.hpp"
#include <rosbag2_cpp/readers/sequential_reader.hpp>

class DependencyManagerWindow : QWidget
{
    Q_OBJECT

    public:
    DependencyManagerWindow(const int &width, const int &height, const rosbag2_storage::BagMetadata &data);
    ~DependencyManagerWindow();

    private:
    QHBoxLayout* main_layout;

    rosbag2_storage::BagMetadata data;
};

#endif