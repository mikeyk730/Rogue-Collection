#include <cassert>
#include <algorithm>
#include <map>
#include <deque>
#include <mutex>
#include <iterator>
#include <fstream>
#include <ctime>
#include <sstream>
#include <SDL.h>
#include <SDL_image.h>
#include "sdl_rogue.h"
#include "utility.h"
#include "environment.h"
#include "../Shared/pc_gfx_charmap.h"

#define CTRL(ch)   (ch&0x1f)
#define ESCAPE     (0x1b)

namespace
{
    std::map<int, int> unix_chars = {
        { PASSAGE,   '#' },
        { DOOR,      '+' },
        { FLOOR,     '.' },
        { PLAYER,    '@' },
        { TRAP,      '^' },
        { STAIRS,    '%' },
        { GOLD,      '*' },
        { POTION,    '!' },
        { SCROLL,    '?' },
        { FOOD,      ':' },
        { STICK,     '/' },
        { ARMOR,     ']' },
        { AMULET,    ',' },
        { RING,      '=' },
        { WEAPON,    ')' },
        { VWALL,     '|' },
        { HWALL,     '-' },
        { ULWALL,    '-' },
        { URWALL,    '-' },
        { LLWALL,    '-' },
        { LRWALL,    '-' },
        { 204,       '|' },
        { 185,       '|' },
    };

    uint32_t char_text(uint32_t ch)
    {
        return ch & 0x0000ffff;
    }

    uint32_t char_color(uint32_t ch)
    {
        return (ch >> 24) & 0xff;
    }

    bool is_text(uint32_t ch)
    {
        return (ch & 0x010000) == 0;
    }
}

struct SdlRogue::Impl
{
    const unsigned int MAX_QUEUE_SIZE = 1;

    Impl(SDL_Window* window, SDL_Renderer* renderer, std::shared_ptr<Environment>env, int i);
    Impl(SDL_Window* window, SDL_Renderer* renderer, std::shared_ptr<Environment>env, const std::string& file);
    ~Impl();

    void SetDimensions(Coord dimensions);
    void UpdateRegion(uint32_t* info);
    void UpdateRegion(uint32_t* info, Region rect);
    void MoveCursor(Coord pos);
    void SetCursor(bool enable);

    Environment* environment();
    Options options();

    void Run();
    void Quit();

private:
    void LoadAssets();
    void Render(bool force);
    void RenderRegion(uint32_t* info, Coord dimensions, Region rect);
    void RenderText(uint32_t info, SDL_Rect r, bool is_text, unsigned char color);
    void RenderTile(uint32_t info, SDL_Rect r);
    void RenderCursor(Coord pos);

    void SaveGame();
    void RestoreGame(const std::string& filename, Environment* curr_env);
    void SetGame(const std::string& name);
    void SetGame(int i);

    const GraphicsConfig& gfx_cfg() const;

    void set_window_size(int w, int h);
    Coord get_screen_pos(Coord buffer_pos);

    int tile_index(unsigned char c, unsigned short attr);
    bool use_inverse(unsigned short attr);
    SDL_Rect get_tile_rect(int i, bool use_inverse);

    int get_text_index(unsigned short attr);
    SDL_Rect get_text_rect(unsigned char c, int i);

    Region shared_data_full_region();     //must have mutex before calling
    bool shared_data_is_narrow();         //must have mutex before calling

private:
    SDL_Window* m_window = 0;
    SDL_Renderer* m_renderer = 0;
    SDL_Texture* m_tiles = 0;
    SDL_Texture* m_text = 0;
    std::shared_ptr<Environment> m_env;

    Coord m_block_size = { 0, 0 };
    int m_tile_states = 0;
    Coord m_text_dimensions = { 0, 0 };
    std::map<int, int> m_attr_index;
    int m_gfx_mode = 0;

    Options m_options;

