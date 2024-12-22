#include "bagfile_parser_qt/dependency_manager_window.hpp"

DependencyManagerWindow::DependencyManagerWindow(const int &width, const int &height, const std::string &output_path, const rosbag2_storage::BagMetadata &data, QWidget *parent) : QWidget(parent)
{
    this->main_layout = new QVBoxLayout();
    this->resize(width, height);
    this->setWindowTitle("Configure Dependency Packages");

    this->bag_packages.clear();
    this->packages.clear();
    this->output_path = output_path;
    this->data = data;
    this->getBagPackages();

    cleanupDependsFile();

    this->show_depends_button = new QPushButton();
    this->show_depends_button->setText("Show Tracked Dependencies");
    this->reset_depends_button = new QPushButton();
    this->reset_depends_button->setText("Reset Tracked Dependencies");
    this->add_depends_button = new QPushButton();
    this->add_depends_button->setText("Add Dependency");
    this->remove_depends_button = new QPushButton();
    this->remove_depends_button->setText("Remove Depends");

    this->depends_edit = new QLineEdit();
    this->add_depends_layout = new QHBoxLayout();
    this->add_depends_layout->addWidget(this->depends_edit);
    this->add_depends_layout->addWidget(this->add_depends_button);
    this->add_depends_layout->addWidget(this->remove_depends_button);

    this->main_layout->addWidget(this->show_depends_button);
    this->main_layout->addLayout(this->add_depends_layout);
    this->main_layout->addWidget(this->reset_depends_button);
    //this->main_layout->addWidget(this->remove_depends_button);

    QObject::connect(this->show_depends_button, &QPushButton::released, this, &DependencyManagerWindow::openShowDependsWindow);
    QObject::connect(this->add_depends_button, &QPushButton::released, this, &DependencyManagerWindow::addDependency);
    QObject::connect(this->reset_depends_button, &QPushButton::released, this, &DependencyManagerWindow::resetDependencies);
    QObject::connect(this->remove_depends_button, &QPushButton::released, this, &DependencyManagerWindow::openRemoveDependsWindow);

    RecommendedDependsWindow recommended_depends_window(bag_packages, output_path);
    recommended_depends_window.exec();

    this->setLayout(main_layout);
}

DependencyManagerWindow::~DependencyManagerWindow()
{

}

void DependencyManagerWindow::getBagPackages()
{
    for (size_t i = 0; i < data.topics_with_message_count.size(); ++i)
    {
        std::vector<std::string> split_string;
        std::string topic_type = data.topics_with_message_count.at(i).topic_metadata.type;
        
        boost::split(split_string, topic_type, boost::is_any_of("/"));
        if (split_string.size() > 1)
        {
            std::string package_name = split_string.front();
            if (!vectorContains(bag_packages, package_name))
            {
                bag_packages.push_back(package_name);
            }
        }
    }
}

bool DependencyManagerWindow::vectorContains(const std::vector<std::string> &vec, const std::string &val)
{
    for (size_t i = 0; i < vec.size(); ++i)
    {
        if (vec.at(i) == val)
        {
            return true;
        }
    }
    return false;
}

void DependencyManagerWindow::cleanupDependsFile()
{
    std::string filename = output_path + "/files/depends.txt";
    std::ifstream in_file;
    in_file.open(filename.c_str());

    std::vector<std::string> in_file_list;
    while (!in_file.eof())
    {
        std::string line;
        getline(in_file, line);
        if (line.length() > 1)
        {
            in_file_list.push_back(line);
        }
    }
    in_file.close();

    std::vector<std::string> out_file_list;
    for (size_t i = 0; i < in_file_list.size(); ++i)
    {
        if (!vectorContains(out_file_list, in_file_list.at(i)))
        {    
            out_file_list.push_back(in_file_list.at(i));
        }
    }

    std::ofstream out_file;
    out_file.open(filename.c_str(), std::ios_base::trunc);
    for (const std::string &item : out_file_list)
    {
        out_file << item;
        out_file << "\n";
    }
    out_file.close();
}

void DependencyManagerWindow::openShowDependsWindow()
{
    packages.clear();

    std::ifstream file;
    std::string filename = output_path + "/files/depends.txt";
    file.open(filename);

    while (!file.eof())
    {
        std::string line;
        getline(file, line);
        if (line.length() > 1)
        {
            packages.push_back(line);
        }
    }

    StringDisplayWindow string_display_window(packages, "Tracked Depends", this);
    string_display_window.exec();
}

void DependencyManagerWindow::addDependency()
{
    std::ofstream file;
    std::string filename = output_path + "/files/depends.txt";

    std::string depend = depends_edit->text().toStdString();
    depend += "\n";

    depends_edit->clear();

    file.open(filename, std::ios_base::app);
    file << depend;
    file.close();
}

void DependencyManagerWindow::resetDependencies()
{
    if (confirmDialog())
    {
        std::ofstream file;
        std::string filename = output_path + "/files/depends.txt";
        file.open(filename, std::ios_base::trunc);
        file.close();

        RecommendedDependsWindow recommended_depends_window(bag_packages, output_path);
        recommended_depends_window.exec();
    }
}

bool DependencyManagerWindow::confirmDialog()
{
    QMessageBox msg_box;
    msg_box.setIcon(QMessageBox::Question);
    msg_box.setWindowTitle("Warning");
    msg_box.setText("This will remove all tracked dependencies. Are you sure?");
    msg_box.setStandardButtons(QMessageBox::Yes | QMessageBox::Cancel);
    msg_box.setDefaultButton(QMessageBox::Cancel);

    int result = msg_box.exec();
    if (result == QMessageBox::Yes)
    {
        return true;
    }
    else 
    {
        return false;
    }
}

void DependencyManagerWindow::openRemoveDependsWindow()
{
    RemoveDependsWindow remove_depends_window(output_path+"/files/depends.txt", this);
    remove_depends_window.exec();
}