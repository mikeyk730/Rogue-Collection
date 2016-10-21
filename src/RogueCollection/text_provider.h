#pragma once
#include <vector>
#include <map>
#include <coord.h>
#include "utility.h"

struct SDL_Renderer;
struct SDL_Texture;
struct SDL_Surface;
struct SDL_Rect;

struct TextConfig
{
    std::string filename;
    Coord layout;
    std::vector<int> colors;
    bool generate_colors;
    bool monochrome;
};

struct ITextProvider
{
    virtual ~ITextProvider();
    virtual Coord dimensions() const = 0;
    virtual void GetTexture(int ch, int color, SDL_Texture** texture, SDL_Rect* rect) = 0;
};

struct TextProvider : ITextProvider
{
    TextProvider(const TextConfig& config, SDL_Renderer* renderer);
    ~TextProvider();
    Coord dimensions() const override;
    void GetTexture(int ch, int color, SDL_Texture** texture, SDL_Rect* rect) override;

private:
    int get_text_index(unsigned short attr);
    SDL_Rect get_text_rect(unsigned char ch, int i);

    TextConfig m_cfg;
    SDL_Texture* m_text = 0;
    Coord m_text_dimensions = { 0, 0 };
    std::map<int, int> m_attr_index;
};


struct TextGenerator : ITextProvider
{
    TextGenerator(const TextConfig& config, SDL_Renderer* renderer);
    ~TextGenerator();
    Coord dimensions() const override;
    void GetTexture(int ch, int color, SDL_Texture** texture, SDL_Rect* rect) override;

private:
    SDL_Rect get_text_rect(unsigned char ch);

    TextConfig m_cfg;
    SDL_Renderer* m_renderer;
    SDL::Scoped::Surface m_text = 0;
    Coord m_text_dimensions = { 0, 0 };
    std::map<int, SDL_Texture*> m_textures;
    std::vector<SDL_Color> m_colors;
};