    struct ThreadData
    {
        uint32_t* m_data = 0;
        Coord m_dimensions = { 0, 0 };
        bool m_cursor = false;
        Coord m_cursor_pos = { 0, 0 };
        std::vector<Region> m_render_regions;
    };
    ThreadData m_shared_data;
    std::mutex m_mutex;

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
        { MAGIC,  63 },
        { BMAGIC, 64 },
        { '\\',   65 },
        { '/',    66 },
        { '-',    67 },
        { '|',    68 },
        { '*',    77 },
    };    

    //todo: 2 classes
public:
    char GetChar(bool block, bool *is_replay);
    void Flush();
private:
    void HandleEventText(const SDL_Event& e);
    void HandleEventKeyDown(const SDL_Event& e);
    void HandleEventKeyUp(const SDL_Event& e);

    SDL_Keycode TranslateNumPad(SDL_Keycode keycode, uint16_t modifiers);
    std::string TranslateKey(SDL_Keycode keycode, uint16_t modifiers);

    std::deque<unsigned char> m_buffer;
    std::vector<unsigned char> m_keylog;
    int m_replay_steps_remaining = 0;
    int m_steps_to_take = 0;
    bool m_paused = false;

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

SdlRogue::Impl::Impl(SDL_Window* window, SDL_Renderer* renderer, std::shared_ptr<Environment> current_env, const std::string& file) :
    m_window(window),
    m_renderer(renderer)
{
    RestoreGame(file, current_env.get());

    std::string gfx_pref;
    if (current_env->get("gfx", &gfx_pref)) {
        for (size_t i = 0; i < m_options.gfx_options.size(); ++i)
        {
            if (m_options.gfx_options[i].name == gfx_pref) {
                m_gfx_mode = i;
                break;
            }
        }
    }

    SDL_ShowWindow(window);
    LoadAssets();
}

SdlRogue::Impl::Impl(SDL_Window* window, SDL_Renderer* renderer, std::shared_ptr<Environment>env, int i) : 
    m_window(window),
    m_renderer(renderer),
    m_env(env)
{
    int seed = (int)time(0);
    std::ostringstream ss;
    ss << seed;
    m_env->set("seed", ss.str());
 
    SetGame(i);

    std::string gfx_pref;
    if (m_env->get("gfx", &gfx_pref)) {
        for (size_t i = 0; i < m_options.gfx_options.size(); ++i)
        {
            if (m_options.gfx_options[i].name == gfx_pref) {
                m_gfx_mode = i;
                break;
            }
        }
    }

    SDL_ShowWindow(window);
    LoadAssets();
}

SdlRogue::Impl::~Impl()
{
    SDL_DestroyTexture(m_text);
    SDL_DestroyTexture(m_tiles);
}

int SdlRogue::Impl::tile_index(unsigned char c, unsigned short attr)
{
    if (c >= 'A' && c <= 'Z')
        return c - 'A';

    auto i = m_index.find(c);
    if (i == m_index.end())
        return -1;

    int index = i->second;
    if (index >= 65 && index <= 68) //different color bolts use different tiles
    {
        if (attr & 0x02) //yellow
            index += 8;
        else if (attr & 0x04) //red
            index += 4;
    }
    return index;
}

bool SdlRogue::Impl::use_inverse(unsigned short attr)
{
    return attr > 100 && attr != 160;
}

SDL_Rect SdlRogue::Impl::get_tile_rect(int i, bool use_inverse)
{
    SDL_Rect r;
    r.h = m_block_size.y;
    r.w = m_block_size.x;
    r.x = i*m_block_size.x;
    r.y = use_inverse ? m_block_size.y : 0;
    return r;
}

