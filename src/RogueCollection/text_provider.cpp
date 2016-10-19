#include <SDL.h>
#include <cassert>
#include "text_provider.h"
#include "utility.h"

TextProvider::TextProvider(const TextConfig & config, SDL_Renderer * renderer)
    : m_cfg(config)
{
    SDL::Scoped::Texture text(loadImage(getResourcePath("") + config.filename, renderer));
    m_text = text.release();
    int textw, texth;
    SDL_QueryTexture(m_text, NULL, NULL, &textw, &texth);

    m_text_dimensions.x = textw / config.layout.x;
    m_text_dimensions.y = texth / (int)config.colors.size() / config.layout.y;
    for (int i = 0; i < (int)config.colors.size(); ++i)
        m_attr_index[config.colors[i]] = i;
}

TextProvider::~TextProvider()
{
    SDL_DestroyTexture(m_text);
}

Coord TextProvider::dimensions() const
{
    return m_text_dimensions;
}

int TextProvider::get_text_index(unsigned short attr)
{
    auto i = m_attr_index.find(attr);
    if (i != m_attr_index.end())
        return i->second;
    return 0;
}

SDL_Rect TextProvider::get_text_rect(unsigned char ch, int i)
{
    Coord layout = m_cfg.layout;
    SDL_Rect r;
    r.h = m_text_dimensions.y;
    r.w = m_text_dimensions.x;
    r.x = (ch % layout.x) * m_text_dimensions.x;
    r.y = (i*layout.y + ch / layout.x) * m_text_dimensions.y;
    return r;
}

void TextProvider::GetTexture(int ch, int color, SDL_Texture ** texture, SDL_Rect * rect)
{
    int i = get_text_index(color);
    *rect = get_text_rect(ch, i);
    *texture = m_text;
}

TextGenerator::TextGenerator(const TextConfig & config, SDL_Renderer * renderer) : 
    m_cfg(config),
    m_renderer(renderer),
    m_text(load_bmp(getResourcePath("") + config.filename))
{
    assert(config.colors.size() == 1);
    m_text_dimensions.x = m_text->w / config.layout.x;
    m_text_dimensions.y = m_text->h / config.layout.y;

    m_colors = {
        SDL::Colors::black(),
        SDL::Colors::blue(),
        SDL::Colors::green(),
        SDL::Colors::cyan(),
        SDL::Colors::red(),
        SDL::Colors::magenta(),
        SDL::Colors::brown(),
        SDL::Colors::grey(),
        SDL::Colors::d_grey(),
        SDL::Colors::l_blue(),
        SDL::Colors::l_green(),
        SDL::Colors::l_cyan(),
        SDL::Colors::l_red(),
        SDL::Colors::l_magenta(),
        SDL::Colors::yellow(),
        SDL::Colors::white()
    };
}

TextGenerator::~TextGenerator()
{
    for (auto i = m_textures.begin(); i != m_textures.end(); ++i)
        SDL_DestroyTexture(i->second);
}

Coord TextGenerator::dimensions() const
{
    return m_text_dimensions;
}

void TextGenerator::GetTexture(int ch, int color, SDL_Texture ** texture, SDL_Rect * rect)
{
    *rect = get_text_rect(ch);
    
    auto i = m_textures.find(color);
    if (i != m_textures.end()) {
        *texture = i->second;
        return;
    }
    auto fg = m_colors[color & 0xf];
    auto bg = m_colors[(color >> 4) & 0xf];
    auto t = painted_texture(m_text.get(), 0, fg, bg, m_renderer);
    *texture = t.release();
    m_textures[color] = *texture;
}

SDL_Rect TextGenerator::get_text_rect(unsigned char ch)
{
    Coord layout = m_cfg.layout;
    SDL_Rect r;
    r.h = m_text_dimensions.y;
    r.w = m_text_dimensions.x;
    r.x = (ch % layout.x) * m_text_dimensions.x;
    r.y = (ch / layout.x) * m_text_dimensions.y;
    return r;
}
