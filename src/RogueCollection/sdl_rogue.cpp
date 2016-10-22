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
#include <pc_gfx_charmap.h>
#include "sdl_rogue.h"
#include "utility.h"
#include "environment.h"
#include "text_provider.h"
#include "tile_provider.h"
#include "window_sizer.h"

#define CTRL(ch)   (ch&0x1f)
#define ESCAPE     (0x1b)

const char* SdlRogue::WindowTitle = "Rogue Collection 1.0";

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

    const unsigned char s_version = 1;
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

    Environment* GameEnv();
    Options options();

    void Run();
    void PostQuit();

private:
    void LoadAssets();
    void RenderMainMenu(bool force);
    void Animate();
    void RenderRegion(uint32_t* info, Coord dimensions, Region rect);
    void RenderText(uint32_t info, unsigned char color, SDL_Rect r, bool is_tile);
    void RenderTile(uint32_t info, SDL_Rect r);
    void RenderCursor(Coord pos);
    void RenderReplayOverlay(int steps, Coord dimensions);

    void PostRenderMsg(int force);

    void SaveGame(std::string path, bool notify);
    void RestoreGame(const std::string& filename);
    void SetGame(const std::string& name);
    void SetGame(int i);

    const GraphicsConfig& current_gfx() const;

    Coord get_screen_pos(Coord buffer_pos);
    SDL_Rect get_screen_rect(Coord buffer_pos);

    Region shared_data_full_region();     //must have mutex before calling
    bool shared_data_is_narrow();         //must have mutex before calling

private:
    SDL_Window* m_window = 0;
    SDL_Renderer* m_renderer = 0;
    std::shared_ptr<Environment> m_game_env;
    std::shared_ptr<Environment> m_current_env;

    Coord m_block_size = { 0, 0 };
    int m_gfx_mode = 0;
    WindowSizer m_sizer;
    std::unique_ptr<ITextProvider> m_text_provider;
    std::unique_ptr<TileProvider> m_tile_provider;
    int m_frame_number = 0;
    uint32_t RENDER_EVENT = 0;
    uint32_t TIMER_EVENT = 0;

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

    //todo: 2 classes
public:
    char GetChar(bool block, bool for_string, bool *is_replay);
    void Flush();
