#include <cassert>
#include <map>
#include <vector>
#include <deque>
#include <mutex>
#include "SDL.h"
#include "sdl_rogue.h"
#include "utility.h"
#include "RogueCore/rogue.h"
#include "RogueCore/coord.h"
#include "RogueCore/io.h"
#include "RogueCore/mach_dep.h"
#include "RogueCore/game_state.h"
#include "RogueCore/curses.h"

struct TileConfig
{
    std::string filename;
    int count;
    int states;
};

struct SdlRogue::Impl
{
    const Coord EMPTY_COORD = { 0, 0 };
    const unsigned int MAX_QUEUE_SIZE = 50;

    Impl();
    ~Impl();

    void SetDimensions(Coord dimensions);
    void Draw(CharInfo * info, bool* text_mask);
    void Draw(CharInfo * info, bool* text_mask, Region rect);
    void Run();
    void Quit();

private:
    void Render();
    void RenderRegion(CharInfo* info, bool* text_mask, Coord dimensions, Region rect);
    void RenderText(CharInfo info, SDL_Rect r);
    void RenderTile(CharInfo info, SDL_Rect r);

    Coord get_screen_pos(Coord buffer_pos);

    int tile_index(unsigned char c);
    bool use_inverse(unsigned short attr);
    SDL_Rect get_tile_rect(int i, bool use_inverse);

    int get_text_index(unsigned short attr);
    SDL_Rect get_text_rect(unsigned char c, int i);

    int shared_data_size() const;         //must have mutex before calling
    Region shared_data_full_region();     //must have mutex before calling
    bool shared_data_is_narrow();         //must have mutex before calling

private:
    SDL_Window* m_window = 0;
    SDL_Renderer* m_renderer = 0;
    SDL_Texture* m_tiles = 0;
    SDL_Texture* m_text = 0;

    TileConfig m_tile_cfg;
    Coord m_tile_dimensions = { 0, 0 };

    TileConfig m_text_cfg;
    Coord m_text_dimensions = { 0, 0 };

    struct ThreadData
    {
        CharInfo* m_data = 0;
        bool* m_text_mask = 0;
        Coord m_dimensions = { 0, 0 };
        std::vector<Region> m_render_regions;
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

    //todo: 2 classes
public:
    char GetNextChar(bool do_key_state);
    virtual std::string GetNextString(int size);
private:
    void HandleEventText(const SDL_Event& e);
    void HandleEventKeyDown(const SDL_Event& e);
    void HandleEventKeyUp(const SDL_Event& e);

    SDL_Keycode TranslateNumPad(SDL_Keycode keycode, uint16_t modifiers);
    char TranslateKey(SDL_Keycode keycode, uint16_t modifiers);


    std::deque<unsigned char> m_buffer;
    std::mutex m_input_mutex;
    std::condition_variable m_input_cv;

    std::map<SDL_Keycode, SDL_Keycode> m_numpad = {
        { SDLK_KP_0, SDLK_INSERT },
        { SDLK_KP_1, SDLK_END },
        { SDLK_KP_2, SDLK_DOWN },
        { SDLK_KP_3, SDLK_PAGEDOWN },
        { SDLK_KP_4, SDLK_LEFT },
        { SDLK_KP_6, SDLK_RIGHT },
        { SDLK_KP_7, SDLK_HOME },
        { SDLK_KP_8, SDLK_UP },
        { SDLK_KP_9, SDLK_PAGEUP },
        { SDLK_KP_ENTER, SDLK_RETURN },
    };

