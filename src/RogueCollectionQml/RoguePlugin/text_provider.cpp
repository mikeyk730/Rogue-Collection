#include <QImage>
#include "tile_provider.h"
#include "dos_to_unicode.h"
#include "utility_qml.h"
#include "colors.h"

TextProvider::TextProvider(const TextConfig & config) :
    config_(config)
{
    QImage img(GetResourcePath(config.imagefile).c_str());

    image_size_ = img.size();

    tile_size_.setWidth(image_size_.width() / config.layout.x);
    tile_size_.setHeight(image_size_.height() / config.layout.y);

    mask_ = QBitmap::fromImage(img.createMaskFromColor(qRgb(255, 255, 255), Qt::MaskOutColor));
}

TextProvider::~TextProvider() = default;

QSize TextProvider::TileSize() const
{
    return tile_size_;
}

void TextProvider::PaintTile(QPainter *painter, QRect dest_rect, int ch, int color)
{
    QRect source_rect = GetTextRect(ch);

    if (ch == ' ')
        painter->fillRect(dest_rect, Colors::GetBg(color));
    else
        painter->drawPixmap(dest_rect, *GetPixMap(color), source_rect);
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

QPixmap *TextProvider::GetPixMap(int color)
{
    auto i = images_.find(color);
    if (i == images_.end()) {
        std::unique_ptr<QPixmap> p(new QPixmap(image_size_));

        QPainter painter(p.get());
        painter.fillRect(p->rect(), Colors::GetBg(color));
        painter.setPen(Colors::GetFg(color));
        painter.drawPixmap(0, 0, mask_);
        painter.end();

        i = images_.insert(std::make_pair(color,std::move(p))).first;
    }
    return i->second.get();
}
