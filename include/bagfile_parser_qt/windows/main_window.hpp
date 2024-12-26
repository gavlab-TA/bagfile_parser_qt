#ifndef MAIN_WINDOW_HPP
#define MAIN_WINDOW_HPP

#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QLayout>
#include <QFileDialog>

#include "rclcpp/rclcpp.hpp"
#include <rosbag2_cpp/readers/sequential_reader.hpp>
#include <ament_index_cpp/get_package_share_directory.hpp>
#include <fstream>
#include <boost/filesystem.hpp>

#include "bagfile_parser_qt/windows/configure_window.hpp"
#include "bagfile_parser_qt/windows/select_topics_window.hpp"
#include "bagfile_parser_qt/windows/dependency_manager_window.hpp"

#include "bagfile_parser_qt/parser_generator/bag_analyzer.hpp"
#include "bagfile_parser_qt/parser_generator/message_analyzer.hpp"
#include "bagfile_parser_qt/parser_generator/csv_parser_generator.hpp"
#include "bagfile_parser_qt/parser_generator/csv_matlab_generator.hpp"
#include "bagfile_parser_qt/parser_generator/matlab_parser_generator.hpp"

class MainWindow : public QWidget
{
    Q_OBJECT

    public:
    MainWindow(const int &width, const int &height);
    ~MainWindow();

    private:
    QGridLayout* main_layout;
    QVBoxLayout* workflow_layout;
    QVBoxLayout* bag_select_layout;
    QGridLayout* parser_generator_layout;

    QPushButton* bag_select_button;
    QLabel* bag_path_label;
    QLabel* status_label;
    
    QPushButton* configure_parser_button;
    QPushButton* select_topics_button;
    QPushButton* configure_depends_button;
    QPushButton* autoconfigure_generator_button;

    QPushButton* generate_parser_button;
    QPushButton* build_csv_parser_button;
    QPushButton* run_csv_parser_button;
    QPushButton* generate_csv_matlab_parser_button;

    QPushButton* generate_matlab_parser_button;
    QPushButton* build_matlab_parser_button;
    QPushButton* run_matlab_parser_button;

    QPushButton* reset_workspace_button;
    QLabel* csv_parser_label;
    QLabel* matlab_parser_label;


    std::string bagfile_path;
    std::string share_path;
    std::string output_path;

    rosbag2_storage::BagMetadata data;

    bool bag_selected;

    void setupFileLocations();

    private slots:
    void openBagSelectWindow();
    void openConfigureParserWindow();
    void openSelectTopicsWindow();
    void openConfigureDependsWindow();
    void autoconfigureGenerator();
    void generateParser();
    void buildParser();
    void runCsvParser();
    void generateCsvMatlabParser();

    void generateMatlabParser();
    void buildMatlabParser();
    void runMatlabParser();
    void resetParser();
};

#endif