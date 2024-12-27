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
    delete layout;
    delete done_button;
    delete list_widget;
}

void StringDisplayWindow::initialize(const std::vector<std::string> &strings, const std::string &title)
{
    this->setWindowTitle(QString(title.c_str()));
    this->setWindowModality(Qt::ApplicationModal);

    // Init Layout   
    this->layout = new QVBoxLayout();
    
    // Setup Button
    this->done_button = new QPushButton();
    this->done_button->setText("Done");
    QObject::connect(this->done_button, &QPushButton::released, this, &StringDisplayWindow::done);

    // Init list widget
    this->list_widget = new QListWidget();

    // Load list widget
    for (const std::string &str : strings)
    {
        QListWidgetItem *item = new QListWidgetItem(QString(str.c_str()));
        this->list_widget->addItem(item);
    }

    // Load layout
    this->layout->addWidget(list_widget);
    this->layout->addWidget(done_button);

    // Set layout
    this->setLayout(this->layout);
    this->show(); 
}

void StringDisplayWindow::done()
{
    this->close();
}