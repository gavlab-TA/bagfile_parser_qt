#include "bagfile_parser_qt/windows/task_window.hpp"

TaskWindow::TaskWindow(const std::string &message, QWidget* parent) : QDialog(parent)
{
    this->setWindowModality(Qt::ApplicationModal);
    this->resize(500, 250);
    this->setWindowFlags(Qt::Dialog | Qt::WindowTitleHint | Qt::CustomizeWindowHint);

    this->setWindowTitle("Please Wait");
    
    this->label = new QLabel();
    this->label->setText(QString(message.c_str()));
    this->label->setAlignment(Qt::AlignCenter);

    this->layout = new QVBoxLayout();
    this->layout->addWidget(this->label, Qt::AlignCenter);

    this->setLayout(this->layout);
}

TaskWindow::~TaskWindow()
{

}

void TaskWindow::closeWindow()
{
    emit finished();
    this->close();
}
