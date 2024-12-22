#ifndef REMOVE_DEPENDS_WINDOW_HPP
#define REMOVE_DEPENDS_WINDOW_HPP

#include <QtWidgets>
#include <fstream>

class RemoveDependsWindow : public QDialog
{
    Q_OBJECT

    public:
    RemoveDependsWindow(const std::string &depends_file, QWidget* parent = nullptr);
    ~RemoveDependsWindow();

    private:
    QListWidget *list_widget;
    QPushButton *remove_button;

    std::string depends_filename;
    
    void initialize();

    private slots:
    void removeSelected();
};

#endif