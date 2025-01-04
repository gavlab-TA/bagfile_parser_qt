#include "bagfile_parser_qt/windows/main_window.hpp"

MainWindow::MainWindow(const int &width, const int &height) : QWidget()
{
    this->resize(width, height);

    // Init Layouts
    this->main_layout = new QGridLayout();
    this->workflow_layout = new QVBoxLayout();
    this->parser_generator_layout = new QGridLayout();
    this->bag_select_layout = new QGridLayout();
    this->output_directory_layout = new QGridLayout();

    // Init Labels
    this->matlab_parser_label = new QLabel();
    this->csv_parser_label = new QLabel();
    this->bag_path_label = new QLabel();
    this->output_directory_label = new QLabel();
    this->status_label = new QLabel();

    // Init Buttons
    this->bag_select_button = new QPushButton();
    this->output_directory_select_button = new QPushButton();
    this->configure_parser_button = new QPushButton();
    this->generate_parser_button = new QPushButton();
    this->select_topics_button = new QPushButton();
    this->configure_depends_button = new QPushButton();
    this->autoconfigure_generator_button = new QPushButton();
    this->build_csv_parser_button = new QPushButton();
    this->run_csv_parser_button = new QPushButton();
    this->generate_csv_matlab_parser_button = new QPushButton();
    this->generate_matlab_parser_button = new QPushButton();
    this->build_matlab_parser_button = new QPushButton();
    this->reset_workspace_button = new QPushButton();
    this->run_matlab_parser_button = new QPushButton();

    // Set Label Text
    this->matlab_parser_label->setText("Matlab Parser");
    this->csv_parser_label->setText("CSV Parser");
    this->bag_path_label->setText("");
    this->status_label->setText("Ready");

    // Set Button Text
    this->bag_select_button->setText("Select Bagfile");
    this->output_directory_select_button->setText("Select Data Output Directory");
    this->configure_parser_button->setText("Manage Package Dependencies");
    this->generate_parser_button->setText("Generate CSV Parser");
    this->select_topics_button->setText("Select Topics to Parse");
    this->configure_depends_button->setText("Select Dependencies");
    this->autoconfigure_generator_button->setText("Run Message Analysis");
    this->build_csv_parser_button->setText("Build CSV Parser");
    this->run_csv_parser_button->setText("Run CSV Parser");
    this->generate_csv_matlab_parser_button->setText("Generate Matlab Parser");
    this->generate_matlab_parser_button->setText("Generate Matlab Parser");
    this->build_matlab_parser_button->setText("Build Matlab Parser");
    this->reset_workspace_button->setText("Reset Parser");
    this->run_matlab_parser_button->setText("Run Matlab Parser");

    // Set Additional Button Settings
    this->generate_parser_button->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    this->build_csv_parser_button->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    this->run_csv_parser_button->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    this->generate_csv_matlab_parser_button->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    this->generate_matlab_parser_button->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    this->build_matlab_parser_button->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    this->run_matlab_parser_button->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    // Pack Bag Select Layout
    this->bag_select_layout->addWidget(this->bag_select_button, 0, 0, 1, 1);
    this->bag_select_layout->addWidget(this->bag_path_label, 1, 0, 1, 1, Qt::AlignCenter);

    // Pack Data Output Layout
    this->output_directory_layout->addWidget(this->output_directory_select_button, 0, 0, 1, 1);
    this->output_directory_layout->addWidget(this->output_directory_label, 1, 0, 1, 1, Qt::AlignCenter);

    // Pack Parser Generator Layout
    this->parser_generator_layout->addWidget(this->csv_parser_label, 0, 0, Qt::AlignCenter);
    this->parser_generator_layout->addWidget(this->matlab_parser_label, 0, 1, Qt::AlignCenter);
    this->parser_generator_layout->addWidget(this->generate_parser_button, 1, 0);
    this->parser_generator_layout->addWidget(this->build_csv_parser_button, 2, 0);
    this->parser_generator_layout->addWidget(this->run_csv_parser_button, 3, 0);
    this->parser_generator_layout->addWidget(this->generate_csv_matlab_parser_button, 4, 0);
    this->parser_generator_layout->addWidget(this->generate_matlab_parser_button, 1, 1);
    this->parser_generator_layout->addWidget(this->build_matlab_parser_button, 2, 1);
    this->parser_generator_layout->addWidget(this->run_matlab_parser_button, 3, 1);

    // Pack Main Layouts
    this->workflow_layout->addLayout(this->bag_select_layout);
    this->workflow_layout->addLayout(this->output_directory_layout);
    this->workflow_layout->addWidget(this->select_topics_button);
    this->workflow_layout->addWidget(this->configure_depends_button);
    this->workflow_layout->addWidget(this->configure_parser_button);
    this->workflow_layout->addWidget(this->autoconfigure_generator_button);
    this->workflow_layout->addLayout(this->parser_generator_layout);
    this->workflow_layout->addWidget(this->reset_workspace_button);

    this->main_layout->addLayout(this->workflow_layout, 0, 0, Qt::AlignCenter);
    this->main_layout->addWidget(this->status_label, 1, 0, 1, 1, Qt::AlignCenter);
    this->setLayout(this->main_layout);

    // Set Button Callbacks
    QObject::connect(this->bag_select_button, &QPushButton::released, this, &MainWindow::openBagSelectWindow);
    QObject::connect(this->output_directory_select_button, &QPushButton::released, this, &MainWindow::openOutputDirectorySelectWindow);
    QObject::connect(this->configure_parser_button, &QPushButton::released, this, &MainWindow::openConfigureParserWindow);
    QObject::connect(this->select_topics_button, &QPushButton::released, this, &MainWindow::openSelectTopicsWindow);
    QObject::connect(this->configure_depends_button, &QPushButton::released, this, &MainWindow::openConfigureDependsWindow);
    QObject::connect(this->autoconfigure_generator_button, &QPushButton::released, this, &MainWindow::autoconfigureGenerator);
    QObject::connect(this->generate_parser_button, &QPushButton::released, this, &MainWindow::generateCsvParser);
    QObject::connect(this->build_csv_parser_button, &QPushButton::released, this, &MainWindow::buildCsvParser);
    QObject::connect(this->run_csv_parser_button, &QPushButton::released, this, &MainWindow::runCsvParser);
    QObject::connect(this->generate_csv_matlab_parser_button, &QPushButton::released, this, &MainWindow::generateCsvMatlabParser);
    QObject::connect(this->generate_matlab_parser_button, &QPushButton::released, this, &MainWindow::generateMatlabParser);
    QObject::connect(this->build_matlab_parser_button, &QPushButton::released, this, &MainWindow::buildMatlabParser);
    QObject::connect(this->run_matlab_parser_button, &QPushButton::released, this, &MainWindow::runMatlabParser);
    QObject::connect(this->reset_workspace_button, &QPushButton::released, this, &MainWindow::resetParser);

    this->show();

    // Default Bag Selected
    bag_selected = false;

    // Get Local Path Info
    this->share_path = ament_index_cpp::get_package_share_directory("bagfile_parser_qt") + "/package_path.txt";
    std::ifstream file;
    file.open(share_path.c_str());
    getline(file, this->output_path);
    file.close();
    this->output_path += "/generated";

    // Setup Local Files 
    this->setupFileLocations();
    this->mutex = new QMutex();

    // Prompt Paths from User
    this->openBagSelectWindow();
    this->openOutputDirectorySelectWindow();
    this->openSelectTopicsWindow();
    this->openConfigureDependsWindow();
    this->openConfigureParserWindow();
}

