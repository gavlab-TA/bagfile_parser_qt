#include "bagfile_parser_qt/configure_window.hpp"

ConfigureWindow::ConfigureWindow(int width, int height) : QWidget()
{
    this->resize(width, height);

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

    QObject::connect(this->add_package_button, &QPushButton::released, [=]
                     { this->addPackageButtonPushed(); });
    QObject::connect(this->remove_package_button, &QPushButton::released, [=]
                     { this->removePackageButtonPushed(); });
    QObject::connect(this->build_workspace_button, &QPushButton::released, [=]
                     { this->buildWorkspaceButtonPushed(); });
    QObject::connect(this->clear_packages_button, &QPushButton::released, [=]
                     { this->clearPacakgesButtonPushed(); });

    this->main_layout->addWidget(this->package_list_title_label, 0, 0, 1, 3, Qt::AlignCenter);
    this->main_layout->addWidget(this->package_list_label, 1, 0, 1, 1, Qt::AlignCenter);
    this->main_layout->addWidget(this->package_path_label, 1, 1, 1, 2, Qt::AlignCenter);
    this->main_layout->addWidget(this->add_package_button, 2, 0, Qt::AlignCenter);
    this->main_layout->addWidget(this->remove_package_button, 2, 1, Qt::AlignCenter);
    this->main_layout->addWidget(this->build_workspace_button, 2, 2, Qt::AlignCenter);
    this->main_layout->addWidget(this->status_label, 3, 0, Qt::AlignCenter);
    this->main_layout->addWidget(this->clear_packages_button, 4, 0, Qt::AlignCenter);

    this->setLayout(main_layout);

    this->log_filename = "workspace_log.txt";

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

void ConfigureWindow::addPackageButtonPushed()
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

    std::ofstream file;
    file.open(log_filename, std::ios_base::app);
    std::string output = package_data.first + "," + package_data.second;
    file << output;
    file << "\n";
    file.close();

    getWorkspaceLog();
}

void ConfigureWindow::removePackageButtonPushed()
{
}

void ConfigureWindow::buildWorkspaceButtonPushed()
{
}

void ConfigureWindow::clearPacakgesButtonPushed()
{
    std::ofstream file;
    file.open(log_filename, std::ios_base::trunc);
    file.close();

    getWorkspaceLog();
    // TODO: Delete workspace src
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