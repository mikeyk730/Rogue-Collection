#include "tile_provider.h"
#include "colors.h"

ITileProvider::~ITileProvider() {}

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
    if (ch == ' ')
        return;

    painter->setPen(Colors::GetFg(color));
    //font_.setPixelSize(r.height());
    painter->setFont(font_);
    painter->setRenderHint(QPainter::TextAntialiasing, false);
    painter->drawText(r, 0, QChar(ch));
}

QFont FontProvider::Font() const
{
    return font_;
}
