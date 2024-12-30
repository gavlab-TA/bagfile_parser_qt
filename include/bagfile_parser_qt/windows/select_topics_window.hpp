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

    struct TopicData
    {
        std::string topic_name;
        std::string message_type;
    };

    static bool compareTopicName(const SelectTopicsWindow::TopicData &a, const SelectTopicsWindow::TopicData &b)
    {
        return a.topic_name < b.topic_name;
    }

private:
    // Layout
    QVBoxLayout *layout;
    QHBoxLayout *selection_layout;

    // Buttons
    QPushButton *select_button;
    QPushButton *select_all_button;
    QPushButton *unselect_all_button;

    // List
    QListWidget *list_widget;

    // Data
    rosbag2_storage::BagMetadata data;
    std::vector<TopicData> topic_data;

    std::string output_path;
    std::vector<std::string> valid_topics;
    std::vector<std::string> valid_msgs;

    QStringList items;

private slots:
    void saveSelected();
    void selectAll();
    void unselectAll();
};

#endif