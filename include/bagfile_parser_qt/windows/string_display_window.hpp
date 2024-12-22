#ifndef STRING_DISPLAY_WINDOW_HPP
#define STRING_DISPLAY_WINDOW_HPP

#include <QWidget>
#include <QStringList>
#include <QStringListIterator>
#include <QListWidget>
#include <QListWidgetItem>
#include <QVBoxLayout>
#include <QPushButton>
#include <QDialog>

class StringDisplayWindow : public QDialog
{
    Q_OBJECT

    public:
    StringDisplayWindow(const std::vector<std::string> &strings, const std::string &title = "Display", QWidget *parent = nullptr);
    StringDisplayWindow(const std::vector<std::string> &strings, QWidget *parent = nullptr, const std::string &title = "Display");
    ~StringDisplayWindow();

    private:
    QPushButton *done_button;
    
    void initialize(const std::vector<std::string> &strings, const std::string &title);

    private slots:
    void done();
};

#endif