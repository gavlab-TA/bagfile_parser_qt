#ifndef SELECT_TOPICS_WINDOW_HPP
#define SELECT_TOPICS_WINDOW_HPP

#include <QWidget>
#include <QStringList>
#include <QStringListIterator>
#include <QListWidget>
#include <QListWidgetItem>
#include <QVBoxLayout>
#include <QPushButton>
#include <QDialog>

#include "rclcpp/rclcpp.hpp"
#include <rosbag2_cpp/readers/sequential_reader.hpp>

#include <fstream>

class SelectTopicsWindow : public QDialog
{
    Q_OBJECT

    public:
    SelectTopicsWindow(const int &width, const int &height, const rosbag2_storage::BagMetadata &data, const std::string &output_path, QWidget *parent);
    ~SelectTopicsWindow();

    private: 
    QListWidget *list_widget;
    QPushButton *select_button;

    rosbag2_storage::BagMetadata data;
    std::string output_path;

    private slots:
    void saveSelected();
};

#endif