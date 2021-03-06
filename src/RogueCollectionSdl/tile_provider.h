#pragma once
#include <string>
#include <map>
#include <coord.h>
#include <SDL.h>
#include "game_config.h"

struct TileProvider
{
    TileProvider(const TileConfig& config, SDL_Renderer* renderer);
    ~TileProvider();
    Coord Dimensions() const;
    bool GetTexture(int ch, int color, SDL_Texture** texture, SDL_Rect* rect);

private:
    int TitleIndex(unsigned char c, unsigned short attr);
    bool UseInverse(unsigned short attr);
    SDL_Rect GetTileRect(int i, bool use_inverse);

    TileConfig m_cfg;
    SDL_Texture* m_tiles = 0;
    Coord m_tile_dimensions = { 0, 0 };
    std::map<int, int> m_index;
};

