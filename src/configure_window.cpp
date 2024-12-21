#include "bagfile_parser_qt/configure_window.hpp"

ConfigureWindow::ConfigureWindow(const int &width, const int &height, const std::string &output_path) : QWidget()
{
    this->resize(width, height);
    this->output_path = output_path;

    this->main_layout = new QGridLayout();

    this->package_list_label = new QLabel();
    this->status_label = new QLabel();
    this->package_path_label = new QLabel();
    this->package_list_title_label = new QLabel();
    this->package_list_title_label->setText("Tracked Message Packages");
    this->package_list_title_label->setFont(QFont("Sans Serif", 20));

    this->add_package_button = new QPushButton();
    this->add_package_button->setText("Add Msg Pkg");
    this->remove_package_button = new QPushButton();
    this->remove_package_button->setText("Remove Msg Pkg");
    this->build_workspace_button = new QPushButton();
    this->build_workspace_button->setText("Build Msgs");
    this->clear_packages_button = new QPushButton();
    this->clear_packages_button->setText("Reset Workspace");

    QObject::connect(this->add_package_button, &QPushButton::released, this, &ConfigureWindow::openAddPackageWindow);
    QObject::connect(this->remove_package_button, &QPushButton::released, this, &ConfigureWindow::openRemoveWindow);
    QObject::connect(this->build_workspace_button, &QPushButton::released, this, &ConfigureWindow::buildWorkspace);
    QObject::connect(this->clear_packages_button, &QPushButton::released, this, &ConfigureWindow::clearPackages);

    this->main_layout->addWidget(this->package_list_title_label, 0, 0, 1, 3, Qt::AlignCenter);
    this->main_layout->addWidget(this->package_list_label, 1, 0, 1, 1, Qt::AlignCenter);
    this->main_layout->addWidget(this->package_path_label, 1, 1, 1, 2, Qt::AlignCenter);
    this->main_layout->addWidget(this->add_package_button, 2, 0, Qt::AlignCenter);
    this->main_layout->addWidget(this->remove_package_button, 2, 1, Qt::AlignCenter);
    this->main_layout->addWidget(this->build_workspace_button, 2, 2, Qt::AlignCenter);
    this->main_layout->addWidget(this->status_label, 3, 1, Qt::AlignCenter);
    this->main_layout->addWidget(this->clear_packages_button, 4, 0, Qt::AlignCenter);

    this->main_layout->setContentsMargins(20, 20, 20, 20);
    this->setLayout(main_layout);

    this->log_filename = this->output_path + "/workspace_log.txt";

    getWorkspaceLog();
}

ConfigureWindow::~ConfigureWindow()
{
    delete main_layout;
    delete add_package_button;
    delete remove_package_button;
    delete build_workspace_button;
    delete clear_packages_button;
    delete status_label;
    delete package_list_label;
    delete package_list_title_label;
    delete package_path_label;
}

void ConfigureWindow::openAddPackageWindow()
{
    QWidget w;
    QString path = QFileDialog::getExistingDirectory(&w, QString("Directory"), "~", QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);

    std::string full_path = path.toStdString();
    std::vector<std::string> split_string;
    boost::split(split_string, full_path, boost::is_any_of("/"));
    std::pair<std::string, std::string> package_data;

    package_data.first = split_string.at(split_string.size() - 1);
    package_data.second = full_path;
    packages.push_back(package_data);

    std::string command;

    if (!std::filesystem::exists(output_path + "/src/"))
    {
        command = "mkdir -p " + output_path + "/src";
        if (system(command.c_str()))
        {
            std::cout << "Something went wrong with the generated folder file paths. May need to start fresh." << std::endl;
        }
    }

    if (package_data.second != "")
    {
        command = "ln -s " + package_data.second + " " + output_path + "/src/";

        int res = system(command.c_str());
        if (res)
        {
            std::cout << "Issue adding directory" << std::endl;
        }
        else
        {
            std::ofstream file;
            file.open(log_filename, std::ios_base::app);
            std::string output = package_data.first + "," + package_data.second;
            file << output;
            file << "\n";
            file.close();
        }
    }

    getWorkspaceLog();
}

void ConfigureWindow::openRemoveWindow()
{
    RemovePackagesWindow window(500, 500, log_filename, output_path, this);
    window.exec();
}

void ConfigureWindow::buildWorkspace()
{
    status_label->setText("Building\nPlease Wait...");

    QCoreApplication::processEvents();
    runBuild();
}

void ConfigureWindow::runBuild()
{
    std::string command;
    command = "cd " + output_path + " && colcon build";

    int res = system(command.c_str());
    if (res)
    {
        std::cout << "Error in building" << std::endl;
        status_label->setText("Failed");
    }
    else
    {
        status_label->setText("Message Packages Built Successfully");
    }
}

void ConfigureWindow::clearPackages()
{
    if (confirmDialog())
    {
        std::ofstream file;
        file.open(log_filename, std::ios_base::trunc);
        file.close();

        getWorkspaceLog();

        std::string command;
        command = "rm -r " + output_path + "/src";
        if (system(command.c_str()))
        {
            std::cout << "Failed to clear the workspace. Did it exist?" << std::endl;
        }

        if (std::filesystem::exists(output_path + "/install"))
        {
            std::cout << "Removing install" << std::endl;
            command = "rm -r " + output_path + "/install";
            if (system(command.c_str()))
            {
                std::cout << "Failed to delete install" << std::endl;
            }
        }

        if (std::filesystem::exists(output_path + "/build"))
        {
            std::cout << "Removing build" << std::endl;
            command = "rm -r " + output_path + "/build";
            if (system(command.c_str()))
            {
                std::cout << "Failed to delete build" << std::endl;
            }
        }

        if (std::filesystem::exists(output_path + "/log"))
        {
            std::cout << "Removing log" << std::endl;
            command = "rm -r " + output_path + "/log";
            if (system(command.c_str()))
            {
                std::cout << "Failed to delete log" << std::endl;
            }
        }
    }
}

void ConfigureWindow::getWorkspaceLog()
{
    packages.clear();
    if (std::filesystem::exists(log_filename))
    {
        std::ifstream file;
        file.open(log_filename);
        while (!file.eof())
        {
            std::string line;
            getline(file, line);

            std::vector<std::string> split_string;
            boost::split(split_string, line, boost::is_any_of(","));

            std::pair<std::string, std::string> package;
            if (split_string.size() > 1)
            {
                package.first = split_string.at(0);
                package.second = split_string.at(1);

                packages.push_back(package);
            }
        }
        file.close();
    }
    else
    {
        std::ofstream file;
        file.open(log_filename, std::ios_base::trunc);
        file.close();
    }

    std::string label_text1;
    std::string label_text2;
    for (size_t i = 0; i < packages.size(); i++)
    {
        label_text1 += packages.at(i).first + "\n";
        label_text2 += packages.at(i).second + "\n";
    }

    package_list_label->setText(QString(label_text1.c_str()));
    package_path_label->setText(QString(label_text2.c_str()));
}

bool ConfigureWindow::confirmDialog()
{
    QMessageBox msg_box;
    msg_box.setIcon(QMessageBox::Question);
    msg_box.setWindowTitle("Warning");
    msg_box.setText("This will remove all message files. Are you sure?");
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