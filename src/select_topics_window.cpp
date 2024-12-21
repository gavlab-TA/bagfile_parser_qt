#include "bagfile_parser_qt/select_topics_window.hpp"

SelectTopicsWindow::SelectTopicsWindow(const int &width, const int &height, const rosbag2_storage::BagMetadata &data, const std::string &output_path, QWidget *parent) : QDialog(parent)
{
    this->setWindowTitle("Select Topics");
    this->resize(width, height);
    this->setWindowModality(Qt::ApplicationModal);
    this->data = data;
    this->output_path = output_path;

    QVBoxLayout *layout = new QVBoxLayout();

    list_widget = new QListWidget();

    QStringList items;

    for (size_t i = 0; i < this->data.topics_with_message_count.size(); i++)
    {
        if (this->data.topics_with_message_count.at(i).message_count > 0)
        {
            items.push_back(QString(this->data.topics_with_message_count.at(i).topic_metadata.name.c_str()));
        }
    }    

    for (const QString &item_text : items)
    {
        QListWidgetItem *item = new QListWidgetItem(item_text);
        item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
        item->setCheckState(Qt::Checked);
        this->list_widget->addItem(item);
    }

    layout->addWidget(list_widget);
    
    this->select_button = new QPushButton();
    this->select_button->setText("Save Selected Topics");

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
    std::string topic_data_filename = output_path + "/files/selected_topic_data";
    file.open(topic_data_filename, std::ios_base::trunc);
    size_t index = 0;
    for (size_t i = 0; i < save_indices.size(); ++i)
    {
        if ((int) i == save_indices.at(index))
        {
            index++;
            std::string line = data.topics_with_message_count.at(i).topic_metadata.name + "," + data.topics_with_message_count.at(i).topic_metadata.type + "\n";
            file << line;
        }
    }

    file.close();
    this->close();
}