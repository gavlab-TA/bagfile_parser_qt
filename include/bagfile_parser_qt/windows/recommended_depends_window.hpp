#ifndef RECOMMENDED_DEPENDS_WINDOW_HPP
#define RECOMMENDED_DEPENDS_WINDOW_HPP

#include <QWidget>
#include <QStringList>
#include <QStringListIterator>
#include <QListWidget>
#include <QListWidgetItem>
#include <QVBoxLayout>
#include <QPushButton>
#include <QDialog>
#include <fstream>

class RecommendedDependsWindow : public QDialog
{
    Q_OBJECT

    public:
    RecommendedDependsWindow(const std::vector<std::string> &recommended_depends, const std::string &output_path, QWidget *parent = nullptr);
    ~RecommendedDependsWindow();

    private:
    // Widgets
    QListWidget *list_widget;
    QPushButton *approve_button;

    std::string output_path;

    private slots:
    void approveDependencies();
};

#endif