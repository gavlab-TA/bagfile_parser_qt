#include "bagfile_parser_qt/gui.hpp"

#include <QtCore/QFileInfo>
#include <QtCore/QString>
#include <QtCore/QThread>
#include <QtGui/QFont>
#include <QtGui/QFontMetrics>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QDialogButtonBox>
#include <QtWidgets/QFileDialog>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QListWidgetItem>
#include <QtWidgets/QMessageBox>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

#include <set>
#include <utility>

// ----- TopicDialog -----

TopicDialog::TopicDialog(const std::vector<TopicSummary>& topics, const std::vector<std::string>& selected, QWidget* parent)
    : QDialog(parent)
{
    this->setWindowTitle("Select Topics");
    this->resize(640, 480);

    this->list_ = new QListWidget(this);
    std::set<std::string> sel(selected.begin(), selected.end());
    for (const TopicSummary& t : topics)
    {
        QString text = QString("%1    %2    [%3]")
            .arg(QString::fromStdString(t.topic), -48)
            .arg(QString::fromStdString(t.msgtype), -38)
            .arg(static_cast<qulonglong>(t.count));
        QListWidgetItem* item = new QListWidgetItem(text, this->list_);
        item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
        bool is_selected = sel.empty() || sel.count(t.topic) > 0;
        item->setCheckState(is_selected ? Qt::Checked : Qt::Unchecked);
        item->setData(Qt::UserRole, QString::fromStdString(t.topic));
        this->topic_names_.push_back(t.topic);
    }
    QFont mono("Monospace");
    mono.setStyleHint(QFont::TypeWriter);
    this->list_->setFont(mono);

    QPushButton* all_btn = new QPushButton("Select All", this);
    QPushButton* none_btn = new QPushButton("Select None", this);
    QObject::connect(all_btn, &QPushButton::clicked, this, &TopicDialog::selectAll);
    QObject::connect(none_btn, &QPushButton::clicked, this, &TopicDialog::selectNone);

    QDialogButtonBox* btns = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    QObject::connect(btns, &QDialogButtonBox::accepted, this, &QDialog::accept);
    QObject::connect(btns, &QDialogButtonBox::rejected, this, &QDialog::reject);

    QHBoxLayout* sel_row = new QHBoxLayout;
    sel_row->addWidget(all_btn);
    sel_row->addWidget(none_btn);
    sel_row->addStretch();

    QVBoxLayout* main = new QVBoxLayout(this);
    main->addWidget(this->list_);
    main->addLayout(sel_row);
    main->addWidget(btns);
}

void TopicDialog::selectAll()
{
    for (int i = 0; i < this->list_->count(); i++)
    {
        this->list_->item(i)->setCheckState(Qt::Checked);
    }
}

void TopicDialog::selectNone()
{
    for (int i = 0; i < this->list_->count(); i++)
    {
        this->list_->item(i)->setCheckState(Qt::Unchecked);
    }
}

std::vector<std::string> TopicDialog::getSelected() const
{
    std::vector<std::string> out;
    for (int i = 0; i < this->list_->count(); i++)
    {
        if (this->list_->item(i)->checkState() == Qt::Checked)
        {
            out.push_back(this->list_->item(i)->data(Qt::UserRole).toString().toStdString());
        }
    }
    return out;
}

// ----- ConvertWorker -----

ConvertWorker::ConvertWorker(ConvertOptions opts) : opts_(std::move(opts)) {}

void ConvertWorker::run()
{
    ConvertCallbacks cbs;
    cbs.cancel = &this->cancel_;
    cbs.log = [this](const std::string& m)
    {
        emit logMessage(QString::fromStdString(m));
    };

    try
    {
        convert(this->opts_, cbs);
        emit finished(true, QString());
    }
    catch (const std::exception& e)
    {
        emit finished(false, QString::fromStdString(e.what()));
    }
    catch (...)
    {
        emit finished(false, "unknown error");
    }
}

// ----- MainWindow -----