MainWindow::~MainWindow()
{

}

void MainWindow::setupFileLocations()
{
    // Create Space for GUI and Parser Data
    std::string files_dir = output_path + "/files/";
    if (!boost::filesystem::exists(files_dir.c_str()))
    {
        std::string command = "mkdir -p " + files_dir;
        int res = system(command.c_str());
        if (res)
        {
            std::cout << "Issue creating logfile space" << std::endl;
        }
    }

    // Create space for parser resource files
    std::string parser_resource_dir = output_path + "/files/parser_files/";
    if (!boost::filesystem::exists(parser_resource_dir.c_str()))
    {
        std::string command = "mkdir -p " + parser_resource_dir;
        int res = system(command.c_str());
        if (res)
        {
            std::cout << "Issue creating parser resource space" << std::endl;
        }
    }

    // Create space for message definition resoureces
    std::string message_data_dir = output_path + "/files/parser_files/msg_data/";
    if (!boost::filesystem::exists(message_data_dir.c_str()))
    {
        std::string command = "mkdir -p " + message_data_dir;
        int res = system(command.c_str());
        if (res)
        {
            std::cout << "Issue creating message data space" << std::endl;
        }
    }
}

void MainWindow::openBagSelectWindow()
{
    mutex->lock();

    // Select Bagfile Directory
    QString path = QFileDialog::getExistingDirectory(this, QString("Select Bagfile to Parse"), "~", QFileDialog::DontResolveSymlinks | QFileDialog::DontUseNativeDialog);
    bagfile_path = path.toStdString();

    // Read and store bag metadata
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

    mutex->unlock();
}

