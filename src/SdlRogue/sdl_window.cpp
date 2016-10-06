#include <cassert>
#include <map>
#include <Windows.h> //todo: change interface to avoid char_info and small_rect
#include "SDL.h"
#include "sdl_window.h"
#include "utility.h"
#include "res_path.h"
#include "RogueCore/rogue.h"

struct SdlWindow::Impl
{
    const int TILE_COUNT = 78;
    const int TILE_STATES = 2;

    const int TEXT_COUNT = 96;
    const int TEXT_STATES = 16;

    Impl()
    {
        m_window = SDL_CreateWindow("Rogue", 100, 100, 8*80, 16*25, SDL_WINDOW_SHOWN);
        if (m_window == nullptr)
            throw_error("SDL_CreateWindow");

        m_renderer = SDL_CreateRenderer(m_window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
        if (m_renderer == nullptr)
            throw_error("SDL_CreateRenderer");

        SDL::Scoped::Surface tiles(load_bmp(getResourcePath("") + "tiles.bmp"));
        m_tile_height = tiles->h / TILE_STATES;
        m_tile_width = tiles->w / TILE_COUNT;
        m_tiles = create_texture(tiles.get(), m_renderer).release();

        SDL::Scoped::Surface text(load_bmp(getResourcePath("") + "text.bmp"));
        assert(m_tile_height == text->h / TEXT_STATES);
        assert(m_tile_width == text->w / TEXT_COUNT);
        m_text = create_texture(text.get(), m_renderer).release();
    }

    int tile_index(unsigned char c)
    {
        auto i = m_index.find(c);
        if (i != m_index.end())
            return i->second;
        return -1;
    }

    bool use_inverse(unsigned short attr)
    {
        return attr > 100 && attr != 160;
    }

    SDL_Rect get_tile_rect(int i, bool use_inverse)
    {
        SDL_Rect r;
        r.h = m_tile_height;
        r.w = m_tile_width;
        r.x = i*m_tile_width;
        r.y = use_inverse ? m_tile_height : 0;
        return r;
    }

    Coord get_screen_pos(Coord buffer_pos)
    {
        Coord p;
        p.x = buffer_pos.x * m_tile_width;
        p.y = buffer_pos.y * m_tile_height;
        return p;
    }

    int get_text_index(unsigned short attr)
    {
        auto i = m_attr_index.find(attr);
        if (i != m_attr_index.end())
            return i->second;
        return 0;
    }

    SDL_Rect get_text_rect(int c, int i)
    {
        c -= 0x20;
        SDL_Rect r;
        r.h = m_tile_height;
        r.w = m_tile_width;
        r.x = c*m_tile_width;
        r.y = i*m_tile_height;
        return r;
    }

    void Draw(_CHAR_INFO * info, Coord dimensions, _SMALL_RECT rect)
    {
        //SDL_RenderClear(m_renderer);
        for (int x = rect.Left; x <= rect.Right; ++x) {
            for (int y = rect.Top; y <= rect.Bottom; ++y) {
                auto p = get_screen_pos({ x, y });

                auto c = info[y*dimensions.x + x];
                if (c.Char.AsciiChar >= 0x20 && c.Char.AsciiChar < 128)
                {
                    int i = get_text_index(c.Attributes);
                    auto r = get_text_rect(c.Char.AsciiChar, i);
                    render_texture_at(m_text, m_renderer, p, r);
                }
                else {
                    auto i = tile_index(c.Char.AsciiChar);
                    if (i != -1) {
                        bool inv = use_inverse(c.Attributes);
                        auto r = get_tile_rect(i, inv);
                        render_texture_at(m_tiles, m_renderer, p, r);
                    }
                }
            }
        }
        SDL_RenderPresent(m_renderer);
    }


    ~Impl()
    {
        SDL_DestroyRenderer(m_renderer);
        SDL_DestroyWindow(m_window);
    }

    SDL_Window* m_window = 0;
    SDL_Renderer* m_renderer = 0;
    SDL_Texture* m_tiles = 0;
    SDL_Texture* m_text = 0;

    int m_tile_height = 0;
    int m_tile_width = 0;
    std::map<int, int> m_attr_index = {
        {  7,  0 },
        {  2,  1 },
        {  3,  2 },
        {  4,  3 },
        {  5,  4 },
        {  6,  5 },
        {  8,  6 },
        {  9,  7 },
        { 10,  8 },
        { 12,  9 },
        { 13, 10 },
        { 14, 11 },
        { 15, 12 },
        {  1, 13 },
        {112, 14 },
        { 15, 15 },
    };

    std::map<int, int> m_index = {
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
    };
};

SdlWindow::SdlWindow() :
    m_impl(new Impl())
{
}

SdlWindow::~SdlWindow()
{
}

void SdlWindow::Draw(_CHAR_INFO * info, Coord dimensions)
{
    SMALL_RECT r;
    r.Top = 0;
    r.Left = 0;
    r.Bottom = dimensions.y;
    r.Right = dimensions.x;
    m_impl->Draw(info, dimensions, r);
}

void SdlWindow::Draw(_CHAR_INFO * info, Coord dimensions, _SMALL_RECT r)
{
    m_impl->Draw(info, dimensions, r);
}

void SdlWindow::MoveCursor(Coord pos)
{
}

void SdlWindow::SetCursor(bool enable)
{
}