    std::map<SDL_Keycode, unsigned char> m_keymap = {
        { SDLK_RETURN,   '\r' },
        { SDLK_BACKSPACE,'\b' },
        { SDLK_ESCAPE,    ESCAPE },
        { SDLK_HOME,     'y' },
        { SDLK_UP,       'k' },
        { SDLK_PAGEUP,   'u' },
        { SDLK_LEFT,     'h' },
        { SDLK_RIGHT,    'l' },
        { SDLK_END,      'b' },
        { SDLK_DOWN,     'j' },
        { SDLK_PAGEDOWN, 'n' },
        { SDLK_INSERT,   '>' },
        { SDLK_DELETE,   's' },
        { SDLK_F1,       '?' },
        { SDLK_F2,       '/' },
        { SDLK_F3,       'a' },
        { SDLK_F4,       CTRL('R') },
        { SDLK_F5,       'c' },
        { SDLK_F6,       'D' },
        { SDLK_F7,       'i' },
        { SDLK_F8,       '^' },
        { SDLK_F9,       CTRL('F') },
        { SDLK_F10,      '!' },
    };
};

namespace
{
    TileConfig pc_tiles = { "tiles.bmp", 78, 2 };
    TileConfig pc_text = { "text.bmp", 256, 16 };

    TileConfig atari_tiles = { "atari.bmp", 78, 1 };
}


SdlRogue::Impl::Impl()
{
    m_tile_cfg = pc_tiles;
    m_tile_cfg = atari_tiles;

    SDL::Scoped::Surface tiles(load_bmp(getResourcePath("") + m_tile_cfg.filename));
    m_tile_dimensions.x = tiles->w / m_tile_cfg.count;
    m_tile_dimensions.y = tiles->h / m_tile_cfg.states;

    m_text_cfg = pc_text;
    SDL::Scoped::Surface text(load_bmp(getResourcePath("") + m_text_cfg.filename));
    m_text_dimensions.x = text->w / m_text_cfg.count;
    m_text_dimensions.y = text->h / m_text_cfg.states;

    m_window = SDL_CreateWindow("Rogue", 100, 100, m_tile_dimensions.x * 80, m_tile_dimensions.y * 25, SDL_WINDOW_SHOWN);
    if (m_window == nullptr)
        throw_error("SDL_CreateWindow");

    m_renderer = SDL_CreateRenderer(m_window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (m_renderer == nullptr)
        throw_error("SDL_CreateRenderer");

    m_tiles = create_texture(tiles.get(), m_renderer).release();
    m_text = create_texture(text.get(), m_renderer).release();
}

SdlRogue::Impl::~Impl()
{
    SDL_DestroyTexture(m_text);
    SDL_DestroyTexture(m_tiles);
    SDL_DestroyRenderer(m_renderer);
    SDL_DestroyWindow(m_window);
}

int SdlRogue::Impl::tile_index(unsigned char c)
{
    if (c >= 'A' && c <= 'Z')
        return c - 'A';
    auto i = m_index.find(c);
    if (i != m_index.end())
        return i->second;
    return -1;
}

bool SdlRogue::Impl::use_inverse(unsigned short attr)
{
    return attr > 100 && attr != 160;
}

inline SDL_Rect SdlRogue::Impl::get_tile_rect(int i, bool use_inverse)
{
    SDL_Rect r;
    r.h = m_tile_dimensions.y;
    r.w = m_tile_dimensions.x;
    r.x = i*m_tile_dimensions.x;
    r.y = use_inverse ? m_tile_dimensions.y : 0;
    return r;
}

void SdlRogue::Impl::Render()
{
    std::vector<Region> regions;
    Coord dimensions;
    std::unique_ptr<CharInfo[]> data;
    std::unique_ptr<bool[]> text_mask;

    //locked region
    {
        std::lock_guard<std::mutex> lock(m_mutex);

        dimensions = m_shared_data.m_dimensions;
        if (dimensions == EMPTY_COORD)
            return;

        if (m_shared_data.m_render_regions.empty())
            return;

        CharInfo* temp = new CharInfo[dimensions.x*dimensions.y];
        memcpy(temp, m_shared_data.m_data, shared_data_size());
        data.reset(temp);

        bool* temp2 = new bool[dimensions.x*dimensions.y];
        memcpy(temp2, m_shared_data.m_text_mask, dimensions.x*dimensions.y*sizeof(bool));
        text_mask.reset(temp2);

        regions = m_shared_data.m_render_regions;
        m_shared_data.m_render_regions.clear();
    }

    for (auto i = regions.begin(); i != regions.end(); ++i)
    {
        RenderRegion(data.get(), text_mask.get(), dimensions, *i);
    }

    SDL_RenderPresent(m_renderer);
}

void SdlRogue::Impl::RenderRegion(CharInfo* data, bool* text_mask, Coord dimensions, Region rect)
{
    for (int x = rect.Left; x <= rect.Right; ++x) {
        for (int y = rect.Top; y <= rect.Bottom; ++y) {
            Coord p = get_screen_pos({ x, y });

            // We always render using the tile size.  Text will be scaled if it doesn't match
            SDL_Rect r;
            r.x = p.x;
            r.y = p.y;
            r.w = m_tile_dimensions.x;
            r.h = m_tile_dimensions.y;

            CharInfo info = data[y*dimensions.x + x];

            //todo: how to correctly determine text or monster/passage/wall?
            //the code below works for original tile set, but not others
            if (text_mask[y*dimensions.x + x])
            {
                RenderText(info, r);
            }
            else {
                RenderTile(info, r);
            }
        }
    }
}

void SdlRogue::Impl::RenderText(CharInfo info, SDL_Rect r)
{
    short attr = info.Attributes;
    if (use_inverse(attr))
        attr = 112;
    int i = get_text_index(attr);
    SDL_Rect clip = get_text_rect(info.Char.AsciiChar, i);

    SDL_RenderCopy(m_renderer, m_text, &clip, &r);
}

void SdlRogue::Impl::RenderTile(CharInfo info, SDL_Rect r)
{
    auto i = tile_index(info.Char.AsciiChar);
    if (i == -1)
    {
        //draw a black tile if we don't have a tile for this character
        SDL_SetRenderDrawColor(m_renderer, 0, 0, 0, 255);
        SDL_RenderFillRect(m_renderer, &r);
        return;
    }
    bool inv = (m_tile_cfg.states > 1) && use_inverse(info.Attributes);
    SDL_Rect clip = get_tile_rect(i, inv);
    SDL_RenderCopy(m_renderer, m_tiles, &clip, &r);
}

inline Coord SdlRogue::Impl::get_screen_pos(Coord buffer_pos)
{
    Coord p;
    p.x = buffer_pos.x * m_tile_dimensions.x;
    p.y = buffer_pos.y * m_tile_dimensions.y;
    return p;
}

inline int SdlRogue::Impl::get_text_index(unsigned short attr)
{
    auto i = m_attr_index.find(attr);
    if (i != m_attr_index.end())
        return i->second;
    return 0;
}

inline SDL_Rect SdlRogue::Impl::get_text_rect(unsigned char c, int i)
{
    SDL_Rect r;
    r.h = m_text_dimensions.y;
    r.w = m_text_dimensions.x;
    r.x = c*m_text_dimensions.x;
    r.y = i*m_text_dimensions.y;
    return r;
}

void SdlRogue::Impl::SetDimensions(Coord dimensions)
{
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_shared_data.m_dimensions = dimensions;
        m_shared_data.m_data = new CharInfo[dimensions.x*dimensions.y];
        m_shared_data.m_text_mask = new bool[dimensions.x*dimensions.y];
    }
    SDL_SetWindowSize(m_window, m_tile_dimensions.x*dimensions.x, m_tile_dimensions.y*dimensions.y);
}

void SdlRogue::Impl::Draw(CharInfo * info, bool* text_mask)
{
    Region r;
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        //If we're adding a full render to the queue, we can ignore any previous regions.
        m_shared_data.m_render_regions.clear();
        r = shared_data_full_region();
    }
    Draw(info, text_mask, r);
}

