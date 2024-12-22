#include "bagfile_parser_qt/recommended_depends_window.hpp"

RecommendedDependsWindow::RecommendedDependsWindow(const std::vector<std::string> &recommended_depends, const std::string &output_path, QWidget *parent) : QDialog(parent)
{
    this->setWindowTitle("Recommended Dependencies");
    this->setWindowModality(Qt::ApplicationModal);
    this->output_path = output_path;
    
    QVBoxLayout *layout = new QVBoxLayout();
    this->list_widget = new QListWidget();
    QStringList items;

    for (size_t i = 0; i < recommended_depends.size(); ++i)
    {
        items.push_back(QString(recommended_depends.at(i).c_str()));
    }

    for (const QString &item_text : items)
    {
        QListWidgetItem *item = new QListWidgetItem(item_text);
        item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
        item->setCheckState(Qt::Checked);
        this->list_widget->addItem(item);
    }

    layout->addWidget(list_widget);
    this->approve_button = new QPushButton();
    this->approve_button->setText("Approved Recommendations");
    
    QObject::connect(this->approve_button, &QPushButton::released, this, &RecommendedDependsWindow::approveDependencies);
    layout->addWidget(approve_button);
    this->setLayout(layout);
}

RecommendedDependsWindow::~RecommendedDependsWindow()
{

}

void RecommendedDependsWindow::approveDependencies()
{
    std::vector<std::string> depends;

    std::ofstream file;
    std::string depends_filename = output_path + "/files/depends.txt";
    file.open(depends_filename, std::ios_base::app);

    for (int i = 0; i < list_widget->count(); ++i)
    {
        if (list_widget->item(i)->checkState() == Qt::Checked)
        {
            std::string line = list_widget->item(i)->text().toStdString() + "\n";
            file << line;
        }
    }

    file.close();
    this->close();
}