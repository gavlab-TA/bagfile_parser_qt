#include "bagfile_parser_qt/windows/string_display_window.hpp"

StringDisplayWindow::StringDisplayWindow(const std::vector<std::string> &strings, const std::string &title, QWidget *parent) : QDialog(parent)
{
    this->initialize(strings, title);
}

StringDisplayWindow::StringDisplayWindow(const std::vector<std::string> &strings, QWidget *parent, const std::string &title) : QDialog(parent)
{
    this->initialize(strings, title);
}

StringDisplayWindow::~StringDisplayWindow()
{

}

void StringDisplayWindow::initialize(const std::vector<std::string> &strings, const std::string &title)
{
    this->setWindowTitle(QString(title.c_str()));
    this->setWindowModality(Qt::ApplicationModal);
    
    QVBoxLayout *layout = new QVBoxLayout();
    done_button = new QPushButton();
    done_button->setText("Done");

    QListWidget *list_widget = new QListWidget();

    for (const std::string &str : strings)
    {
        QListWidgetItem *item = new QListWidgetItem(QString(str.c_str()));
        list_widget->addItem(item);
    }

    layout->addWidget(list_widget);
    layout->addWidget(done_button);

    QObject::connect(done_button, &QPushButton::released, this, &StringDisplayWindow::done);

    this->setLayout(layout);
    this->show(); // Maybe remove this (modality)
}

void StringDisplayWindow::done()
{
    this->close();
}