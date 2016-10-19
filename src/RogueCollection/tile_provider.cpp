#include <SDL.h>
#include <pc_gfx_charmap.h>
#include "tile_provider.h"
#include "utility.h"

TileProvider::TileProvider(const TileConfig & config, SDL_Renderer * renderer)
    : m_cfg(config)
{
    m_index = {
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

    SDL::Scoped::Surface tiles(load_bmp(getResourcePath("") + config.filename));
    m_tile_dimensions.x = tiles->w / config.count;
    m_tile_dimensions.y = tiles->h / config.states;
    m_tiles = create_texture(tiles.get(), renderer).release();
}

TileProvider::~TileProvider()
{
    SDL_DestroyTexture(m_tiles);
}

Coord TileProvider::dimensions() const
{
    return m_tile_dimensions;
}

int TileProvider::tile_index(unsigned char c, unsigned short attr)
{
    if (c >= 'A' && c <= 'Z')
        return c - 'A';

    auto i = m_index.find(c);
    if (i == m_index.end())
        return -1;

    int index = i->second;
    if (index >= 65 && index <= 68) //different color bolts use different tiles
    {
        if (attr & 0x02) //yellow
            index += 8;
        else if (attr & 0x04) //red
            index += 4;
    }
    return index;
}

bool TileProvider::use_inverse(unsigned short attr)
{
    return attr > 100 && attr != 160;
}

SDL_Rect TileProvider::get_tile_rect(int i, bool use_inverse)
{
    SDL_Rect r;
    r.h = m_tile_dimensions.y;
    r.w = m_tile_dimensions.x;
    r.x = i*m_tile_dimensions.x;
    r.y = use_inverse ? m_tile_dimensions.y : 0;
    return r;
}

bool TileProvider::GetTexture(int ch, int color, SDL_Texture** texture, SDL_Rect* rect)
{
    auto i = tile_index(ch, color);
    if (i == -1) {
        return false;
    }

    bool inv = (m_cfg.states > 1 && use_inverse(color));
    *rect = get_tile_rect(i, inv);
    *texture = m_tiles;

    return true;
}
