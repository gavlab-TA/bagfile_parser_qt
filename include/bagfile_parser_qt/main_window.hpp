#ifndef MAIN_WINDOW_HPP
#define MAIN_WINDOW_HPP

#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QLayout>
#include <QFileDialog>

#include "rclcpp/rclcpp.hpp"
#include <rosbag2_cpp/readers/sequential_reader.hpp>

#include "bagfile_parser_qt/configure_window.hpp"

class MainWindow : public QWidget
{
    Q_OBJECT

    public:
    MainWindow(int width, int height);
    ~MainWindow();

    private:
    QGridLayout* main_layout;
    QVBoxLayout* workflow_layout;
    QVBoxLayout* bag_select_layout;

    QPushButton* bag_select_button;
    QLabel* bag_path_label;
    QLabel* status_label;
    
    QPushButton* configure_parser_button;

    void bagSelectButtonPushed();
    void configureParserButtonPushed();

    std::string bagfile_path;
};

#endif