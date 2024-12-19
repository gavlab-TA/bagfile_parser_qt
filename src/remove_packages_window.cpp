#include "bagfile_parser_qt/remove_packages_window.hpp"

RemovePackagesWindow::RemovePackagesWindow(const int &width, const int &height, const std::string &package_log_filename)
{
    this->resize(width, height);
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

    for (const QString &item_text : items)
    {
        QListWidgetItem *item = new QListWidgetItem(item_text);
        item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
        item->setCheckState(Qt::Unchecked);
        list_widget->addItem(item);
    }

    layout->addWidget(list_widget);
    this->setLayout(layout);

    this->show();
}

RemovePackagesWindow::~RemovePackagesWindow()
{

}