#include "bagfile_parser_qt/windows/main_window.hpp"

MainWindow::MainWindow(const int &width, const int &height) : QWidget()
{
    this->resize(width, height);

    this->main_layout = new QGridLayout();
    this->workflow_layout = new QVBoxLayout();
    this->bag_select_layout = new QVBoxLayout();

    this->bag_select_button = new QPushButton();
    this->bag_select_button->setText("Select Bagfile");
    this->bag_select_layout->addWidget(this->bag_select_button);

    this->bag_path_label = new QLabel();
    this->bag_path_label->setText("");
    this->bag_select_layout->addWidget(this->bag_path_label);

    this->configure_parser_button = new QPushButton();
    this->configure_parser_button->setText("Configure Message Dependencies");

    this->generate_parser_button = new QPushButton();
    this->generate_parser_button->setText("Generate Parser");

    this->select_topics_button = new QPushButton();
    this->select_topics_button->setText("Select Topics to Parse");

    this->configure_depends_button = new QPushButton();
    this->configure_depends_button->setText("Configure Dependencies");

    this->autoconfigure_generator_button = new QPushButton();
    this->autoconfigure_generator_button->setText("Autoconfigure Generator");

    bag_selected = false;

    this->workflow_layout->addLayout(this->bag_select_layout);
    this->workflow_layout->addWidget(this->select_topics_button);
    this->workflow_layout->addWidget(this->configure_depends_button);
    this->workflow_layout->addWidget(this->configure_parser_button);
    this->workflow_layout->addWidget(this->autoconfigure_generator_button);
    this->workflow_layout->addWidget(this->generate_parser_button);
    this->main_layout->addLayout(this->workflow_layout, 0, 0, Qt::AlignCenter);

    this->setLayout(this->main_layout);

    QObject::connect(this->bag_select_button, &QPushButton::released, this, &MainWindow::openBagSelectWindow);
    QObject::connect(this->configure_parser_button, &QPushButton::released, this, &MainWindow::openConfigureParserWindow);
    QObject::connect(this->select_topics_button, &QPushButton::released, this, &MainWindow::openSelectTopicsWindow);
    QObject::connect(this->configure_depends_button, &QPushButton::released, this, &MainWindow::openConfigureDependsWindow);
    QObject::connect(this->autoconfigure_generator_button, &QPushButton::released, this, &MainWindow::autoconfigureGenerator);
    QObject::connect(this->generate_parser_button, &QPushButton::released, this, &MainWindow::generateParser);

    this->show();

    this->share_path = ament_index_cpp::get_package_share_directory("bagfile_parser_qt") + "/package_path.txt";
    std::ifstream file;
    file.open(share_path.c_str());
    getline(file, this->output_path);
    file.close();
    this->output_path += "/generated";

    this->setupFileLocations();    
}

MainWindow::~MainWindow()
{
    delete main_layout;
    delete workflow_layout;
    delete bag_select_layout;
    delete bag_select_button;
    delete bag_path_label;
    delete status_label;
    delete configure_parser_button;
}

void MainWindow::setupFileLocations()
{
    std::string files_dir = output_path + "/files/";
    if (!boost::filesystem::exists(files_dir.c_str()))
    {
        std::string command = "mkdir -p " + files_dir;
        int res = system(command.c_str());
        if (res)
        {
            std::cout<<"Issue creating logfile space"<<std::endl;
        }
    }

    std::string parser_resource_dir = output_path + "/files/parser_files/";
    if (!boost::filesystem::exists(parser_resource_dir.c_str()))
    {
        std::string command = "mkdir -p " + parser_resource_dir;
        int res = system(command.c_str());
        if (res)
        {
            std::cout<<"Issue creating parser resource space"<<std::endl;
        }
    }

    std::string message_data_dir = output_path + "/files/parser_files/msg_data/";
    if (!boost::filesystem::exists(message_data_dir.c_str()))
    {
        std::string command = "mkdir -p " + message_data_dir;
        int res = system(command.c_str());
        if (res)
        {
            std::cout<<"Issue creating message data space"<<std::endl;
        }
    }
}

void MainWindow::openBagSelectWindow()
{
    QWidget w;
    QString path = QFileDialog::getExistingDirectory(&w, QString("Directory"), "/home/kyle/Data/Vegas2025_data/2024-12-16_VEGAS_run5", QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    bagfile_path = path.toStdString();

    if (bagfile_path != "")
    {
        bag_path_label->setText(path);

        rosbag2_cpp::readers::SequentialReader *reader;
        rosbag2_storage::StorageOptions storage_options{};
        rosbag2_cpp::ConverterOptions converter_options{};
        rosbag2_cpp::SerializationFormatConverterFactory factory;
        std::unique_ptr<rosbag2_cpp::converter_interfaces::SerializationFormatDeserializer> deserializer;

        storage_options.uri = bagfile_path;
        storage_options.storage_id = "sqlite3";
        converter_options.input_serialization_format = "cdr";
        converter_options.output_serialization_format = "cdr";
        deserializer = factory.load_deserializer("cdr");

        reader = new rosbag2_cpp::readers::SequentialReader();
        reader->open(storage_options, converter_options);
        data = reader->get_metadata();

        reader->close();

        bag_selected = true;
    }
    else
    {
        bag_path_label->setText("NO BAG FOUND");
    }
    // TODO: DISPLAY TOPICS AND MESSAGES
}

void MainWindow::openConfigureParserWindow()
{
    ConfigureWindow *configure_window = new ConfigureWindow(500, 500, output_path);
    configure_window->show();
}

void MainWindow::openSelectTopicsWindow()
{
    SelectTopicsWindow topics_window(500, 500, data, output_path, this);
    topics_window.exec();
}

void MainWindow::openConfigureDependsWindow()
{
    DependencyManagerWindow *dependency_window = new DependencyManagerWindow(500, 500, output_path); 
    dependency_window->show();
}

void MainWindow::autoconfigureGenerator()
{
    BagAnalyzer bag_analyzer(output_path + "/files/selected_topic_data.txt", output_path);
    MessageAnalyzer message_analyzer(output_path + "/files/parser_files/", output_path);
}

void MainWindow::generateParser()
{
    ParserGenerator parser_generator(output_path, bagfile_path);
}