void SdlRogue::Impl::LoadAssets()
{
    if (m_text) {
        SDL_DestroyTexture(m_text);
        m_text = 0;
    }
    if (m_tiles) {
        SDL_DestroyTexture(m_tiles);
        m_tiles = 0;
    }
    m_attr_index.clear();

    const GraphicsConfig& gfx = gfx_cfg();

    SDL::Scoped::Texture text(loadImage(getResourcePath("") + gfx.text_cfg->filename, m_renderer));
    m_text = text.release();
    int textw, texth;
    SDL_QueryTexture(m_text, NULL, NULL, &textw, &texth);

    m_text_dimensions.x = textw / gfx.text_cfg->layout.x;
    m_text_dimensions.y = texth / gfx.text_cfg->colors.size() / gfx.text_cfg->layout.y;
    for (size_t i = 0; i < gfx.text_cfg->colors.size(); ++i)
        m_attr_index[gfx.text_cfg->colors[i]] = i;
    m_block_size = m_text_dimensions;

    if (gfx.tile_cfg)
    {
        SDL::Scoped::Surface tiles(load_bmp(getResourcePath("") + gfx.tile_cfg->filename));
        m_block_size.x = tiles->w / gfx.tile_cfg->count;
        m_block_size.y = tiles->h / gfx.tile_cfg->states;
        m_tile_states = gfx.tile_cfg->states;
        m_tiles = create_texture(tiles.get(), m_renderer).release();
    }

    set_window_size(m_block_size.x * m_env->cols(), m_block_size.y * m_env->lines());
    SDL_RenderClear(m_renderer);
}

void SdlRogue::Impl::Render(bool force)
{
    std::vector<Region> regions;
    Coord dimensions;
    std::unique_ptr<uint32_t[]> data;
    Coord cursor_pos;
    bool show_cursor;

    //locked region
    {
        std::lock_guard<std::mutex> lock(m_mutex);

        dimensions = m_shared_data.m_dimensions;
        if (dimensions.x == 0 || dimensions.y == 0)
            return;

        if (m_shared_data.m_render_regions.empty() && !force)
            return;

        uint32_t* temp = new uint32_t[dimensions.x*dimensions.y];
        memcpy(temp, m_shared_data.m_data, dimensions.x*dimensions.y*sizeof(uint32_t));
        data.reset(temp);

        regions = m_shared_data.m_render_regions;
        m_shared_data.m_render_regions.clear();

        show_cursor = m_shared_data.m_cursor;
        cursor_pos = m_shared_data.m_cursor_pos;
    }

    if (force) {
        SDL_RenderClear(m_renderer);
        regions.push_back({ 0,0,dimensions.x-1,dimensions.y-1});
    }

    for (auto i = regions.begin(); i != regions.end(); ++i)
    {
        RenderRegion(data.get(), dimensions, *i);
    }

    if (show_cursor) {
        RenderCursor(cursor_pos);
    }

    SDL_RenderPresent(m_renderer);
}

void SdlRogue::Impl::RenderRegion(uint32_t* data, Coord dimensions, Region rect)
{
    for (int x = rect.Left; x <= rect.Right; ++x) {
        for (int y = rect.Top; y <= rect.Bottom; ++y) {
            Coord p = get_screen_pos({ x, y });

            // We always render using the tile size.  Text will be scaled if it doesn't match
            SDL_Rect r;
            r.x = p.x;
            r.y = p.y;
            r.w = m_block_size.x;
            r.h = m_block_size.y;

            uint32_t info = data[y*dimensions.x+x];

            //todo: how to correctly determine text vs monster/passage/wall?
            if (!m_tiles || is_text(info))
            {
                //int color = (y >= 23) ? 0x0e : 0;
                RenderText(info, r, false, 0);
            }
            else {
                RenderTile(info, r);
            }
        }
    }
}