void MainWindow::openOutputDirectorySelectWindow()
{
    mutex->lock();

    // Select Output Directory
    QString path = QFileDialog::getExistingDirectory(this, QString("Select Data Output Directory"), "~", QFileDialog::DontResolveSymlinks | QFileDialog::DontUseNativeDialog);
    data_output_path = path.toStdString();

    // Display Selected Output Path in Main Window
    output_directory_label->setText(data_output_path.c_str());

    mutex->unlock();
}

void MainWindow::openConfigureParserWindow()
{
    mutex->lock();

    ConfigureWindow *configure_window = new ConfigureWindow(500, 500, output_path, this);
    configure_window->exec();

    mutex->unlock();
}

void MainWindow::openSelectTopicsWindow()
{
    mutex->lock();
    
    SelectTopicsWindow topics_window(500, 500, data, output_path, this);
    topics_window.exec();

    mutex->unlock();
}

void MainWindow::openConfigureDependsWindow()
{
    mutex->lock();

    DependencyManagerWindow *dependency_window = new DependencyManagerWindow(500, 500, output_path, this);
    dependency_window->exec();

    mutex->unlock();
}

void MainWindow::autoconfigureGenerator()
{
    mutex->lock();
    status_label->setText("Pulling Message Data");
    QApplication::processEvents();

    AnalyzeMessagesWorker* worker = new AnalyzeMessagesWorker(mutex, output_path);
    QThread* thread = new QThread();
    TaskWindow* task_window = new TaskWindow("Generating Message Resources... Please Wait...", this);

    worker->moveToThread(thread);

    QObject::connect(thread, &QThread::started, worker, &AnalyzeMessagesWorker::runAnalyzeMessagesThread);
    QObject::connect(worker, &AnalyzeMessagesWorker::workFinished, thread, &QThread::quit);
    QObject::connect(worker, &AnalyzeMessagesWorker::workFinished, this, &MainWindow::resetStatusLabel);
    QObject::connect(worker, &AnalyzeMessagesWorker::workFinished, worker, &AnalyzeMessagesWorker::deleteLater);
    QObject::connect(thread, &QThread::finished, thread, &QThread::deleteLater);
    QObject::connect(worker, &AnalyzeMessagesWorker::workFinished, task_window, &TaskWindow::closeWindow);

    mutex->unlock();

    thread->start();
    task_window->exec();
}

void MainWindow::generateCsvParser()
{
    mutex->lock();

    status_label->setText("Generating Parser");
    QApplication::processEvents();
    CsvParserGenerator csv_parser_generator(output_path, bagfile_path, data_output_path);
    status_label->setText("Ready");

    mutex->unlock();
}

void MainWindow::buildCsvParser()
{
    mutex->lock();

    status_label->setText("Building Parser");
    QApplication::processEvents();

    BuildCsvParserWorker* worker = new BuildCsvParserWorker(mutex, output_path);
    QThread* thread = new QThread();
    TaskWindow* task_window = new TaskWindow("Building CSV Parser... Please Wait...", this);

    worker->moveToThread(thread);

    QObject::connect(thread, &QThread::started, worker, &BuildCsvParserWorker::runBuildCsvParserThread);
    QObject::connect(worker, &BuildCsvParserWorker::workFinished, thread, &QThread::quit);
    QObject::connect(worker, &BuildCsvParserWorker::workFinished, task_window, &TaskWindow::closeWindow);
    QObject::connect(worker, &BuildCsvParserWorker::workFinished, worker, &BuildCsvParserWorker::deleteLater);
    QObject::connect(thread, &QThread::finished, thread, &QThread::deleteLater);
    QObject::connect(worker, &BuildCsvParserWorker::success, this, &MainWindow::resetStatusLabel);
    QObject::connect(worker, &BuildCsvParserWorker::failure, this, &MainWindow::displayStatusError);
    
    mutex->unlock();

    thread->start();
    task_window->exec();
}