private:
    void HandleEventText(const SDL_Event& e);
    void HandleEventKeyDown(const SDL_Event& e);
    void HandleEventKeyUp(const SDL_Event& e);

    void HandleInputReplay(int ch);

    SDL_Keycode TranslateNumPad(SDL_Keycode keycode, uint16_t modifiers);
    std::string TranslateKey(SDL_Keycode keycode, uint16_t modifiers);

    std::deque<unsigned char> m_buffer;
    std::vector<unsigned char> m_keylog;
    int m_replay_steps_remaining = 0;
    int m_steps_to_take = 0;
    int m_replay_sleep = 0;
    int m_pause_at = 0;
    bool m_paused = false;
    uint16_t m_restore_count = 0;

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
    m_renderer(renderer),
    m_current_env(current_env),
    m_sizer(window, renderer, current_env.get())
{
    RestoreGame(file);

    std::string gfx_pref;
    if (m_current_env->get("gfx", &gfx_pref)) {
        for (int i = 0; i < (int)m_options.gfx_options.size(); ++i)
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

SdlRogue::Impl::Impl(SDL_Window* window, SDL_Renderer* renderer, std::shared_ptr<Environment> env, int i) : 
    m_window(window),
    m_renderer(renderer),
    m_current_env(env),
    m_game_env(env),
    m_sizer(window, renderer, env.get())
{
    int seed = (int)time(0);
    std::ostringstream ss;
    ss << seed;
    m_game_env->set("seed", ss.str());
 
    SetGame(i);

    std::string gfx_pref;
    if (m_current_env->get("gfx", &gfx_pref)) {
        for (int i = 0; i < (int)m_options.gfx_options.size(); ++i)
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
}

void SdlRogue::Impl::LoadAssets()
{
    if (current_gfx().text_cfg->generate_colors)
        m_text_provider.reset(new TextGenerator(*(current_gfx().text_cfg), m_renderer)); 
    else
        m_text_provider.reset(new TextProvider(*(current_gfx().text_cfg), m_renderer));
    m_block_size = m_text_provider->dimensions();

    m_tile_provider.reset();
    if (current_gfx().tile_cfg)
    {
        m_tile_provider.reset(new TileProvider(*(current_gfx().tile_cfg), m_renderer));
        m_block_size = m_tile_provider->dimensions();
    }

    m_sizer.SetWindowSize(m_block_size.x * m_game_env->cols(), m_block_size.y * m_game_env->lines());
    SDL_RenderClear(m_renderer);
}

void SdlRogue::Impl::RenderMainMenu(bool force)
{
    std::vector<Region> regions;
    Coord dimensions;
    std::unique_ptr<uint32_t[]> data;
    Coord cursor_pos;
    bool show_cursor;
    int replay_steps_remaining;
    
    //locked region
    {
        std::lock_guard<std::mutex> lock(m_input_mutex);
        replay_steps_remaining = m_replay_steps_remaining;
    }

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

    if (replay_steps_remaining) {
        RenderReplayOverlay(replay_steps_remaining, dimensions);
    }

    SDL_RenderPresent(m_renderer);
}

void SdlRogue::Impl::Animate()
{
    bool update = false;

    if (current_gfx().animate) {

        Coord dimensions;
        std::unique_ptr<uint32_t[]> data;
        {
            std::lock_guard<std::mutex> lock(m_mutex);

            dimensions = m_shared_data.m_dimensions;
            if (dimensions.x == 0 || dimensions.y == 0)
                return;

            uint32_t* temp = new uint32_t[dimensions.x*dimensions.y];
            memcpy(temp, m_shared_data.m_data, dimensions.x*dimensions.y * sizeof(uint32_t));
            data.reset(temp);
        }

        for (int i = 0; i < dimensions.x*dimensions.y; ++i) {
            auto c = char_text(data[i]);
            if (c != STAIRS)
                continue;

            int x = i % dimensions.x;
            int y = i / dimensions.x;
            SDL_Rect r = get_screen_rect({ x, y });
            RenderText(c, char_color(data[i]), r, true);
            update = true;
        }
    }

    bool show_cursor;
    Coord cursor_pos;

    {
        std::lock_guard<std::mutex> lock(m_mutex);
        show_cursor = m_shared_data.m_cursor;
        cursor_pos = m_shared_data.m_cursor_pos;
    }

    if (show_cursor) {
        RenderCursor(cursor_pos);
        update = true;
    }

    if (update)
        SDL_RenderPresent(m_renderer);
}

void SdlRogue::Impl::RenderRegion(uint32_t* data, Coord dimensions, Region rect)
{
    for (int y = rect.Top; y <= rect.Bottom; ++y) {
        for (int x = rect.Left; x <= rect.Right; ++x) {

            SDL_Rect r = get_screen_rect({ x, y });

            uint32_t info = data[y*dimensions.x + x];

            if (!m_tile_provider || is_text(info))
            {
                int color = char_color(info);
                if (y == 0 && color == 0x70) {
                    // Hack for consistent standout in msg lines.  Unix versions use '-'.
                    // PC uses ' ' with background color.  We want consistent behavior.
                    if (current_gfx().use_colors) {
                        if (char_text(info) == '-')
                            info = ' ';
                    }
                    else {
                        if (char_text(info) == ' ')
                            info = '-';
                        color = 0x07;
                    }
                }
                RenderText(info, color, r, !is_text(info));
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
        return 0x20; //black on light green
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


void SdlRogue::Impl::RenderText(uint32_t info, unsigned char color, SDL_Rect r, bool is_tile)
{
    unsigned char c = char_text(info);

    // Tiles from Unix versions come in with either color=0x00 (for regular state)
    // or color=0x70 (for standout).  We need to translate these into more diverse
    // colors.  Tiles from PC versions already have the correct color, so we
    // technically don't need to do anything here, but it doesn't hurt to call
    // GetColor.
    if (is_tile) {
        color = GetColor(c, color);
    }
    if (!color || !current_gfx().use_colors) {
        color = 0x07; //grey on black is the default
    }

    if (current_gfx().animate && c == STAIRS && m_frame_number == 1)
        c = ' ';

    if (current_gfx().use_unix_gfx && is_tile)
    {
        auto i = unix_chars.find(c);
        if (i != unix_chars.end())
            c = i->second;
    }

    SDL_Rect clip;
    SDL_Texture* text;
    m_text_provider->GetTexture(c, color, &text, &clip);

    SDL_SetRenderDrawColor(m_renderer, 0, 0, 0, 255);
    SDL_RenderFillRect(m_renderer, &r);
    SDL_RenderCopy(m_renderer, text, &clip, &r);
}

void SdlRogue::Impl::RenderTile(uint32_t info, SDL_Rect r)
{
    SDL_Texture* tiles;
    SDL_Rect clip;
    if (m_tile_provider->GetTexture(char_text(info), char_color(info), &tiles, &clip)) {
        SDL_RenderCopy(m_renderer, tiles, &clip, &r);
    }
    else{
        //draw a black tile if we don't have a tile for this character
        SDL_SetRenderDrawColor(m_renderer, 0, 0, 0, 255);
        SDL_RenderFillRect(m_renderer, &r);
    }
}

void SdlRogue::Impl::RenderCursor(Coord pos)
{
    pos = get_screen_pos(pos);

    SDL_Rect r;
    r.x = pos.x;
    r.y = pos.y + (m_block_size.y * 3 / 4);
    r.w = m_block_size.x;
    r.h = m_block_size.y/8;

    int color = m_frame_number ? 0x00 : 0x07;

    SDL_Rect clip;
    SDL_Texture* text;
    m_text_provider->GetTexture(0xdb, color, &text, &clip);

    SDL_RenderCopy(m_renderer, text, &clip, &r);
}

void SdlRogue::Impl::RenderReplayOverlay(int steps, Coord dimensions)
{
    std::ostringstream ss;
    ss << "Replay " << steps;
    std::string s(ss.str());
    int len = (int)s.size();
    for (int i = 0; i < len; ++i) {
        SDL_Rect r = get_screen_rect({ dimensions.x - (len - i) - 1, dimensions.y - 1 });
        RenderText(s[i], 0x70, r, false);
    }
}

void SdlRogue::Impl::PostRenderMsg(int force)
{
    SDL_Event e;
    SDL_zero(e);
    e.type = RENDER_EVENT;
    e.user.code = force;
    SDL_PushEvent(&e);
}

void SdlRogue::Impl::SaveGame(std::string path, bool notify)
{
    if (path.empty()) {
        if (!m_current_env->get("savefile", &path) || path.empty()) {
            if (!GetSavePath(m_window, path)) {
                return;
            }
        }
    }

    std::ofstream file(path, std::ios::binary | std::ios::out);
    if (!file) {
        DisplayMessage(SDL_MESSAGEBOX_ERROR, "Save Game", "Couldn't open file: " + path);
        return;
    }

    write(file, s_version);
    write(file, m_restore_count);
    write_short_string(file, m_options.name);
    m_game_env->serialize(file);

    //locked region
    {
        std::unique_lock<std::mutex> lock(m_input_mutex);
        std::copy(m_keylog.begin(), m_keylog.end(), std::ostreambuf_iterator<char>(file));
    }

    if (!file) {
        DisplayMessage(SDL_MESSAGEBOX_ERROR, "Save Game", "Error writing to file: " + path);
        return;
    }
    if (notify) {
        DisplayMessage(SDL_MESSAGEBOX_INFORMATION, "Save Game", "Your game was saved successfully.  Come back soon!");
    }

    std::string value;
    if (m_current_env->get("exit_on_save", &value) && value != "false")
        PostQuit();
}

void SdlRogue::Impl::RestoreGame(const std::string& path)
{
    std::ifstream file(path, std::ios::binary | std::ios::in);
    if (!file) {
        throw_error("Couldn't open file: " + path);
    }

    unsigned char version;
    read(file, &version);
    if (version > s_version)
        throw_error("This file is not recognized.  It may have been saved with a newer version of Rogue Collection.  Please download the latest version and try again.");

    read(file, &m_restore_count);
    ++m_restore_count;

    std::string name;
    read_short_string(file, &name);

    // set up game environment
    m_game_env.reset(new Environment());
    m_game_env->deserialize(file);
    m_game_env->set("in_replay", "true");
    std::string value;
    if (m_current_env->get("logfile", &value)) {
        m_game_env->set("logfile", value);
    }
    
    m_buffer.assign(std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>());
    m_replay_steps_remaining = (int)m_buffer.size();

    file.close();

    if (m_current_env->get("replay_paused", &value) && value == "true") {
        m_paused = true;
    }
    if (m_current_env->get("replay_pause_at", &value)) {
        m_pause_at = atoi(value.c_str());
    }
    if (m_current_env->get("replay_step_delay", &value)) {
        m_replay_sleep = atoi(value.c_str());
    }

    SetGame(name);

    if (!m_current_env->get("delete_on_restore", &value) || value != "false") {
        std::remove(path.c_str());
    }
}

void SdlRogue::Impl::SetGame(const std::string & name)
{
    for (int i = 0; i < (int)s_options.size(); ++i)
    {
        if (s_options[i].name == name) {
            SetGame(i);
            return;
        }
    }
    throw_error("Save file specified unknown game: " + name);
}

void SdlRogue::Impl::SetGame(int i)
{
    m_options = s_options[i];

    if (m_options.name == "PC Rogue 1.1") {
        m_game_env->set("emulate_version", "1.1");
    }

    if (!m_game_env->write_to_os(m_options.is_unix))
        throw_error("Couldn't write environment");

    std::string screen;
    Coord dims = m_options.screen;
    if (m_game_env->get("small_screen", &screen) && screen == "true")
    {
        dims = m_options.small_screen;
    }
    m_game_env->cols(dims.x);
    m_game_env->lines(dims.y);

    std::string title(SdlRogue::WindowTitle);
    title += " - ";
    title += m_options.name;
    SDL_SetWindowTitle(m_window, title.c_str());
}

const GraphicsConfig & SdlRogue::Impl::current_gfx() const
{
    return m_options.gfx_options[m_gfx_mode];
}

Coord SdlRogue::Impl::get_screen_pos(Coord buffer_pos)
{
    Coord p;
    p.x = buffer_pos.x * m_block_size.x;
    p.y = buffer_pos.y * m_block_size.y;
    return p;
}

SDL_Rect SdlRogue::Impl::get_screen_rect(Coord buffer_pos)
{
    Coord p = get_screen_pos(buffer_pos);

    // We always render using the tile size.  Text will be scaled if it doesn't match
    SDL_Rect r;
    r.x = p.x;
    r.y = p.y;
    r.w = m_block_size.x;
    r.h = m_block_size.y;

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
        PostRenderMsg(0);
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

Environment* SdlRogue::Impl::GameEnv()
{
    return m_game_env.get();
}

Options SdlRogue::Impl::options()
{
    return m_options;
}

namespace
{
    Uint32 PostTimerMsg(Uint32 interval, void *type)
    {
        static bool parity = true;
        parity = !parity;

        SDL_Event e;
        SDL_zero(e);
        e.type = *static_cast<uint32_t*>(type);
        e.user.code = parity;
        SDL_PushEvent(&e);

        return interval;
    }
}

void SdlRogue::Impl::Run()
{
    RENDER_EVENT = SDL_RegisterEvents(2);
    TIMER_EVENT = RENDER_EVENT + 1;

    SDL_AddTimer(250, PostTimerMsg, &TIMER_EVENT);

    SDL_Event e;
    while (SDL_WaitEvent(&e)) {
        if (e.type == SDL_QUIT) {
            std::string path;
            if (m_current_env->get("autosave", &path))
                SaveGame(path, false);
            return;
        }
        else if (e.type == SDL_WINDOWEVENT) {
            switch (e.window.event) {
            case SDL_WINDOWEVENT_SIZE_CHANGED:
            case SDL_WINDOWEVENT_EXPOSED:
                PostRenderMsg(1);
            }
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
        else if (e.type == RENDER_EVENT) {
            SDL_FlushEvent(RENDER_EVENT);
            RenderMainMenu(e.user.code != 0);
        }
        else if (e.type == TIMER_EVENT) {
            m_frame_number = e.user.code;
            Animate();
        }

    }
}

void SdlRogue::Impl::PostQuit()
{
    SDL_Event e;
    SDL_zero(e);
    e.type = SDL_QUIT;
    SDL_PushEvent(&e);
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
    m_impl->PostQuit();
}

Environment* SdlRogue::GameEnv() const
{
    return m_impl->GameEnv();
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

char SdlRogue::GetChar(bool block, bool for_string, bool *is_replay)
{
    return m_impl->GetChar(block, for_string, is_replay);
}

void SdlRogue::Flush()
{
    m_impl->Flush();
}

char SdlRogue::Impl::GetChar(bool block, bool for_string, bool *is_replay)
{
    char c = 0;
    int sleep = 0;

    //locked region
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

        c = m_buffer.front();
        m_buffer.pop_front();
        m_keylog.push_back(c);

        if (m_replay_steps_remaining > 0) {
            --m_replay_steps_remaining;
            if (is_replay)
                *is_replay = true;
            if (!m_paused && m_replay_sleep)
                sleep = m_replay_sleep;
            if (m_steps_to_take > 0 && !(for_string && c != '\r' && c != ESCAPE))
                --m_steps_to_take;

            if (m_pause_at && m_pause_at == m_replay_steps_remaining)
            {
                m_paused = true;
                m_steps_to_take = 0;
            }
        }
    }

    if (sleep)
        delay(sleep);

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
    if (m_sizer.ConsumeEvent(e))
        return;

    if (m_replay_steps_remaining > 0) {
        if (e.key.keysym.sym == SDLK_RETURN || e.key.keysym.sym == SDLK_ESCAPE) {
            HandleInputReplay(e.key.keysym.sym);
        }
        return;
    }

    if (e.key.keysym.sym == 's' && (e.key.keysym.mod & KMOD_CTRL)) {
        SaveGame("", true);
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
        PostRenderMsg(1);
        return;
    }

    if (m_replay_steps_remaining > 0)
    {
        HandleInputReplay(ch);
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

void SdlRogue::Impl::HandleInputReplay(int ch)
{
    // Pause playback
    if (ch == SDLK_SPACE) {
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
    // Resume playback
    else if (ch == SDLK_RETURN) {
        std::lock_guard<std::mutex> lock(m_input_mutex);
        m_paused = false;
        m_steps_to_take = 0;
        m_input_cv.notify_all();
    }
    // Cancel playback
    else if (ch == SDLK_ESCAPE) {
        std::lock_guard<std::mutex> lock(m_input_mutex);
        m_buffer.clear();
        m_replay_steps_remaining = 0;
        m_paused = false;
        m_steps_to_take = 0;
        m_input_cv.notify_all();
        PostRenderMsg(1);
    }
    // Decrease playback speed
    else if (ch == '-') {
        std::lock_guard<std::mutex> lock(m_input_mutex);
        m_replay_sleep *= 2;
        if (m_replay_sleep == 0)
            m_replay_sleep = 20;
    }
    // Increase playback speed
    else if (ch == '+') {
        std::lock_guard<std::mutex> lock(m_input_mutex);
        m_replay_sleep /= 2;
    }
    else if (ch == '0') {
        std::lock_guard<std::mutex> lock(m_input_mutex);
        m_replay_sleep = 0;
        m_paused = false;
        m_steps_to_take = 0;
        m_input_cv.notify_all();
    }
    else if (ch >= '1' && ch <= '9') {
        std::lock_guard<std::mutex> lock(m_input_mutex);
        m_replay_sleep = (ch - '0') * 15;
        m_paused = false;
        m_steps_to_take = 0;
        m_input_cv.notify_all();
    }
}