unsigned int GetColor(int chr, int attr)
{
    //if it is inside a room
    if (attr == 0x07 || attr == 0) switch (chr)
    {
    case DOOR:
    case VWALL: case HWALL:
    case ULWALL: case URWALL: case LLWALL: case LRWALL:
        return 0x06; //brown
    case FLOOR:
        return 0x0a; //light green
    case STAIRS:
        return 0xa0; //black on light green
    case TRAP:
        return 0x05; //magenta
    case GOLD:
    case PLAYER:
        return 0x0e; //yellow
    case POTION:
    case SCROLL:
    case STICK:
    case ARMOR:
    case AMULET:
    case RING:
    case WEAPON:
        return 0x09; //light blue
    case FOOD:
        return 0x04; //red
    }
    //if inside a passage or a maze
    else if (attr == 0x70) switch (chr)
    {
    case FOOD:
        return 0x74; //red on grey
    case GOLD: case PLAYER:
        return 0x7e; //yellow on grey
    case POTION: case SCROLL: case STICK: case ARMOR: case AMULET: case RING: case WEAPON:
        return 0x71; //blue on grey
    }

    return attr;
}


void SdlRogue::Impl::RenderText(uint32_t info, SDL_Rect r, bool is_text, unsigned char color)
{
    unsigned char c = char_text(info);
    if (!color) {
        color = GetColor(c, char_color(info));
    }
    if (!gfx_cfg().use_colors) {
        color = 0;
    }

    if (!is_text && gfx_cfg().use_unix_gfx)
    {
        auto i = unix_chars.find(c);
        if (i != unix_chars.end())
            c = i->second;
    }
    int i = get_text_index(color);
    SDL_Rect clip = get_text_rect(c, i);

    SDL_SetRenderDrawColor(m_renderer, 0, 0, 0, 255);
    SDL_RenderFillRect(m_renderer, &r);
    SDL_RenderCopy(m_renderer, m_text, &clip, &r);
}

void SdlRogue::Impl::RenderTile(uint32_t info, SDL_Rect r)
{
    auto i = tile_index(char_text(info), char_color(info));
    if (i == -1)
    {
        //draw a black tile if we don't have a tile for this character
        SDL_SetRenderDrawColor(m_renderer, 0, 0, 0, 255);
        SDL_RenderFillRect(m_renderer, &r);
        return;
    }
    bool inv = (m_tile_states > 1 && char_color(info));
    SDL_Rect clip = get_tile_rect(i, inv);
    SDL_RenderCopy(m_renderer, m_tiles, &clip, &r);
}

void SdlRogue::Impl::RenderCursor(Coord pos)
{
    pos = get_screen_pos(pos);

    SDL_Rect r;
    r.x = pos.x;
    r.y = pos.y + (m_block_size.y * 3 / 4);
    r.w = m_block_size.x;
    r.h = m_block_size.y/4;

    int color = 0x0f;
    int i = get_text_index(color);
    SDL_Rect clip = get_text_rect(0xdb, i);

    SDL_RenderCopy(m_renderer, m_text, &clip, &r);
}

void SdlRogue::Impl::SaveGame()
{
    const unsigned char version = 1;

    std::string path;
    if (!GetSavePath(path))
        return;

    std::ofstream file(path, std::ios::binary | std::ios::out);
    if (!file) {
        ErrorBox("Couldn't open file: " + path);
    }

    write(file, version);
    write_short_string(file, m_options.name);
    m_env->serialize(file);    
    std::copy(m_keylog.begin(), m_keylog.end(), std::ostreambuf_iterator<char>(file));

    if (!file) {
        ErrorBox("Error writing to file: " + path);
    }
}

void SdlRogue::Impl::RestoreGame(const std::string& path, Environment* curr_env)
{
    std::ifstream file(path, std::ios::binary | std::ios::in);
    if (!file) {
        ErrorBox("Couldn't open file: " + path);
    }

    unsigned char version;
    read(file, &version);

    std::string name;
    read_short_string(file, &name);

    m_env.reset(new Environment());
    m_env->deserialize(file);
    
    std::string value;
    if (curr_env->get("logfile", &value))
        m_env->set("logfile", value);

    SetGame(name);
    m_buffer.assign(std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>());
    m_replay_steps_remaining = m_buffer.size();
    if (curr_env->get("pause_replay", &value) && value == "true")
        m_paused = true;
}

