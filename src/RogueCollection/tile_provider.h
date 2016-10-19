#pragma once
#include <string>
#include <map>
#include <coord.h>

struct SDL_Renderer;
struct SDL_Texture;
struct SDL_Rect;

struct TileConfig
{
    std::string filename;
    int count;
    int states;
};

struct TileProvider
{
    TileProvider(const TileConfig& config, SDL_Renderer* renderer);
    ~TileProvider();
    Coord dimensions() const;
    bool GetTexture(int ch, int color, SDL_Texture** texture, SDL_Rect* rect);

private:
    int tile_index(unsigned char c, unsigned short attr);
    bool use_inverse(unsigned short attr);
    SDL_Rect get_tile_rect(int i, bool use_inverse);

    TileConfig m_cfg;
    SDL_Texture* m_tiles = 0;
    Coord m_tile_dimensions = { 0, 0 };
    std::map<int, int> m_index;
};