inline void SdlRogue::Impl::Draw(CharInfo * info, bool* text_mask, Region rect)
{
    std::lock_guard<std::mutex> lock(m_mutex);

    //If we're behind on rendering, clear the queue and do a single full render.
    if (m_shared_data.m_render_regions.size() > MAX_QUEUE_SIZE)
    {
        m_shared_data.m_render_regions.clear();
        m_shared_data.m_render_regions.push_back(shared_data_full_region());
    }
    else {
        m_shared_data.m_render_regions.push_back(rect);
    }

    memcpy(m_shared_data.m_data, info, shared_data_size());
    memcpy(m_shared_data.m_text_mask, text_mask, m_shared_data.m_dimensions.x*m_shared_data.m_dimensions.y*sizeof(bool));
}

void SdlRogue::Impl::Run()
{
    SDL_Event e;
    while (SDL_WaitEvent(&e)) {
        if (e.type == SDL_QUIT) {
            return;
        }
        else if (e.type == SDL_TEXTEDITING) {
            continue;
        }
        else if (e.type == SDL_TEXTINPUT) {
            HandleEventText(e);
        }
        else if (e.type == SDL_KEYDOWN) {
            HandleEventKeyDown(e);
        }
        else if (e.type == SDL_KEYUP) {
            HandleEventKeyUp(e);
        }
        else if (e.type == SDL_USEREVENT) {
            Render();
        }
    }
}

