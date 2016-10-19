#include <SDL.h>
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