void MainWindow::runCsvParser()
{
    mutex->lock();

    status_label->setText("Running CSV Parser");
    QApplication::processEvents();

    RunCsvParserWorker* worker = new RunCsvParserWorker(mutex, output_path);
    QThread* thread = new QThread();
    TaskWindow* task_window = new TaskWindow("Running CSV Parser... Please Wait...", this);

    worker->moveToThread(thread);

    QObject::connect(thread, &QThread::started, worker, &RunCsvParserWorker::runCsvParserThread);
    QObject::connect(worker, &RunCsvParserWorker::workFinished, thread, &QThread::quit);
    QObject::connect(worker, &RunCsvParserWorker::workFinished, this, &MainWindow::resetStatusLabel);
    QObject::connect(worker, &RunCsvParserWorker::workFinished, worker, &RunCsvParserWorker::deleteLater);
    QObject::connect(thread, &QThread::finished, thread, &QThread::deleteLater);
    QObject::connect(worker, &RunCsvParserWorker::workFinished, task_window, &TaskWindow::closeWindow);
    
    mutex->unlock();

    thread->start();
    task_window->exec();
}

void MainWindow::generateCsvMatlabParser()
{
    mutex->lock();

    status_label->setText("Generating Parser");
    QApplication::processEvents();
    CsvMatlabGenerator csv_matlab_generator(data_output_path, output_path);
    status_label->setText("Ready");

    mutex->unlock();
}

void MainWindow::generateMatlabParser()
{
    mutex->lock();

    status_label->setText("Generating Parser");
    QApplication::processEvents();
    MatlabParserGenerator matlab_parser_generator(output_path, bagfile_path, data_output_path);
    std::string command;

    // Symlink TinyMAT to generated workspace for use with the Matlab parser
    if (std::filesystem::exists(output_path + "/src/TinyMAT"))
    {
        command = "rm " + output_path + "/src/TinyMAT";
        if (system(command.c_str()))
        {
            std::cout << "Issue removing old symlinks" << std::endl;
        }
    }

    command = "ln -s " + output_path + "/../external/TinyMAT " + output_path + "/src/";

    if (system(command.c_str()))
    {
        std::cout << "Issue Creating symlink for TinyMAT" << std::endl;
    }
    status_label->setText("Ready");

    mutex->unlock();
}

void MainWindow::buildMatlabParser()
{
    mutex->lock();

    status_label->setText("Building Parser");
    QApplication::processEvents();
    
    BuildMatlabParserWorker* worker = new BuildMatlabParserWorker(mutex, output_path);
    QThread* thread = new QThread();
    worker->moveToThread(thread);

    TaskWindow* task_window = new TaskWindow("Building MATLAB Parser Workspace... Please Wait...", this);

    QObject::connect(thread, &QThread::started, worker, &BuildMatlabParserWorker::runBuildMatlabParserThread);
    QObject::connect(worker, &BuildMatlabParserWorker::workFinished, thread, &QThread::quit);
    QObject::connect(worker, &BuildMatlabParserWorker::workFinished, worker, &BuildMatlabParserWorker::deleteLater);
    QObject::connect(thread, &QThread::finished, thread, &QThread::deleteLater);
    QObject::connect(thread, &QThread::finished, task_window, &TaskWindow::closeWindow);
    QObject::connect(worker, &BuildMatlabParserWorker::success, this, &MainWindow::resetStatusLabel);
    QObject::connect(worker, &BuildMatlabParserWorker::failure, this, &MainWindow::displayStatusError);

    mutex->unlock();

    thread->start();
    task_window->exec();
}

