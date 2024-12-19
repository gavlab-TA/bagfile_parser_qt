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

class RemovePackagesWindow : public QWidget
{
    Q_OBJECT

    public:
    RemovePackagesWindow(const int &width, const int &height, const std::string &package_log_filename);
    ~RemovePackagesWindow();

    private:
    QListWidget *list_widget;

};

#endif