void SdlRogue::Impl::SetGame(const std::string & name)
{
    for (size_t i = 0; i < s_options.size(); ++i)
    {
        if (s_options[i].name == name) {
            SetGame(i);
            return;
        }
    }
    throw_error("Unknown game " + name);
}

void SdlRogue::Impl::SetGame(int i)
{
    m_options = s_options[i];

    if (!m_env->write_to_os(m_options.is_unix))
        throw_error("Couldn't write environment");

    std::string screen;
    Coord dims = m_options.screen;
    if (m_env->get("small_screen", &screen) && screen == "true")
    {
        dims = m_options.small_screen;
    }
    m_env->cols(dims.x);
    m_env->lines(dims.y);

    SDL_SetWindowTitle(m_window, std::string("Rogue Collection - " + m_options.name).c_str());
}

const GraphicsConfig & SdlRogue::Impl::gfx_cfg() const
{
    return m_options.gfx_options[m_gfx_mode];
}

void SdlRogue::Impl::set_window_size(int w, int h)
{
    //mdk: i don't know why it's needed, but this code prevents ugly
    //scaling for very narrow windows
    int winw = std::max(w, 342);
    SDL_SetWindowSize(m_window, winw, h);
    SDL_RenderSetLogicalSize(m_renderer, w, h);
}

Coord SdlRogue::Impl::get_screen_pos(Coord buffer_pos)
{
    Coord p;
    p.x = buffer_pos.x * m_block_size.x;
    p.y = buffer_pos.y * m_block_size.y;
    return p;
}

int SdlRogue::Impl::get_text_index(unsigned short attr)
{
    auto i = m_attr_index.find(attr);
    if (i != m_attr_index.end())
        return i->second;
    return 0;
}

SDL_Rect SdlRogue::Impl::get_text_rect(unsigned char ch, int i)
{
    Coord layout = gfx_cfg().text_cfg->layout;
    SDL_Rect r;
    r.h = m_text_dimensions.y;
    r.w = m_text_dimensions.x;
    r.x = (ch % layout.x) * m_text_dimensions.x;
    r.y = (i*layout.y + ch/layout.x) * m_text_dimensions.y;
    return r;
}

void SdlRogue::Impl::SetDimensions(Coord dimensions)
{
    //todo: this function is not needed now that we have env.  we can do
    //this logic in the ctor, and take dimensions out of shared data
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_shared_data.m_dimensions = dimensions;
        m_shared_data.m_data = new uint32_t[dimensions.x*dimensions.y];
    }
    //set_window_size(m_block_size.x*dimensions.x, m_block_size.y*dimensions.y);
}

void SdlRogue::Impl::UpdateRegion(uint32_t * info)
{
    UpdateRegion(info, shared_data_full_region());
}

void SdlRogue::Impl::UpdateRegion(uint32_t* info, Region rect)
{
    std::lock_guard<std::mutex> lock(m_mutex);

    //todo: If we're adding a full render to the queue, we can ignore any previous regions.

    //If we're behind on rendering, clear the queue and do a single full render.
    if (m_shared_data.m_render_regions.size() > MAX_QUEUE_SIZE)
    {
        m_shared_data.m_render_regions.clear();
        m_shared_data.m_render_regions.push_back(shared_data_full_region());
    }
    else {
        m_shared_data.m_render_regions.push_back(rect);
        SDL_Event sdlevent;
        sdlevent.type = SDL_USEREVENT;
        sdlevent.user.code = 0;
        SDL_PushEvent(&sdlevent);
    }

    memcpy(m_shared_data.m_data, info, m_shared_data.m_dimensions.x * m_shared_data.m_dimensions.y * sizeof(int32_t));
}

void SdlRogue::Impl::MoveCursor(Coord pos)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    m_shared_data.m_cursor_pos = pos;
}

