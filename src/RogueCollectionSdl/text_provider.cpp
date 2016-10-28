#include <SDL.h>
#include <SDL_ttf.h>
#include <cassert>
#include "text_provider.h"
#include "sdl_rogue.h"
#include "sdl_utility.h"
#include "dos_to_unicode.h"

TextProvider::TextProvider(const TextConfig & config, SDL_Renderer * renderer)
    : m_cfg(config)
{
    SDL::Scoped::Texture text(LoadImage(GetResourcePath("") + config.imagefile, renderer));
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

Coord TextProvider::Dimensions() const
{
    return m_text_dimensions;
}

int TextProvider::GetTextIndex(unsigned short attr)
{
    auto i = m_attr_index.find(attr);
    if (i != m_attr_index.end())
        return i->second;
    return 0;
}

SDL_Rect TextProvider::GetTextRect(unsigned char ch, int i)
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
    int i = GetTextIndex(color);
    *rect = GetTextRect(ch, i);
    *texture = m_text;
}

TextGenerator::TextGenerator(const TextConfig & config, SDL_Renderer * renderer) : 
    m_cfg(config),
    m_renderer(renderer),
    m_text(LoadBmp(GetResourcePath("") + config.imagefile))
{
    assert(config.colors.size() == 1);
    Init();
}

std::string all_chars()
{
    std::string s;
    s.push_back(' ');
    for (size_t i = 1; i < 256; ++i)
        s.push_back((unsigned char)i);
    return s;
}

TextGenerator::TextGenerator(const FontConfig & config, SDL_Renderer * renderer) :
    m_cfg(),
    m_renderer(renderer),
    m_text(0, SDL_FreeSurface)
{
    SDL::Scoped::Font font(LoadFont(config.fontfile, config.size));
    TTF_SetFontKerning(font.get(), 0);

    std::string s(all_chars());
    uint16_string u16s(DosToUnicode(s));

    auto text = TTF_RenderUNICODE_Solid(font.get(), u16s.c_str(), SDL::Colors::grey());
    m_text = SDL::Scoped::Surface(text, SDL_FreeSurface);

    m_cfg.layout.x = (int)s.size();
    m_cfg.layout.y = 1;
    
    Init();

    //mdk: hack.  I don't know why we sometimes get a height that's greater than the requested font size
    m_text_dimensions.y = config.size;
}

TextGenerator::~TextGenerator()
{
    for (auto i = m_textures.begin(); i != m_textures.end(); ++i)
        SDL_DestroyTexture(i->second);
}

void TextGenerator::Init()
{
    m_text_dimensions.x = m_text->w / m_cfg.layout.x;
    m_text_dimensions.y = m_text->h / m_cfg.layout.y;

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

Coord TextGenerator::Dimensions() const
{
    return m_text_dimensions;
}

void TextGenerator::GetTexture(int ch, int color, SDL_Texture ** texture, SDL_Rect * rect)
{
    *rect = GetTextRect(ch);
    
    auto i = m_textures.find(color);
    if (i != m_textures.end()) {
        *texture = i->second;
        return;
    }
    auto fg = m_colors[color & 0xf];
    auto bg = m_colors[(color >> 4) & 0xf];
    auto t = PaintedTexture(m_text.get(), 0, fg, bg, m_renderer);
    *texture = t.release();
    m_textures[color] = *texture;
}

SDL_Rect TextGenerator::GetTextRect(unsigned char ch)
{
    Coord layout = m_cfg.layout;
    SDL_Rect r;
    r.h = m_text_dimensions.y;
    r.w = m_text_dimensions.x;
    r.x = (ch % layout.x) * m_text_dimensions.x;
    r.y = (ch / layout.x) * m_text_dimensions.y;
    return r;
}

ITextProvider::~ITextProvider()
{
}

std::unique_ptr<ITextProvider> CreateTextProvider(FontConfig* font_cfg, TextConfig* text_cfg, SDL_Renderer* renderer)
{
    std::unique_ptr<ITextProvider> p;

    if (font_cfg && !font_cfg->fontfile.empty()) {
        p.reset(new TextGenerator(*font_cfg, renderer));
    }
    else if (text_cfg->generate_colors) {
        p.reset(new TextGenerator(*text_cfg, renderer));
    }
    else {
        p.reset(new TextProvider(*text_cfg, renderer));
    }

    return p;
}

