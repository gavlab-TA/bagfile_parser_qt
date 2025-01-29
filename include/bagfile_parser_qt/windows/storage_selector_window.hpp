#ifndef STORAGE_SELECTOR_WINDOW_HPP
#define STORAGE_SELECTOR_WINDOW_HPP

#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QComboBox>
#include <QDialog>
#include <QPushButton>
#include <QVBoxLayout>

class StorageSelectorWindow : public QDialog
{
    Q_OBJECT

public:
    StorageSelectorWindow(const int &width, const int &height, QWidget *parent = nullptr);
    ~StorageSelectorWindow();

    std::string getStorageType();

private:
    QVBoxLayout *layout;
    QComboBox *combo_box;
    std::string storage_type;

private slots:
    void closeWindow();
};

#endif