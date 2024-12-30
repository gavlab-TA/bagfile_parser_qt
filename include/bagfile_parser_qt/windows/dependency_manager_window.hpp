#ifndef DEPENDENCY_MANAGER_WINDOW_HPP
#define DEPENDENCY_MANAGER_WINDOW_HPP

#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QLayout>
#include <QtWidgets>
#include <QDialog>

#include "rclcpp/rclcpp.hpp"
#include <rosbag2_cpp/readers/sequential_reader.hpp>
#include <boost/algorithm/string.hpp>
#include <filesystem>

#include "bagfile_parser_qt/windows/recommended_depends_window.hpp"
#include "bagfile_parser_qt/windows/string_display_window.hpp"
#include "bagfile_parser_qt/windows/remove_depends_window.hpp"

class DependencyManagerWindow : public QDialog
{
    Q_OBJECT

    public:
    DependencyManagerWindow(const int &width, const int &height, const std::string &output_path, QWidget *parent = nullptr);
    ~DependencyManagerWindow();

    private:
    // Layouts
    QVBoxLayout* main_layout;
    QHBoxLayout* add_depends_layout;

    // Buttons
    QPushButton* show_depends_button;
    QPushButton* add_depends_button;
    QPushButton* remove_depends_button;
    QPushButton* reset_depends_button;
    
    // Line Edit
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