#include "colors.h"

namespace Colors
{
    QColor black()    { return QColor(   0,   0,   0, 255 ); }
    QColor white()    { return QColor( 255, 255, 255, 255 ); }
    QColor grey()     { return QColor( 170, 170, 170, 255 ); }
    QColor d_grey()   { return QColor(  65,  65,  65, 255 ); }
    QColor l_grey()   { return QColor( 205, 205, 205, 255 ); }
    QColor red()      { return QColor( 170,   0,   0, 255 ); }
    QColor l_red()    { return QColor( 255,  85,  85, 255 ); }
    QColor green()    { return QColor(   0, 170,   0, 255 ); }
    QColor l_green()  { return QColor(  85, 255,  85, 255 ); }
    QColor blue()     { return QColor(   0,   0, 170, 255 ); }
    QColor l_blue()   { return QColor(  85,  85, 255, 255 ); }
    QColor cyan()     { return QColor(   0, 170, 170, 255 ); }
    QColor l_cyan()   { return QColor(  25, 255, 255, 255 ); }
    QColor magenta()  { return QColor( 170,   0, 170, 255 ); }
    QColor l_magenta(){ return QColor( 255,  25, 255, 255 ); }
    QColor yellow()   { return QColor( 255, 255,  25, 255 ); }
    QColor brown()    { return QColor( 170,  85,   0, 255 ); }
    QColor orange()   { return QColor( 234, 118,   2, 255 ); }

    QColor colors[] = {
        Colors::black(),
        Colors::blue(),
        Colors::green(),
        Colors::cyan(),
        Colors::red(),
        Colors::magenta(),
        Colors::brown(),
        Colors::grey(),
        Colors::d_grey(),
        Colors::l_blue(),
        Colors::l_green(),
        Colors::l_cyan(),
        Colors::l_red(),
        Colors::l_magenta(),
        Colors::yellow(),
        Colors::white()
    };

    QColor GetTileColor(int color)
    {
        return colors[color];
    }

    QColor GetFg(int color)
    {
        return GetTileColor(color & 0x0f);
    }

    QColor GetBg(int color)
    {
        return GetTileColor(color >> 4);
    }
}
