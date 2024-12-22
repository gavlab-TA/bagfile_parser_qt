#include "bagfile_parser_qt/main_window.hpp"

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

    this->select_topics_button = new QPushButton();
    this->select_topics_button->setText("Select Topics to Parse");

    this->configure_depends_button = new QPushButton();
    this->configure_depends_button->setText("Configure Dependencies");

    bag_selected = false;


    this->workflow_layout->addLayout(this->bag_select_layout);
    this->workflow_layout->addWidget(this->select_topics_button);
    this->workflow_layout->addWidget(this->configure_depends_button);
    this->workflow_layout->addWidget(this->configure_parser_button);
    this->main_layout->addLayout(this->workflow_layout, 0, 0, Qt::AlignCenter);

    this->setLayout(this->main_layout);

    QObject::connect(this->bag_select_button, &QPushButton::released, this, &MainWindow::openBagSelectWindow);
    QObject::connect(this->configure_parser_button, &QPushButton::released, this, &MainWindow::openConfigureParserWindow);
    QObject::connect(this->select_topics_button, &QPushButton::released, this, &MainWindow::openSelectTopicsWindow);
    QObject::connect(this->configure_depends_button, &QPushButton::released, this, &MainWindow::openConfigureDependsWindow);

    this->show();

    this->share_path = ament_index_cpp::get_package_share_directory("bagfile_parser_qt") + "/package_path.txt";
    std::ifstream file;
    file.open(share_path.c_str());
    getline(file, this->output_path);
    file.close();
    this->output_path += "/generated";
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
    DependencyManagerWindow *dependency_window = new DependencyManagerWindow(500, 500, output_path, data); 
    dependency_window->show();
}