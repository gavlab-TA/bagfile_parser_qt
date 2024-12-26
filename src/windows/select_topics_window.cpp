#include "bagfile_parser_qt/windows/select_topics_window.hpp"

SelectTopicsWindow::SelectTopicsWindow(const int &width, const int &height, const rosbag2_storage::BagMetadata &data, const std::string &output_path, QWidget *parent) : QDialog(parent)
{
    this->setWindowTitle("Select Topics");
    this->resize(width, height);
    this->setWindowModality(Qt::ApplicationModal);
    this->data = data;
    this->output_path = output_path;

    QVBoxLayout *layout = new QVBoxLayout();

    list_widget = new QListWidget();

    for (size_t i = 0; i < this->data.topics_with_message_count.size(); i++)
    {
        if (this->data.topics_with_message_count.at(i).message_count > 0)
        {
            items.push_back(QString(this->data.topics_with_message_count.at(i).topic_metadata.name.c_str()));
            valid_topics.push_back(this->data.topics_with_message_count.at(i).topic_metadata.name);
            valid_msgs.push_back(this->data.topics_with_message_count.at(i).topic_metadata.type);
        }
    }    


    list_widget = new QListWidget();
    for (const QString &item_text : items)
    {
        QListWidgetItem *item = new QListWidgetItem(item_text);
        item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
        item->setCheckState(Qt::Unchecked);
        this->list_widget->addItem(item);
    }

    layout->addWidget(list_widget);

    this->selection_layout = new QHBoxLayout();
    this->select_all_button = new QPushButton();
    this->select_all_button->setText("Select All");
    this->unselect_all_button = new QPushButton();
    this->unselect_all_button->setText("Clear All");

    this->selection_layout->addWidget(select_all_button);
    this->selection_layout->addWidget(unselect_all_button);
    layout->addLayout(selection_layout);
    
    this->select_button = new QPushButton();
    this->select_button->setText("Save Selected Topics");

    QObject::connect(this->select_all_button, &QPushButton::released, this, &SelectTopicsWindow::selectAll);
    QObject::connect(this->unselect_all_button, &QPushButton::released, this, &SelectTopicsWindow::unselectAll);
    QObject::connect(this->select_button, &QPushButton::released, this, &SelectTopicsWindow::saveSelected);
    layout->addWidget(this->select_button);
    this->setLayout(layout);
}

SelectTopicsWindow::~SelectTopicsWindow()
{

}

void SelectTopicsWindow::saveSelected()
{
    std::vector<int> save_indices;
    for (int i = 0; i < list_widget->count(); ++i)
    {
        if (list_widget->item(i)->checkState() == Qt::Checked)
        {
            save_indices.push_back(i);
        }
    }

    std::ofstream file;
    std::string topic_data_filename = output_path + "/files/selected_topic_data.txt";
    file.open(topic_data_filename, std::ios_base::trunc);
    for (size_t i = 0; i < save_indices.size(); ++i)
    {
        int index = save_indices.at(i);
        std::string line = valid_topics.at(index) + "," + valid_msgs.at(index) + "\n";
        file << line;       
    }

    file.close();
    this->close();
}

void SelectTopicsWindow::selectAll()
{
    for (int i = 0; i < list_widget->count(); ++i)
    {
        list_widget->item(i)->setCheckState(Qt::Checked);
    }
}

void SelectTopicsWindow::unselectAll()
{
    for (int i = 0; i < list_widget->count(); ++i)
    {
        list_widget->item(i)->setCheckState(Qt::Unchecked);
    }
}