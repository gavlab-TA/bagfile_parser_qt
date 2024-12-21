#ifndef REMOVE_PACKAGES_WINDOW_HPP
#define REMOVE_PACAKGES_WINDOW_HPP

#include <fstream>
#include <iostream>
#include <boost/algorithm/string.hpp>

#include <QWidget>
#include <QStringList>
#include <QStringListIterator>
#include <QListWidget>
#include <QListWidgetItem>
#include <QVBoxLayout>
#include <QPushButton>
#include <QDialog>

class RemovePackagesWindow : public QDialog
{
    Q_OBJECT

    public:
    RemovePackagesWindow(const int &width, const int &height, const std::string &package_log_filename, const std::string &output_path, QWidget *parent = nullptr);
    ~RemovePackagesWindow();

    private:
    QListWidget *list_widget;
    QPushButton *remove_button;

    std::string package_log_filename;
    std::string output_path;

    void initialize();
    void removeButtonPushed();
};

#endif