void MainWindow::runMatlabParser()
{
    mutex->lock();
    status_label->setText("Running Matlab Parser");
    QApplication::processEvents();

    RunMatlabParserWorker* worker = new RunMatlabParserWorker(mutex, output_path);
    QThread* thread = new QThread();
    worker->moveToThread(thread);

    TaskWindow* task_window = new TaskWindow("MATLAB Parser Running... Please Wait...", this);

    QObject::connect(thread, &QThread::started, worker, &RunMatlabParserWorker::runMatlabParserThread);
    QObject::connect(worker, &RunMatlabParserWorker::workFinished, thread, &QThread::quit);
    QObject::connect(worker, &RunMatlabParserWorker::workFinished, worker, &RunMatlabParserWorker::deleteLater);
    QObject::connect(worker, &RunMatlabParserWorker::workFinished, this, &MainWindow::resetStatusLabel);
    QObject::connect(thread, &QThread::finished, thread, &QThread::deleteLater);   
    QObject::connect(thread, &QThread::finished, task_window, &TaskWindow::closeWindow);
    mutex->unlock();

    thread->start();
    task_window->exec();
}

void MainWindow::resetParser()
{
    mutex->lock();

    // Run Confirmation Diaglog
    QMessageBox msg_box(this);
    msg_box.setIcon(QMessageBox::Question);
    msg_box.setWindowTitle("Warning");
    msg_box.setText("This will remove all message files. Are you sure?");
    msg_box.setStandardButtons(QMessageBox::Yes | QMessageBox::Cancel);
    msg_box.setDefaultButton(QMessageBox::Cancel);

    int result = msg_box.exec();

    // Remove files generated directory and restart window
    if (result == QMessageBox::Yes)
    {
        std::string command = "cd " + output_path + " && rm -r install log src build files";
        if (system(command.c_str()))
        {
            std::cout << "Issue clearing out " + output_path << std::endl;
        }

        QApplication::quit();
        QProcess::startDetached(qApp->arguments()[0], qApp->arguments());
    }

    mutex->unlock();
}

void MainWindow::resetStatusLabel()
{
    this->mutex->lock();

    this->status_label->setText("Ready");

    this->mutex->unlock();
}

void MainWindow::displayStatusError()
{
    this->mutex->lock();

    this->status_label->setText("Ready - Task finished with error");

    this->mutex->unlock();
}

// --------------------------------------------------------------------------------------------------------
// Thread Worker Objects
// --------------------------------------------------------------------------------------------------------

void AnalyzeMessagesWorker::runAnalyzeMessagesThread()
{
    this->mutex->lock();

    BagAnalyzer bag_analyzer(this->output_path + "/files/selected_topic_data.txt", this->output_path);
    MessageAnalyzer message_analyzer(this->output_path + "/files/parser_files/", this->output_path);

    this->mutex->unlock();

    emit this->workFinished();
}

void RunMatlabParserWorker::runMatlabParserThread()
{
    this->mutex->lock();
    QApplication::processEvents();
    std::string command = "bash -c 'source " + this->output_path + "/install/setup.bash && ros2 run matlab_parser matlab_parser'";
    if (system(command.c_str()))
    {
        std::cout << "Issue running parser" << std::endl;
    }
    this->mutex->unlock();

    emit this->workFinished();
}

void BuildMatlabParserWorker::runBuildMatlabParserThread()
{
    this->mutex->lock();
    std::string command = "cd " + this->output_path + " && colcon build --cmake-args -DCMAKE_BUILD_TYPE=Release --symlink-install";
    if (system(command.c_str()))
    {
        emit this->failure();
        std::cout << "Issue Compiling Parser" << std::endl;
    }
    else
    {
        emit this->success();
    }
    this->mutex->unlock();

    emit this->workFinished();
}

void BuildCsvParserWorker::runBuildCsvParserThread()
{
    this->mutex->lock();

    std::string command = "cd " + this->output_path + " && colcon build --cmake-args -DCMAKE_BUILD_TYPE=Release --symlink-install";
    if (system(command.c_str()))
    {
        std::cout << "Issue Compiling Parser" << std::endl;
        emit this->failure();
    }
    else
    {
        emit this->success();
    }

    this->mutex->unlock();

    emit this->workFinished();
}

void RunCsvParserWorker::runCsvParserThread()
{
    this->mutex->lock();

    std::string command = "bash -c 'source " + this->output_path + "/install/setup.bash && ros2 launch rosbag2_parser rosbag2_parser.launch.py'";
    if (system(command.c_str()))
    {
        std::cout << "Issue running parser" << std::endl;
    }

    this->mutex->unlock();

    emit this->workFinished();
}