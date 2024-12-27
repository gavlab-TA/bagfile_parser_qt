#ifndef CONFIGURE_WINDOW_HPP
#define CONFIGURE_WINDOW_HPP

#include <cstdlib>
#include <fstream>
#include <iostream>
#include <filesystem>
#include <boost/algorithm/string.hpp>
#include <sstream>

#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QLayout>
#include <QFileDialog>
#include <QCoreApplication>
#include <QMessageBox>

#include "bagfile_parser_qt/windows/remove_packages_window.hpp"

class ConfigureWindow : public QWidget
{
    Q_OBJECT

    public:
    ConfigureWindow(const int &width, const int &height, const std::string &output_path);
    ~ConfigureWindow();

    private:
    // Layouts
    QGridLayout* main_layout;
    QHBoxLayout* button_layout;

    // Buttons
    QPushButton* add_package_button;
    QPushButton* remove_package_button;
    QPushButton* build_workspace_button;
    QPushButton* clear_packages_button;
    
    // Labels
    QLabel* status_label;
    QLabel* package_list_title_label;
    QLabel* package_path_title_label;
    QLabel* package_list_label;
    QLabel* package_path_label;

    void getWorkspaceLog();

    std::fstream file;

    std::vector<std::pair<std::string, std::string>> packages;
    std::string log_filename;
    std::string output_path;

    bool confirmDialog();
    void runBuild();

    private slots:
    void openRemoveWindow();
    void clearPackages();
    void openAddPackageWindow();
    void buildWorkspace();
};

#endif