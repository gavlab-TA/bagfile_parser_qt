#include <QApplication>
#include "bagfile_parser_qt/main_window.hpp"

int main(int argc, char **argv)
{
    QApplication app(argc, argv);

    MainWindow main_window(500, 500);
    app.exec();
}