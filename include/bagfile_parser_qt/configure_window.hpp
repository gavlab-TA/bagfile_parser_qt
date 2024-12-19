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

#include "bagfile_parser_qt/remove_packages_window.hpp"

class ConfigureWindow : public QWidget
{
    Q_OBJECT

    public:
    ConfigureWindow(const int &width, const int &height, const std::string &output_path);
    ~ConfigureWindow();

    private:
    QGridLayout* main_layout;
    QPushButton* add_package_button;
    QPushButton* remove_package_button;
    QPushButton* build_workspace_button;
    QPushButton* clear_packages_button;
    
    QLabel* status_label;
    QLabel* package_list_title_label;
    QLabel* package_list_label;
    QLabel* package_path_label;

    void addPackageButtonPushed();
    void removePackageButtonPushed();
    void buildWorkspaceButtonPushed();
    void clearPackagesButtonPushed();

    void getWorkspaceLog();

    std::fstream file;

    std::vector<std::pair<std::string, std::string>> packages;
    std::string log_filename;
    std::string output_path;

    void buildWorkspace();
    bool confirmDialog();
};

#endif