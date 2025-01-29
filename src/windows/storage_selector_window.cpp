#include "bagfile_parser_qt/windows/storage_selector_window.hpp"

StorageSelectorWindow::StorageSelectorWindow(const int &width, const int &height, QWidget *parent) : QDialog(parent)
{
    this->setWindowTitle("Choose Rosbag Storage Type");
    this->resize(width, height);
    this->setWindowModality(Qt::ApplicationModal);
    
    // Init Layout
    this->layout = new QVBoxLayout();
    
    // Init Dropdown
    this->combo_box = new QComboBox();
    this->combo_box->addItem("sqlite3");
    this->combo_box->addItem("mcap");
    
    // Setup Save Button
    QPushButton *save_button = new QPushButton("Save Storage Type");
    connect(save_button, &QPushButton::released, this, &StorageSelectorWindow::closeWindow);

    // Setup 
    this->layout->addWidget(this->combo_box);
    this->layout->addWidget(save_button);    

    this->setLayout(this->layout);
}   

StorageSelectorWindow::~StorageSelectorWindow()
{

}

void StorageSelectorWindow::closeWindow()
{
    this->storage_type = this->combo_box->currentText().toStdString();
    this->close();
}

std::string StorageSelectorWindow::getStorageType()
{
    return this->storage_type;
}