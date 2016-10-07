#include <cassert>
#include <map>
#include <vector>
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
    const Coord EMPTY_COORD = { 0, 0 };

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
    void RenderRegion(CHAR_INFO* data, Coord dimensions, SMALL_RECT rect);

    Coord get_screen_pos(Coord buffer_pos);

    int tile_index(unsigned char c);
    bool use_inverse(unsigned short attr);
    SDL_Rect get_tile_rect(int i, bool use_inverse);

    int get_text_index(unsigned short attr);
    SDL_Rect get_text_rect(int c, int i);

    int shared_data_size() const;       //must have mutex before calling
    SMALL_RECT shared_data_full_rect(); //must have mutex before calling

private:
    SDL_Window* m_window = 0;
    SDL_Renderer* m_renderer = 0;
    SDL_Texture* m_tiles = 0;
    SDL_Texture* m_text = 0;

    int m_tile_height = 0;
    int m_tile_width = 0;

    struct ThreadData
    {
        CHAR_INFO* m_data = 0;
        Coord m_dimensions = { 0, 0 };
        std::vector<SMALL_RECT> m_render_regions;
    };
    ThreadData m_shared_data;
    std::mutex m_mutex;

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
    SDL::Scoped::Surface tiles(load_bmp(getResourcePath("") + "tiles.bmp"));
    //SDL::Scoped::Surface tiles(load_bmp(getResourcePath("") + "sprites.bmp"));
    m_tile_height = tiles->h / TILE_STATES;
    m_tile_width = tiles->w / TILE_COUNT;

    SDL::Scoped::Surface text(load_bmp(getResourcePath("") + "text.bmp"));
    assert(m_tile_height == text->h / TEXT_STATES);
    assert(m_tile_width == text->w / TEXT_COUNT);

    m_window = SDL_CreateWindow("Rogue", 100, 100, m_tile_width * 80, m_tile_height * 25, SDL_WINDOW_SHOWN);
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

void SdlWindow::Impl::RenderRegion(CHAR_INFO* data, Coord dimensions, SMALL_RECT rect)
{
    for (int x = rect.Left; x <= rect.Right; ++x) {
        for (int y = rect.Top; y <= rect.Bottom; ++y) {
            auto p = get_screen_pos({ x, y });
            auto info = data[y*dimensions.x + x];
            int c = (unsigned char)info.Char.AsciiChar;

            //todo: how to correctly determine text or monster/passage/wall?
            //the code below works for original tile set, but not others
            if (c >= 0x20 && c < 128 ||
                c == PASSAGE || c == HWALL || c == VWALL || c == LLWALL || c == LRWALL || c == URWALL || c == ULWALL ||
                c == 0xcc || c == 0xb9 || //double line drawing
                c == 0xda || c == 0xb3 || c == 0xc0 || c == 0xc4 || c == 0xbf || c == 0xd9 || //line drawing
                c == 0x11 || c == 0x19 || c == 0x1a || c == 0x1b) //used in help
            {
                int i = get_text_index(info.Attributes);
                auto r = get_text_rect(c, i);
                render_texture_at(m_text, m_renderer, p, r);
            }
            else {
                auto i = tile_index(info.Char.AsciiChar);
                if (i != -1) {
                    bool inv = use_inverse(info.Attributes);
                    auto r = get_tile_rect(i, inv);
                    render_texture_at(m_tiles, m_renderer, p, r);
                }
                else {
                    throw_error(std::string("Unsupported char ") + (char)c);
                }
            }
        }
    }
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
    std::lock_guard<std::mutex> lock(m_mutex);
    m_shared_data.m_dimensions = dimensions;
    m_shared_data.m_data = new CHAR_INFO[dimensions.x*dimensions.y];
}

void SdlWindow::Impl::Draw(_CHAR_INFO * info)
{
    SMALL_RECT r;
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        r = shared_data_full_rect();
    }
    Draw(info, r);
}

bool equal(SMALL_RECT r1, SMALL_RECT r2)
{
    return (r1.Left == r2.Left &&
        r1.Top == r2.Top &&
        r1.Right == r2.Right &&
        r1.Bottom == r2.Bottom);
}

inline void SdlWindow::Impl::Draw(_CHAR_INFO * info, _SMALL_RECT rect)
{
    std::lock_guard<std::mutex> lock(m_mutex);

    auto full_region = shared_data_full_rect();

    //If we're adding a full render to the queue, we can ignore any previous regions.
    //If we're behind on rendering, clear the queue and do a single full render.
    if (equal(rect, full_region) || m_shared_data.m_render_regions.size() > 50)
    {
        m_shared_data.m_render_regions.clear();
        m_shared_data.m_render_regions.push_back(full_region);
    }
    else {
        m_shared_data.m_render_regions.push_back(rect);
    }

    memcpy(m_shared_data.m_data, info, shared_data_size());
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

        std::vector<SMALL_RECT> regions;
        Coord dimensions;
        CHAR_INFO* temp = 0;
        {
            std::lock_guard<std::mutex> lock(m_mutex);

            dimensions = m_shared_data.m_dimensions;
            if (dimensions == EMPTY_COORD)
                continue;

            if (m_shared_data.m_render_regions.empty())
                continue;

            temp = new CHAR_INFO[dimensions.x*dimensions.y];
            memcpy(temp, m_shared_data.m_data, shared_data_size());

            regions = m_shared_data.m_render_regions;
            m_shared_data.m_render_regions.clear();
        }
        std::unique_ptr<CHAR_INFO[]> data(temp);

        for (auto i = regions.begin(); i != regions.end(); ++i)
        {
            RenderRegion(data.get(), dimensions, *i);
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

int SdlWindow::Impl::shared_data_size() const
{
    return sizeof(CHAR_INFO) * m_shared_data.m_dimensions.x * m_shared_data.m_dimensions.y;
}

SMALL_RECT SdlWindow::Impl::shared_data_full_rect()
{
    SMALL_RECT r;
    r.Left = 0;
    r.Top = 0;
    r.Right = short(m_shared_data.m_dimensions.x - 1);
    r.Bottom = short(m_shared_data.m_dimensions.y - 1);
    return r;
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
