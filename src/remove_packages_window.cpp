#include "bagfile_parser_qt/remove_packages_window.hpp"

RemovePackagesWindow::RemovePackagesWindow(const int &width, const int &height, const std::string &package_log_filename, const std::string &output_path, QWidget *parent) : QDialog(parent)
{
    this->setWindowTitle("Remove Packages");
    this->resize(width, height);
    this->package_log_filename = package_log_filename;
    this->output_path = output_path;

    this->initialize(); 
    this->setWindowModality(Qt::ApplicationModal);  
}

RemovePackagesWindow::~RemovePackagesWindow()
{
}

void RemovePackagesWindow::initialize()
{
    QVBoxLayout *layout = new QVBoxLayout();

    list_widget = new QListWidget();
    std::ifstream file;
    file.open(package_log_filename);

    QStringList items;
    std::string line;
    std::vector<std::string> line_data;
    while (!file.eof())
    {
        getline(file, line);
        boost::split(line_data, line, boost::is_any_of(","));
        if (line_data.size() > 1)
        {
            items.push_back(QString(line_data.at(0).c_str()));
        }
    }
    file.close();

    for (const QString &item_text : items)
    {
        QListWidgetItem *item = new QListWidgetItem(item_text);
        item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
        item->setCheckState(Qt::Unchecked);
        list_widget->addItem(item);
    }

    layout->addWidget(list_widget);

    remove_button = new QPushButton();
    remove_button->setText("Remove Selected Packages");

    QObject::connect(remove_button, &QPushButton::released, this, &RemovePackagesWindow::removeSelected);

    layout->addWidget(remove_button);

    this->setLayout(layout);
}

void RemovePackagesWindow::removeSelected()
{
    std::vector<int> remove_indices;
    for (int i = 0; i < list_widget->count(); ++i)
    {
        if (list_widget->item(i)->checkState() == Qt::Checked)
        {
            remove_indices.push_back(i);
        }
    }

    std::ifstream file;
    file.open(this->package_log_filename);
    std::vector<std::pair<std::string, std::string>> package_data;
    while (!file.eof())
    {
        std::string line;
        getline(file, line);
        std::vector<std::string> split_string;
        boost::split(split_string, line, boost::is_any_of(","));

        if (split_string.size() > 1)
        {
            std::pair<std::string, std::string> line_data;
            line_data.first = split_string.at(0);
            line_data.second = split_string.at(1);
            package_data.push_back(line_data);
        }
    }
    file.close();

    int rm_index = 0;
    std::ofstream out_file;
    out_file.open(this->package_log_filename, std::ios_base::trunc);
    for (size_t i = 0; i < package_data.size(); i++)
    {
        if ((int) i == remove_indices.at(rm_index))
        {
            std::vector<std::string> split_string;
            boost::split(split_string, package_data.at(i).second, boost::is_any_of("/"));
            std::string package_path = output_path + "/src/" + split_string.back();
            std::string command = "rm -r " + package_path;
            int res = system(command.c_str());
            if (res)
            {
                std::cout<<"Package does not exist in ws. Check " + output_path + " to ensure that it is removed"<<std::endl;
            }
        }
        else
        {
            std::string line = package_data.at(i).first + "," + package_data.at(i).second + "\n";
            out_file << line;
        }
    } 
    out_file.close();

    this->close();
}