MainWindow::MainWindow()
{
    this->setWindowTitle("bagfile_parser_qt");
    this->resize(640, 720);

    QWidget* central = new QWidget(this);
    this->setCentralWidget(central);

    this->status_label_ = new QLabel("Select a bag file to begin.");
    this->status_label_->setAlignment(Qt::AlignCenter);

    this->bag_button_ = new QPushButton("Select Bagfile");
    this->bag_label_ = new QLabel("<none>");
    this->bag_label_->setAlignment(Qt::AlignCenter);

    this->output_button_ = new QPushButton("Select Output Directory");
    this->output_label_ = new QLabel("<none>");
    this->output_label_->setAlignment(Qt::AlignCenter);

    this->topics_button_ = new QPushButton("Select Topics");
    this->topics_button_->setEnabled(false);
    this->topics_label_ = new QLabel("<no bag loaded>");
    this->topics_label_->setAlignment(Qt::AlignCenter);

    this->mat_box_ = new QCheckBox(".mat");
    this->mat_box_->setChecked(true);
    this->csv_box_ = new QCheckBox(".csv");

    this->threads_spin_ = new QSpinBox;
    this->threads_spin_->setRange(1, 256);
    int hc = static_cast<int>(QThread::idealThreadCount());
    this->threads_spin_->setValue(hc > 0 ? hc : 4);
    // Some styles (e.g. Qt 6's windows11) under-report the size hint and clip
    // the digits; reserve room for "256" plus the up/down buttons.
    this->threads_spin_->setMinimumWidth(
        this->threads_spin_->fontMetrics().horizontalAdvance(QStringLiteral("0000")) + 64);

    this->convert_button_ = new QPushButton("Convert");
    this->convert_button_->setEnabled(false);

    this->log_ = new QPlainTextEdit;
    this->log_->setReadOnly(true);
    QFont mono("Monospace");
    mono.setStyleHint(QFont::TypeWriter);
    this->log_->setFont(mono);

    QGridLayout* bag_grid = new QGridLayout;
    bag_grid->addWidget(this->bag_button_, 0, 0);
    bag_grid->addWidget(this->bag_label_, 1, 0, Qt::AlignCenter);

    QGridLayout* out_grid = new QGridLayout;
    out_grid->addWidget(this->output_button_, 0, 0);
    out_grid->addWidget(this->output_label_, 1, 0, Qt::AlignCenter);

    QGridLayout* top_grid = new QGridLayout;
    top_grid->addWidget(this->topics_button_, 0, 0);
    top_grid->addWidget(this->topics_label_, 1, 0, Qt::AlignCenter);

    QHBoxLayout* opts_row = new QHBoxLayout;
    opts_row->addWidget(new QLabel("Output:"));
    opts_row->addWidget(this->mat_box_);
    opts_row->addWidget(this->csv_box_);
    opts_row->addStretch();
    opts_row->addWidget(new QLabel("Threads:"));
    opts_row->addWidget(this->threads_spin_);

    QVBoxLayout* root = new QVBoxLayout(central);
    root->addWidget(this->status_label_);
    root->addLayout(bag_grid);
    root->addLayout(out_grid);
    root->addLayout(top_grid);
    root->addLayout(opts_row);
    root->addWidget(this->convert_button_);
    root->addWidget(new QLabel("Log:"));
    root->addWidget(this->log_, 1);

    QObject::connect(this->bag_button_, &QPushButton::clicked, this, &MainWindow::browseBag);
    QObject::connect(this->output_button_, &QPushButton::clicked, this, &MainWindow::browseOutput);
    QObject::connect(this->topics_button_, &QPushButton::clicked, this, &MainWindow::chooseTopics);
    QObject::connect(this->convert_button_, &QPushButton::clicked, this, &MainWindow::startConvert);
}

void MainWindow::browseBag()
{
    QString dir = QFileDialog::getExistingDirectory(this, "Select Bag Directory");
    if (dir.isEmpty())
    {
        return;
    }

    this->bag_path_ = dir.toStdString();
    this->bag_label_->setText(dir);

    // Check for missing MCAP summary sections before loading
    std::vector<std::string> missing = findMissingSummaries(this->bag_path_);
    if (!missing.empty())
    {
        QString file_list;
        for (const std::string& f : missing)
        {
            file_list += QString::fromStdString(f) + "\n";
        }
        QMessageBox::StandardButton reply = QMessageBox::question(
            this, "Missing MCAP Summary",
            QString("The following MCAP file(s) are missing their summary/index section:\n\n%1\n"
                    "This can happen if recording was interrupted. Loading will fall back to a "
                    "full sequential scan, which may be slow on large files.\n\n"
                    "Continue with fallback scan?").arg(file_list),
            QMessageBox::Yes | QMessageBox::No);
        if (reply == QMessageBox::No)
        {
            this->log_->appendPlainText("Bag load cancelled — missing summary section.");
            this->all_topics_.clear();
            this->updateStatus();
            return;
        }
        this->log_->appendPlainText("Summary missing, falling back to sequential scan...");
    }

    this->log_->appendPlainText("Loading topics from " + dir + "...");
    try
    {
        this->all_topics_ = listTopics(this->bag_path_);
    }
    catch (const std::exception& e)
    {
        QMessageBox::warning(this, "Error", QString("Failed to read bag: ") + e.what());
        this->all_topics_.clear();
    }

    if (this->all_topics_.empty())
    {
        this->topics_label_->setText("<no topics found>");
        this->topics_button_->setEnabled(false);
    }
    else
    {
        this->selected_topics_.clear();
        for (const TopicSummary& t : this->all_topics_)
        {
            this->selected_topics_.push_back(t.topic);
        }
        this->topics_label_->setText(QString("%1 / %2 selected")
                                   .arg(this->selected_topics_.size())
                                   .arg(this->all_topics_.size()));
        this->topics_button_->setEnabled(true);
        this->log_->appendPlainText(QString("Found %1 topics.").arg(this->all_topics_.size()));
    }
    this->updateStatus();
}

