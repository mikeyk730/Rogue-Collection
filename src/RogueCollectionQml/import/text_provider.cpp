#include <QImage>
#include "tile_provider.h"
#include "dos_to_unicode.h"
#include "utility.h"
#include "colors.h"

TextProvider::TextProvider(const TextConfig & config) :
    config_(config)
{
    QPixmap img(GetResourcePath(config.imagefile).c_str());

    tile_size_.setWidth(img.size().width() / config.layout.x);
    tile_size_.setHeight(img.size().height() / config.layout.y);

    mask_ = img.createMaskFromColor(QColor(255, 255, 255), Qt::MaskOutColor);
}

QSize TextProvider::TileSize() const
{
    return tile_size_;
}

void TextProvider::PaintTile(QPainter *painter, QRect dest_rect, int ch, int color)
{
    QRect source_rect = GetTextRect(ch);

    painter->fillRect(dest_rect, Colors::GetBg(color));
    painter->setPen(Colors::GetFg(color));
    painter->drawPixmap(dest_rect, mask_, source_rect);
}

QRect TextProvider::GetTextRect(unsigned int ch)
{
    Coord layout = config_.layout;
    QRect r;
    r.setX((ch % layout.x) * tile_size_.width());
    r.setY((ch / layout.x) * tile_size_.height());
    r.setWidth(tile_size_.width());
    r.setHeight(tile_size_.height());
    return r;
}
