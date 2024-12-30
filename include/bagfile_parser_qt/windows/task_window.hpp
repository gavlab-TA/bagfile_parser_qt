#ifndef TASK_WINDOW_HPP
#define TASK_WINDOW_HPP

#include <QWidget>
#include <QDialog>
#include <QVBoxLayout>
#include <QLabel>  

#include <string>

class TaskWindow : public QDialog
{
    Q_OBJECT
    
    public: 
    TaskWindow(const std::string &message, QWidget* parent = nullptr);
    ~TaskWindow();

    private:
    QVBoxLayout* layout;
    QLabel* label;

    public slots:
    void closeWindow();

    signals:
    void finished();
};

#endif