void MainWindow::browseOutput()
{
    QString dir = QFileDialog::getExistingDirectory(this, "Select Output Directory");
    if (dir.isEmpty())
    {
        return;
    }
    this->output_path_ = dir.toStdString();
    this->output_label_->setText(dir);
    this->updateStatus();
}

void MainWindow::chooseTopics()
{
    if (this->all_topics_.empty())
    {
        return;
    }
    TopicDialog dlg(this->all_topics_, this->selected_topics_, this);
    if (dlg.exec() == QDialog::Accepted)
    {
        this->selected_topics_ = dlg.getSelected();
        this->topics_label_->setText(QString("%1 / %2 selected")
                                   .arg(this->selected_topics_.size())
                                   .arg(this->all_topics_.size()));
        this->updateStatus();
    }
}

bool MainWindow::readyToConvert() const
{
    return !this->bag_path_.empty()
        && !this->output_path_.empty()
        && !this->selected_topics_.empty();
}

void MainWindow::updateStatus()
{
    if (this->bag_path_.empty())
    {
        this->status_label_->setText("Select a bag file to begin.");
    }
    else if (this->all_topics_.empty())
    {
        this->status_label_->setText("No topics found.");
    }
    else if (this->output_path_.empty())
    {
        this->status_label_->setText("Select an output directory.");
    }
    else if (this->selected_topics_.empty())
    {
        this->status_label_->setText("Select at least one topic.");
    }
    else
    {
        this->status_label_->setText("Ready.");
    }
    this->convert_button_->setEnabled(this->readyToConvert() && this->worker_thread_ == nullptr);
}

void MainWindow::startConvert()
{
    if (!this->readyToConvert() || this->worker_thread_)
    {
        return;
    }

    ConvertOptions opts;
    opts.bag_path = this->bag_path_;
    opts.output_dir = this->output_path_;
    opts.topics = this->selected_topics_;
    opts.threads = this->threads_spin_->value();
    bool want_mat = this->mat_box_->isChecked();
    bool want_csv = this->csv_box_->isChecked();
    if (!want_mat && !want_csv)
    {
        QMessageBox::warning(this, "No format selected",
            "Select at least one of .mat or .csv.");
        return;
    }
    opts.format = (want_mat && want_csv) ? OutputFormat::BOTH
                : want_csv               ? OutputFormat::CSV
                                         : OutputFormat::MAT;

    this->log_->appendPlainText("=== Converting ===");
    this->convert_button_->setEnabled(false);
    this->bag_button_->setEnabled(false);
    this->output_button_->setEnabled(false);
    this->topics_button_->setEnabled(false);

    this->worker_thread_ = new QThread(this);
    this->worker_ = new ConvertWorker(opts);
    this->worker_->moveToThread(this->worker_thread_);

    QObject::connect(this->worker_thread_, &QThread::started, this->worker_, &ConvertWorker::run);
    QObject::connect(this->worker_, &ConvertWorker::logMessage, this, &MainWindow::onLog);
    QObject::connect(this->worker_, &ConvertWorker::finished, this, &MainWindow::onConvertFinished);
    QObject::connect(this->worker_, &ConvertWorker::finished, this->worker_thread_, &QThread::quit);
    QObject::connect(this->worker_, &ConvertWorker::finished, this->worker_, &QObject::deleteLater);
    QObject::connect(this->worker_thread_, &QThread::finished, this->worker_thread_, &QObject::deleteLater);

    this->worker_thread_->start();
    this->status_label_->setText("Converting...");
}

void MainWindow::onLog(const QString& msg)
{
    this->log_->appendPlainText(msg);
}

void MainWindow::onConvertFinished(bool success, const QString& error)
{
    this->worker_thread_ = nullptr;
    this->worker_ = nullptr;
    this->bag_button_->setEnabled(true);
    this->output_button_->setEnabled(true);
    this->topics_button_->setEnabled(true);
    if (success)
    {
        this->status_label_->setText("Done.");
    }
    else
    {
        this->status_label_->setText("Failed.");
        QMessageBox::warning(this, "Convert failed", error);
    }
    this->updateStatus();
}
