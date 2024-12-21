#ifndef RECOMMENDED_DEPENDS_WINDOW_HPP
#define RECOMMENDED_DEPENDS_WINDOW_HPP

#include <QDialog>
#include <QLabel>

class RecommendedDependsWindow : QDialog
{
    public:
    RecommendedDependsWindow(const std::vector<std::string> &recommended_depends, QWidget *parent = nullptr);
    ~RecommendedDependsWindow();
};

#endif