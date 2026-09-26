#pragma once
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QDialog>
#include <QtWidgets/QListWidget>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QLabel>
#include <QtWidgets/QPlainTextEdit>
#include <QtWidgets/QSpinBox>
#include <QtCore/QObject>
#include <QtCore/QThread>
#include <atomic>
#include <vector>

#include "bagfile_parser_qt/converter.hpp"

class TopicDialog : public QDialog
{
    Q_OBJECT
public:
    TopicDialog(const std::vector<TopicSummary>& topics, const std::vector<std::string>& selected, QWidget* parent = nullptr);
    std::vector<std::string> getSelected() const;

private slots:
    void selectAll();
    void selectNone();

private:
    QListWidget* list_;
    std::vector<std::string> topic_names_;
};

class ConvertWorker : public QObject
{
    Q_OBJECT
public:
    explicit ConvertWorker(ConvertOptions opts);
    void requestCancel() { this->cancel_ = true; }

public slots:
    void run();

signals:
    void logMessage(QString msg);
    void finished(bool success, QString error);

private:
    ConvertOptions opts_;
    std::atomic<bool> cancel_{false};
};

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    MainWindow();

private slots:
    void browseBag();
    void browseOutput();
    void chooseTopics();
    void onSkipLargeToggled(bool skip);
    void startConvert();
    void onLog(const QString& msg);
    void onConvertFinished(bool success, const QString& error);

private:
    void loadBag(const QString& dir);
    std::vector<TopicSummary> visibleTopics() const;
    void updateTopicsLabel();
    void updateStatus();
    bool readyToConvert() const;

    QLabel* status_label_;

    QPushButton* bag_button_;
    QLabel* bag_label_;

    QPushButton* output_button_;
    QLabel* output_label_;

    QPushButton* topics_button_;
    QCheckBox* skip_large_box_;
    QLabel* topics_label_;

    QCheckBox* mat_box_;
    QCheckBox* csv_box_;
    QSpinBox* threads_spin_;

    QPushButton* convert_button_;
    QPlainTextEdit* log_;

    std::string bag_path_;
    std::string output_path_;
    std::vector<TopicSummary> all_topics_;
    std::vector<std::string> selected_topics_;

    QThread* worker_thread_ = nullptr;
    ConvertWorker* worker_ = nullptr;
};