void SdlRogue::Impl::Quit()
{
    SDL_Event sdlevent;
    sdlevent.type = SDL_QUIT;
    SDL_PushEvent(&sdlevent);
}

int SdlRogue::Impl::shared_data_size() const
{
    return sizeof(CharInfo) * m_shared_data.m_dimensions.x * m_shared_data.m_dimensions.y;
}

Region SdlRogue::Impl::shared_data_full_region()
{
    Region r;
    r.Left = 0;
    r.Top = 0;
    r.Right = short(m_shared_data.m_dimensions.x - 1);
    r.Bottom = short(m_shared_data.m_dimensions.y - 1);
    return r;
}

bool SdlRogue::Impl::shared_data_is_narrow()
{
    return m_shared_data.m_dimensions.x == 40;
}






SdlRogue::SdlRogue() :
    m_impl(new Impl())
{
}

SdlRogue::~SdlRogue()
{
}

void SdlRogue::Run()
{
    m_impl->Run();
}

void SdlRogue::Quit()
{
    m_impl->Quit();
}

void SdlRogue::SetDimensions(Coord dimensions)
{
    m_impl->SetDimensions(dimensions);
}

void SdlRogue::Draw(CharInfo * info, bool* text_mask)
{
    m_impl->Draw(info, text_mask);
    SDL_Event sdlevent;
    sdlevent.type = SDL_USEREVENT;
    SDL_PushEvent(&sdlevent);
}

void SdlRogue::Draw(CharInfo * info, bool* text_mask, Region r)
{
    m_impl->Draw(info, text_mask, r);
    SDL_Event sdlevent;
    sdlevent.type = SDL_USEREVENT;
    SDL_PushEvent(&sdlevent);
}

void SdlRogue::MoveCursor(Coord pos)
{
}

void SdlRogue::SetCursor(bool enable)
{
}

bool SdlRogue::HasMoreInput()
{
    return true;
}

char SdlRogue::GetNextChar()
{
    return m_impl->GetNextChar(true);
}

std::string SdlRogue::GetNextString(int size)
{
    return m_impl->GetNextString(size);
}

bool SdlRogue::IsCapsLockOn()
{
    return is_caps_lock_on();
}

bool SdlRogue::IsNumLockOn()
{
    return is_num_lock_on();
}

bool SdlRogue::IsScrollLockOn()
{
    return is_scroll_lock_on();
}

void SdlRogue::Serialize(std::ostream & out)
{
}

