#ifndef DEPENDENCY_MANAGER_WINDOW_HPP
#define DEPENDENCY_MANAGER_WINDOW_HPP

#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QLayout>
#include <QtWidgets>

#include "rclcpp/rclcpp.hpp"
#include <rosbag2_cpp/readers/sequential_reader.hpp>
#include <boost/algorithm/string.hpp>
#include <filesystem>

#include "bagfile_parser_qt/recommended_depends_window.hpp"
#include "bagfile_parser_qt/string_display_window.hpp"
#include "bagfile_parser_qt/remove_depends_window.hpp"

class DependencyManagerWindow : public QWidget
{
    Q_OBJECT

    public:
    DependencyManagerWindow(const int &width, const int &height, const std::string &output_path, QWidget *parent = nullptr);
    ~DependencyManagerWindow();

    private:
    QVBoxLayout* main_layout;
    QPushButton* show_depends_button;
    QPushButton* add_depends_button;
    QPushButton* remove_depends_button;
    QPushButton* reset_depends_button;
    QHBoxLayout* add_depends_layout;
    QLineEdit* depends_edit;

    std::vector<std::string> packages;
    std::vector<std::string> bag_packages;
    std::string output_path;
    std::vector<std::pair<std::string, std::string>> data;


    void cleanupDependsFile();
    void getBagPackages();
    void getData();

    bool vectorContains(const std::vector<std::string> &vec, const std::string &val);
    bool confirmDialog();

    private slots:
    void openShowDependsWindow();
    void addDependency();
    void openRemoveDependsWindow();
    void resetDependencies();
};

#endif