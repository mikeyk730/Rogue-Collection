#pragma once
#include <vector>
#include <map>
#include <coord.h>

struct SDL_Renderer;
struct SDL_Texture;
struct SDL_Rect;

struct TextConfig
{
    std::string filename;
    Coord layout;
    std::vector<int> colors;
};

struct TextProvider
{
    TextProvider(const TextConfig& config, SDL_Renderer* renderer);
    ~TextProvider();
    Coord dimensions() const;
    void GetTexture(int ch, int color, SDL_Texture** texture, SDL_Rect* rect);

private:
    int TextProvider::get_text_index(unsigned short attr);
    SDL_Rect TextProvider::get_text_rect(unsigned char ch, int i);

    TextConfig m_cfg;
    SDL_Texture* m_text = 0;
    Coord m_text_dimensions = { 0, 0 };
    std::map<int, int> m_attr_index;
};