char SdlRogue::Impl::GetNextChar(bool do_key_state)
{
    std::unique_lock<std::mutex> lock(m_input_mutex);
    while (m_buffer.empty()) {
        if (do_key_state)
            handle_key_state();
        m_input_cv.wait(lock);
    }

    char c = m_buffer.front();
    m_buffer.pop_front();

    return c;
}

namespace 
{
    void backspace()
    {
        int x, y;
        game->screen().getrc(&x, &y);
        if (--y < 0) y = 0;
        game->screen().move(x, y);
        game->screen().add_text(' ');
        game->screen().move(x, y);
    }
}

std::string SdlRogue::Impl::GetNextString(int size)
{
    std::string s;

    while (true)
    {
        char c = GetNextChar(false);
        switch (c)
        {
        case ESCAPE:
            s.clear();
            s.push_back(ESCAPE);
            return s;
        case '\b':
            if (!s.empty()) {
                backspace();
                s.pop_back();
            }
            break;
        default:
            if (s.size() >= unsigned int(size)) {
                beep();
                break;
            }
            game->screen().add_text(c);
            s.push_back(c);
            break;
        case '\n':
        case '\r':
            return s;
        }
    }
}

void SdlRogue::Impl::HandleEventText(const SDL_Event & e)
{
    std::lock_guard<std::mutex> lock(m_input_mutex);
    //todo: when does string have more than 1 char?
    m_buffer.push_back(e.text.text[0]);
    m_input_cv.notify_all();
}

char ApplyShift(char c, uint16_t modifiers)
{
    bool caps((modifiers & KMOD_CAPS) != 0);
    bool shift((modifiers & KMOD_SHIFT) != 0);
    if (caps ^ shift)
        return toupper(c);
    return c;
}

bool IsLetterKey(SDL_Keycode keycode)
{
    return (keycode >= 'a' && keycode <= 'z');
}

bool IsDirectionKey(SDL_Keycode keycode)
{
    switch (keycode)
    {
    case 'h':
    case 'j':
    case 'k':
    case 'l':
    case 'y':
    case 'u':
    case 'b':
    case 'n':
        return true;
    }
    return false;
}

SDL_Keycode SdlRogue::Impl::TranslateNumPad(SDL_Keycode keycode, uint16_t modifiers)
{
    if ((modifiers & KMOD_NUM) == 0){
        auto i = m_numpad.find(keycode);
        if (i != m_numpad.end())
        {
            return i->second;
        }
    }
    return keycode;
}

char SdlRogue::Impl::TranslateKey(SDL_Keycode keycode, uint16_t modifiers)
{
    if (modifiers & KMOD_CTRL) {
        if (IsLetterKey(keycode)) {
            return CTRL(keycode);
        }
    }
    else if (modifiers & KMOD_ALT) {
        if (keycode == SDLK_F9) {
            return 'F';
        }
    }
    else {
        auto i = m_keymap.find(keycode);
        if (i != m_keymap.end())
        {
            char c = i->second;
            if (IsDirectionKey(c))
                return ApplyShift(c, modifiers);
            return c;
        }
    }

    return 0;
}

void SdlRogue::Impl::HandleEventKeyDown(const SDL_Event & e)
{
    auto keycode = TranslateNumPad(e.key.keysym.sym, e.key.keysym.mod);
    char c = TranslateKey(keycode, e.key.keysym.mod);
    if (c == 0)
        return;

    std::lock_guard<std::mutex> lock(m_input_mutex);
    m_buffer.push_back(c);
    m_input_cv.notify_all();
}

void SdlRogue::Impl::HandleEventKeyUp(const SDL_Event & e)
{
    if (e.key.keysym.sym == SDLK_SCROLLLOCK ||
        e.key.keysym.sym == SDLK_CAPSLOCK ||
        e.key.keysym.sym == SDLK_NUMLOCKCLEAR){
        m_input_cv.notify_all();
    }
}
