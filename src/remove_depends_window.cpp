#include "bagfile_parser_qt/remove_depends_window.hpp"

RemoveDependsWindow::RemoveDependsWindow(const std::string &depends_file, QWidget *parent) : QDialog(parent)
{
    this->setWindowTitle("Remove Dependencies");
    this->depends_filename = depends_file;
    
    this->setWindowModality(Qt::ApplicationModal);
    this->initialize();
}

RemoveDependsWindow::~RemoveDependsWindow()
{

}

void RemoveDependsWindow::initialize()
{
    QVBoxLayout *layout = new QVBoxLayout();

    list_widget = new QListWidget();
    std::ifstream file;
    file.open(depends_filename);

    while (!file.eof())
    {
        std::string line;
        getline(file, line);

        if (line.length() < 1)
        {
            QListWidgetItem *item = new QListWidgetItem(QString(line.c_str()));
            item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
            item->setCheckState(Qt::Unchecked);
            list_widget->addItem(item);
        }
    }

    layout->addWidget(list_widget);
    remove_button = new QPushButton();
    remove_button->setText("Remove Selected Depends");
    QObject::connect(remove_button, &QPushButton::released, this, &RemoveDependsWindow::removeSelected);

    layout->addWidget(remove_button);
    this->setLayout(layout);
}

void RemoveDependsWindow::removeSelected()
{
    std::ofstream file;
    file.open(depends_filename, std::ios_base::trunc);

    for (int i = 0; i < list_widget->count(); ++i)
    {
        if (list_widget->item(i)->checkState() == Qt::Checked)
        {
            std::string line;
            line = list_widget->item(i)->text().toStdString();
            line += "\n";
            file << line;
        }
    }
    file.close();    
}