void SdlRogue::Impl::SetCursor(bool enable)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    m_shared_data.m_cursor = enable;
}

Environment* SdlRogue::Impl::environment()
{
    return m_env.get();
}

Options SdlRogue::Impl::options()
{
    return m_options;
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
            SDL_FlushEvent(SDL_USEREVENT);
            Render(e.user.code != 0);
        }
    }
}

void SdlRogue::Impl::Quit()
{
    SDL_Event sdlevent;
    sdlevent.type = SDL_QUIT;
    SDL_PushEvent(&sdlevent);
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

SdlRogue::SdlRogue(SDL_Window* window, SDL_Renderer* renderer, std::shared_ptr<Environment>env, int index) :
    m_impl(new Impl(window, renderer, env, index))
{
}

SdlRogue::SdlRogue(SDL_Window* window, SDL_Renderer* renderer, std::shared_ptr<Environment>env, const std::string& file) :
    m_impl(new Impl(window, renderer, env, file))
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

Environment* SdlRogue::environment() const
{
    return m_impl->environment();
}

Options SdlRogue::options() const
{
    return m_impl->options();
}

void SdlRogue::SetDimensions(Coord dimensions)
{
    m_impl->SetDimensions(dimensions);
}

void SdlRogue::UpdateRegion(uint32_t * buf)
{
    m_impl->UpdateRegion(buf);
}

void SdlRogue::UpdateRegion(uint32_t* info, Region r)
{
    m_impl->UpdateRegion(info, r);
}

void SdlRogue::MoveCursor(Coord pos)
{
    m_impl->MoveCursor(pos);
}

void SdlRogue::SetCursor(bool enable)
{
    m_impl->SetCursor(enable);
}

char SdlRogue::GetChar(bool block, bool *is_replay)
{
    return m_impl->GetChar(block, is_replay);
}

void SdlRogue::Flush()
{
    m_impl->Flush();
}

char SdlRogue::Impl::GetChar(bool block, bool *is_replay)
{
    std::unique_lock<std::mutex> lock(m_input_mutex);
    while (m_replay_steps_remaining > 0 && m_paused && m_steps_to_take == 0)
    {
        m_input_cv.wait(lock);
    }

    while (m_buffer.empty()) {
        if (!block) 
            return 0;
        m_input_cv.wait(lock);
    }

    char c = m_buffer.front();
    m_buffer.pop_front();
    m_keylog.push_back(c);

    if (is_replay)
        *is_replay = (m_replay_steps_remaining > 0);

    if (m_replay_steps_remaining > 0)
        --m_replay_steps_remaining;
    if (m_steps_to_take > 0)
        --m_steps_to_take;

    return c;
}

void SdlRogue::Impl::Flush()
{
    std::unique_lock<std::mutex> lock(m_input_mutex);
    if (m_replay_steps_remaining > 0)
        return;

    m_buffer.clear();
}

#ifdef WIN32
#include <Windows.h>
bool is_scroll_lock_on()
{
    return LOBYTE(GetKeyState(VK_SCROLL)) != 0;
}
#else
bool is_scroll_lock_on()
{
    return false;
}
#endif

std::string GetDirectionKey(SDL_Keycode keycode, uint16_t modifiers, bool emulate_ctrl, bool is_original)
{
    std::string keybuf;

    bool caps((modifiers & KMOD_CAPS) != 0);
    bool shift((modifiers & KMOD_SHIFT) != 0);
    if (caps ^ shift) {
        keybuf.push_back(toupper(keycode));
        return keybuf;
    }

    bool scroll = false; // (is_scroll_lock_on());
    bool ctrl((modifiers & KMOD_CTRL) != 0);
    if (scroll ^ ctrl) {
        if (emulate_ctrl) {
            keybuf.push_back('f');
            keybuf.push_back(keycode);
            return keybuf;
        }
        keybuf.push_back(CTRL(keycode));
        return keybuf;
    }

    if (!is_original)
        keybuf.push_back(keycode);
    return keybuf;
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

//If numlock is off, the numpad gets translated to the arrow keys/page up/page down/etc..
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

std::string SdlRogue::Impl::TranslateKey(SDL_Keycode original, uint16_t modifiers)
{
    bool use = false;
    SDL_Keycode keycode = TranslateNumPad(original, modifiers);

    if ((modifiers & KMOD_ALT) && keycode == SDLK_F9) {
        keycode = 'F';
        use = true;
    }

    auto i = m_keymap.find(keycode);
    if (i != m_keymap.end()) {
        keycode = i->second;
        use = true;
    }

    if (IsDirectionKey(keycode))
        return GetDirectionKey(keycode, modifiers, m_options.emulate_ctrl_controls, keycode==original);

    if ((modifiers & KMOD_CTRL) && IsLetterKey(keycode)) {
        keycode = CTRL(keycode);
        use = true;
    }

    if (use){
        return std::string(1, keycode);
    }
    return "";
}

void SdlRogue::Impl::HandleEventKeyDown(const SDL_Event & e)
{
    if (m_replay_steps_remaining > 0)
        return;

    if (e.key.keysym.sym == 's' && (e.key.keysym.mod & KMOD_CTRL))
    {
        SaveGame();
        return;
    }

    std::string new_input = TranslateKey(e.key.keysym.sym, e.key.keysym.mod);
    if (!new_input.empty())
    {
        std::lock_guard<std::mutex> lock(m_input_mutex);
        std::copy(new_input.begin(), new_input.end(), std::back_inserter(m_buffer));
        m_input_cv.notify_all();
    }
}

void SdlRogue::Impl::HandleEventText(const SDL_Event & e)
{
    //todo: when does string have more than 1 char?
    char ch = e.text.text[0];

    if (ch == '`')
    {
        m_gfx_mode = (m_gfx_mode + 1) % m_options.gfx_options.size();
        LoadAssets();
        SDL_Event sdlevent;
        sdlevent.type = SDL_USEREVENT;
        sdlevent.user.code = 1;
        SDL_PushEvent(&sdlevent);
        return;
    }

    if (m_replay_steps_remaining > 0)
    {
        if (ch == ' ') {
            std::lock_guard<std::mutex> lock(m_input_mutex);   
            if (m_paused) {
                ++m_steps_to_take;
                if (m_buffer.front() == 'f' && m_options.emulate_ctrl_controls) {
                    ++m_steps_to_take;
                }
            }
            m_paused = true;
            m_input_cv.notify_all();
        }
        else if (ch == 'r') {
            std::lock_guard<std::mutex> lock(m_input_mutex);
            m_paused = false;
            m_steps_to_take = 0;
            m_input_cv.notify_all();
        }
        else if (ch == 'c') {
            std::lock_guard<std::mutex> lock(m_input_mutex);
            m_buffer.clear();
            m_replay_steps_remaining = 0;
            m_paused = false;
            m_steps_to_take = 0;
            m_input_cv.notify_all();
        }

        return;
    }

    std::string new_input;
    new_input.push_back(ch);

    std::lock_guard<std::mutex> lock(m_input_mutex);
    std::copy(new_input.begin(), new_input.end(), std::back_inserter(m_buffer));
    m_input_cv.notify_all();
}


void SdlRogue::Impl::HandleEventKeyUp(const SDL_Event & e)
{
    if (m_options.is_unix)
        return;

    if (e.key.keysym.sym == SDLK_SCROLLLOCK ||
        e.key.keysym.sym == SDLK_CAPSLOCK ||
        e.key.keysym.sym == SDLK_NUMLOCKCLEAR){

        std::lock_guard<std::mutex> lock(m_input_mutex);
        m_buffer.push_back(0);
        m_input_cv.notify_all();
    }
}
