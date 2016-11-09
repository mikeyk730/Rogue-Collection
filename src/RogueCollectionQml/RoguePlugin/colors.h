#ifndef COLORS_H
#define COLORS_H

#include <QColor>

namespace Colors
{
    QColor black();
    QColor white();
    QColor grey();
    QColor d_grey();
    QColor l_grey();
    QColor red();
    QColor l_red();
    QColor green();
    QColor l_green();
    QColor blue();
    QColor l_blue();
    QColor cyan();
    QColor l_cyan();
    QColor magenta();
    QColor l_magenta();
    QColor yellow();
    QColor brown();
    QColor orange();

    QColor GetTileColor(int color);
    QColor GetFg(int color);
    QColor GetBg(int color);
}

#endif // COLORS_H
