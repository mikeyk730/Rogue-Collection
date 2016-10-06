#include <cassert>
#include <map>
#include <mutex>
#include <Windows.h> //todo: change interface to avoid char_info and small_rect
#include "SDL.h"
#include "sdl_window.h"
#include "utility.h"
#include "res_path.h"
#include "RogueCore/rogue.h"
#include "RogueCore/coord.h"

struct SdlWindow::Impl
{
    const int TILE_COUNT = 78;
    const int TILE_STATES = 2;

    const int TEXT_COUNT = 256;
    const int TEXT_STATES = 16;

    Impl();
    ~Impl();

    void SetDimensions(Coord dimensions);
    void Draw(_CHAR_INFO * info);
    void Draw(_CHAR_INFO * info, _SMALL_RECT rect);
    void Run();
    void Quit();

private:
    Coord get_screen_pos(Coord buffer_pos);

    int tile_index(unsigned char c);
    bool use_inverse(unsigned short attr);
    SDL_Rect get_tile_rect(int i, bool use_inverse);

    int get_text_index(unsigned short attr);
    SDL_Rect get_text_rect(int c, int i);

    SMALL_RECT full_rect() const;
    int data_size() const;

private:
    SDL_Window* m_window = 0;
    SDL_Renderer* m_renderer = 0;
    SDL_Texture* m_tiles = 0;
    SDL_Texture* m_text = 0;

    int m_tile_height = 0;
    int m_tile_width = 0;

    struct ThreadData
    {
        CHAR_INFO* m_data;
    };
    ThreadData m_shared_data;
    std::mutex m_mutex;
    Coord m_dimensions;

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

SdlWindow::Impl::Impl()
{
    m_dimensions = { 80, 25 };
    SDL::Scoped::Surface tiles(load_bmp(getResourcePath("") + "tiles.bmp"));
    //SDL::Scoped::Surface tiles(load_bmp(getResourcePath("") + "sprites.bmp"));
    m_tile_height = tiles->h / TILE_STATES;
    m_tile_width = tiles->w / TILE_COUNT;

    SDL::Scoped::Surface text(load_bmp(getResourcePath("") + "text.bmp"));
    assert(m_tile_height == text->h / TEXT_STATES);
    assert(m_tile_width == text->w / TEXT_COUNT);

    m_window = SDL_CreateWindow("Rogue", 100, 100, m_tile_width * m_dimensions.x, m_tile_height * m_dimensions.y, SDL_WINDOW_SHOWN);
    if (m_window == nullptr)
        throw_error("SDL_CreateWindow");

    m_renderer = SDL_CreateRenderer(m_window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (m_renderer == nullptr)
        throw_error("SDL_CreateRenderer");

    m_tiles = create_texture(tiles.get(), m_renderer).release();
    m_text = create_texture(text.get(), m_renderer).release();
}

SdlWindow::Impl::~Impl()
{
    SDL_DestroyTexture(m_text);
    SDL_DestroyTexture(m_tiles);
    SDL_DestroyRenderer(m_renderer);
    SDL_DestroyWindow(m_window);
}

int SdlWindow::Impl::tile_index(unsigned char c)
{
    if (c >= 'A' && c <= 'Z')
        return c - 'A';
    auto i = m_index.find(c);
    if (i != m_index.end())
        return i->second;
    return -1;
}

bool SdlWindow::Impl::use_inverse(unsigned short attr)
{
    return attr > 100 && attr != 160;
}

inline SDL_Rect SdlWindow::Impl::get_tile_rect(int i, bool use_inverse)
{
    SDL_Rect r;
    r.h = m_tile_height;
    r.w = m_tile_width;
    r.x = i*m_tile_width;
    r.y = use_inverse ? m_tile_height : 0;
    return r;
}

inline Coord SdlWindow::Impl::get_screen_pos(Coord buffer_pos)
{
    Coord p;
    p.x = buffer_pos.x * m_tile_width;
    p.y = buffer_pos.y * m_tile_height;
    return p;
}

inline int SdlWindow::Impl::get_text_index(unsigned short attr)
{
    auto i = m_attr_index.find(attr);
    if (i != m_attr_index.end())
        return i->second;
    return 0;
}

inline SDL_Rect SdlWindow::Impl::get_text_rect(int c, int i)
{
    SDL_Rect r;
    r.h = m_tile_height;
    r.w = m_tile_width;
    r.x = c*m_tile_width;
    r.y = i*m_tile_height;
    return r;
}

void SdlWindow::Impl::SetDimensions(Coord dimensions)
{
    m_dimensions = dimensions;
    std::lock_guard<std::mutex> lock(m_mutex);
    m_shared_data.m_data = new CHAR_INFO[m_dimensions.x*m_dimensions.y];
}

void SdlWindow::Impl::Draw(_CHAR_INFO * info)
{
    Draw(info, full_rect());
}

inline void SdlWindow::Impl::Draw(_CHAR_INFO * info, _SMALL_RECT rect)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    memcpy(m_shared_data.m_data, info, data_size());
}

void SdlWindow::Impl::Run()
{
    SDL_Event e;
    bool quit = false;
    while (!quit) {
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) {
                quit = true;
            }
            else if (e.type == SDL_TEXTINPUT) {
                std::cout << "ch:" << e.text.text << std::endl;
                //key = e.text.text[0];
            }
            else if (e.type == SDL_KEYUP) {
                //handle esc, backspace, directions, numpad, function keys, return, ctrl-modified, ins, del
                //how to read scroll lock, etc?
            }
        }

        CHAR_INFO* temp = new CHAR_INFO[m_dimensions.x*m_dimensions.y];
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            memcpy(temp, m_shared_data.m_data, data_size());
        }
        std::unique_ptr<CHAR_INFO[]> data(temp);

        auto rect = full_rect();

        SDL_RenderClear(m_renderer);
        for (int x = rect.Left; x <= rect.Right; ++x) {
            for (int y = rect.Top; y <= rect.Bottom; ++y) {
                auto p = get_screen_pos({ x, y });
                auto c = data[y*m_dimensions.x + x];

                //todo: how to determine text or monster?
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
}

void SdlWindow::Impl::Quit()
{
    SDL_Event sdlevent;
    sdlevent.type = SDL_QUIT;
    SDL_PushEvent(&sdlevent);
}

SMALL_RECT SdlWindow::Impl::full_rect() const
{
    SMALL_RECT r;
    r.Left = 0;
    r.Top = 0;
    r.Right = m_dimensions.x - 1;
    r.Bottom = m_dimensions.y - 1;

    return r;
}

int SdlWindow::Impl::data_size() const
{
    return sizeof(CHAR_INFO) * m_dimensions.x * m_dimensions.y;
}






SdlWindow::SdlWindow() :
    m_impl(new Impl())
{
}

SdlWindow::~SdlWindow()
{
}

void SdlWindow::Run()
{
    m_impl->Run();
}

void SdlWindow::Quit()
{
    m_impl->Quit();
}

void SdlWindow::SetDimensions(Coord dimensions)
{
    m_impl->SetDimensions(dimensions);
}

void SdlWindow::Draw(_CHAR_INFO * info)
{
    m_impl->Draw(info);
}

void SdlWindow::Draw(_CHAR_INFO * info, _SMALL_RECT r)
{
    m_impl->Draw(info, r);
}

void SdlWindow::MoveCursor(Coord pos)
{
}

void SdlWindow::SetCursor(bool enable)
{
}
