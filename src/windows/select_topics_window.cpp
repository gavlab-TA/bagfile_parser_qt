#include "bagfile_parser_qt/windows/select_topics_window.hpp"

SelectTopicsWindow::SelectTopicsWindow(const int &width, const int &height, const rosbag2_storage::BagMetadata &data, const std::string &output_path, QWidget *parent) : QDialog(parent)
{
    this->setWindowTitle("Select Topics");
    this->resize(width, height);
    this->setWindowModality(Qt::ApplicationModal);
    this->data = data;
    this->output_path = output_path;
    this->topic_data.clear();

    // Init Layout
    this->layout = new QVBoxLayout();
    this->selection_layout = new QHBoxLayout();

    // Init List
    this->list_widget = new QListWidget();

    // Alphabetize Topic Data by Topic Name
    for (size_t i = 0; i < this->data.topics_with_message_count.size(); ++i)
    {
        if (this->data.topics_with_message_count.at(i).message_count > 0)
        {
            TopicData buffer;
            buffer.topic_name = this->data.topics_with_message_count.at(i).topic_metadata.name;
            buffer.message_type = this->data.topics_with_message_count.at(i).topic_metadata.type;
            this->topic_data.push_back(buffer);
        }
    }
    std::sort(this->topic_data.begin(), this->topic_data.end(), compareTopicName);

    // Load List
    for (TopicData d : this->topic_data)
    {
        items.push_back(QString(d.topic_name.c_str()));
        valid_topics.push_back(d.topic_name);
        valid_msgs.push_back(d.message_type);
    }   

    for (const QString &item_text : items)
    {
        QListWidgetItem *item = new QListWidgetItem(item_text);
        item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
        item->setCheckState(Qt::Unchecked);
        this->list_widget->addItem(item);
    }

    // Init Buttons
    this->select_all_button = new QPushButton();
    this->unselect_all_button = new QPushButton();
    this->select_button = new QPushButton();

    // Set Button Text
    this->select_all_button->setText("Select All");
    this->unselect_all_button->setText("Clear All");
    this->select_button->setText("Save Selected Topics");

    // Set callbacks
    QObject::connect(this->select_all_button, &QPushButton::released, this, &SelectTopicsWindow::selectAll);
    QObject::connect(this->unselect_all_button, &QPushButton::released, this, &SelectTopicsWindow::unselectAll);
    QObject::connect(this->select_button, &QPushButton::released, this, &SelectTopicsWindow::saveSelected);

    // Load Selection Layout
    this->selection_layout->addWidget(select_all_button);
    this->selection_layout->addWidget(unselect_all_button);

    // Load Main layout
    this->layout->addWidget(this->list_widget);
    this->layout->addLayout(this->selection_layout);
    this->layout->addWidget(this->select_button);
    this->setLayout(this->layout);
}

SelectTopicsWindow::~SelectTopicsWindow()
{

}

void SelectTopicsWindow::saveSelected()
{
    // Find Checked Items
    std::vector<int> save_indices;
    for (int i = 0; i < list_widget->count(); ++i)
    {
        if (list_widget->item(i)->checkState() == Qt::Checked)
        {
            save_indices.push_back(i);
        }
    }

    // Write checked items to file
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
    // Set all items checked
    for (int i = 0; i < list_widget->count(); ++i)
    {
        list_widget->item(i)->setCheckState(Qt::Checked);
    }
}

void SelectTopicsWindow::unselectAll()
{
    // Set all items unchecked
    for (int i = 0; i < list_widget->count(); ++i)
    {
        list_widget->item(i)->setCheckState(Qt::Unchecked);
    }
}