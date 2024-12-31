#ifndef MAIN_WINDOW_HPP
#define MAIN_WINDOW_HPP

#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QLayout>
#include <QFileDialog>
#include <QThread>
#include <QMutex>
#include <QCoreApplication>

#include "rclcpp/rclcpp.hpp"
#include <rosbag2_cpp/readers/sequential_reader.hpp>
#include <ament_index_cpp/get_package_share_directory.hpp>
#include <fstream>
#include <boost/filesystem.hpp>

#include "bagfile_parser_qt/windows/configure_window.hpp"
#include "bagfile_parser_qt/windows/select_topics_window.hpp"
#include "bagfile_parser_qt/windows/dependency_manager_window.hpp"
#include "bagfile_parser_qt/windows/task_window.hpp"

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
    // Layouts
    QGridLayout* main_layout;
    QVBoxLayout* workflow_layout;
    QGridLayout* bag_select_layout;
    QGridLayout* parser_generator_layout;

    //Labels
    QLabel* bag_path_label;
    QLabel* status_label;
    QLabel* csv_parser_label;
    QLabel* matlab_parser_label;
    
    //Buttons
    QPushButton* bag_select_button;   
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
    
    // Thread Vars
    QMutex* mutex;

    // File Paths
    std::string bagfile_path;
    std::string share_path;
    std::string output_path;

    // Rosbag Data
    rosbag2_storage::BagMetadata data;

    // Has Bag been selected flag
    bool bag_selected;

    // Setup local files
    void setupFileLocations();

    private slots:
    void openBagSelectWindow();
    void openConfigureParserWindow();
    void openSelectTopicsWindow();
    void openConfigureDependsWindow();
    void autoconfigureGenerator();
    void generateCsvParser();
    void buildCsvParser();
    void runCsvParser();
    void generateCsvMatlabParser();

    void generateMatlabParser();
    void buildMatlabParser();
    void runMatlabParser();
    void resetParser();
    void resetStatusLabel();
    void displayStatusError();
};


// Thread Worker Objects 

class AnalyzeMessagesWorker : public QObject
{
    Q_OBJECT

    public:
    explicit AnalyzeMessagesWorker(QMutex *mutex, const std::string &output_path, QObject *parent = nullptr) : QObject(parent) 
    {
        this->mutex = mutex;
        this->output_path = output_path;
    }

    private:
    QMutex* mutex;
    std::string output_path;

    public slots:
    void runAnalyzeMessagesThread();

    signals:
    void workFinished();
};

class RunMatlabParserWorker : public QObject
{
    Q_OBJECT

    public:
    explicit RunMatlabParserWorker(QMutex *mutex, const std::string &output_path, QObject* parent=nullptr) : QObject(parent)
    {
        this->mutex = mutex;
        this->output_path = output_path;
    }

    private:
    QMutex* mutex;
    std::string output_path;

    public slots:
    void runMatlabParserThread();

    signals:
    void workFinished();
};

class BuildMatlabParserWorker : public QObject
{
    Q_OBJECT

    public: 
    explicit BuildMatlabParserWorker(QMutex *mutex, const std::string &output_path, QObject* parent = nullptr) : QObject(parent)
    {
        this->mutex = mutex;
        this->output_path = output_path;
    }

    private:
    QMutex* mutex;
    std::string output_path;

    public slots:
    void runBuildMatlabParserThread();

    signals:
    void workFinished();
    void success();
    void failure();
};

class BuildCsvParserWorker : public QObject
{
    Q_OBJECT

    public:
    explicit BuildCsvParserWorker(QMutex *mutex, const std::string &output_path, QObject* parent = nullptr) : QObject(parent)
    {
        this->mutex = mutex;
        this->output_path = output_path;
    }

    private:
    QMutex* mutex;
    std::string output_path;

    public slots:
    void runBuildCsvParserThread();

    signals:
    void workFinished();
    void success();
    void failure();    
};

class RunCsvParserWorker : public QObject
{
    Q_OBJECT

    public: 
    explicit RunCsvParserWorker(QMutex *mutex, const std::string &output_path, QObject* parent = nullptr) : QObject(parent)
    {
        this->mutex = mutex;
        this->output_path = output_path;
    }

    private:
    QMutex* mutex;
    std::string output_path;

    public slots:
    void runCsvParserThread();

    signals:
    void workFinished();
};

#endif