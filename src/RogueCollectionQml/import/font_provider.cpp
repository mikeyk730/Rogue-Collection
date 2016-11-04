#include "tile_provider.h"
#include "colors.h"

ITileProvider::~ITileProvider() {}

namespace Colors
{
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

FontProvider::FontProvider(const QFont& font)
{
    font_ = font;
    font_.setStyleStrategy(QFont::NoAntialias);

    QFontMetrics font_metrics(font);
    font_size_.setWidth(font_metrics.width("W"));
    font_size_.setHeight(font_metrics.height());
}

QSize FontProvider::TileSize() const
{
    return font_size_;
}

void FontProvider::PaintTile(QPainter *painter, QRect r, int ch, int color)
{
    painter->fillRect(r, Colors::GetBg(color));
    painter->setPen(Colors::GetFg(color));
    painter->setFont(font_);
    painter->setRenderHint(QPainter::TextAntialiasing, false);
    painter->drawText(r, 0, QChar(ch));
}

QFont FontProvider::Font() const
{
    return font_;
}
