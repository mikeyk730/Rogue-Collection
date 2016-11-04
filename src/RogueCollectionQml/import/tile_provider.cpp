#include <map>
#include <QPainter>
#include <QCoreApplication>
#include <pc_gfx_charmap.h>
#include "tile_provider.h"
#include "utility.h"

std::map<int, int> s_tile_index = {
    { PLAYER, 26 },
    { ULWALL, 27 },
    { URWALL, 28 },
    { LLWALL, 29 },
    { LRWALL, 30 },
    { HWALL,  31 },
    { VWALL,  32 },
    { FLOOR,  33 },
    { PASSAGE,34 },
    { DOOR,   35 },
    { STAIRS, 36 },
    { TRAP,   37 },
    { AMULET, 38 },
    { FOOD,   39 },
    { GOLD,   40 },
    { POTION, 41 },
    { RING,   42 },
    { SCROLL, 43 },
    { STICK,  44 },
    { WEAPON, 45 },
    { ARMOR,  55 },
    { MAGIC,  63 },
    { BMAGIC, 64 },
    { '\\',   65 },
    { '/',    66 },
    { '-',    67 },
    { '|',    68 },
    { '*',    77 },
};

TileProvider::TileProvider(const TileConfig & config)
    : config_(config),
      tilemap_(GetResourcePath(config.filename).c_str())
{
    tile_size_.setWidth(tilemap_.size().width() / config_.count);
    tile_size_.setHeight(tilemap_.size().height() / config_.states);
}

QSize TileProvider::TileSize() const
{
    return tile_size_;
}

void TileProvider::PaintTile(QPainter *painter, QRect dest_rect, int ch, int color)
{
    auto i = TitleIndex(ch, color);
    if (i == -1) {
        painter->fillRect(dest_rect, QColor("black"));
        return;
    }

    bool use_inverse = (config_.states > 1 && UseInverse(color));
    QRect source_rect = GetTileRect(i, use_inverse);
    painter->drawImage(dest_rect, tilemap_, source_rect);
}

bool TileProvider::UseInverse(unsigned int color)
{
    return (color >> 4) == 0x07;
}

int TileProvider::TitleIndex(unsigned int ch, unsigned int color)
{
    if (ch >= 'A' && ch <= 'Z')
        return ch - 'A';

    auto i = s_tile_index.find(ch);
    if (i == s_tile_index.end())
        return -1;

    int index = i->second;
    if (index >= 65 && index <= 68) //different color bolts use different tiles
    {
        if (color & 0x02) //yellow
            index += 8;
        else if (color & 0x04) //red
            index += 4;
    }
    return index;
}

QRect TileProvider::GetTileRect(int i, bool use_inverse)
{
    QRect r;
    r.setX(i*tile_size_.width());
    r.setY(use_inverse ? tile_size_.height() : 0);
    r.setHeight(tile_size_.height());
    r.setWidth(tile_size_.width());
    return r;
}

