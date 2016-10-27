#include <iterator>
#include <sstream>
#include "sdl_input.h"
#include "sdl_display.h"
#include "sdl_utility.h"
#include "environment.h"
#include "utility.h"

#define CTRL(ch)   (ch&0x1f)
#define ESCAPE     0x1b

std::map<SDL_Keycode, SDL_Keycode> SdlInput::m_numpad = {
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

std::map<SDL_Keycode, unsigned char> SdlInput::m_keymap = {
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


SdlInput::SdlInput(Environment* current_env, Environment* game_env, const GameConfig& options) :
    m_current_env(current_env),
    m_game_env(game_env),
    m_options(options)
{
}

char SdlInput::GetChar(bool block, bool for_string, bool *is_replay)
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
        Delay(sleep);

    return c;
}

void SdlInput::Flush()
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

    bool caps((modifiers & KMOD_CAPS) != 0);
    bool shift((modifiers & KMOD_SHIFT) != 0);
    if (caps ^ shift) {
        keybuf.push_back(toupper(keycode));
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
SDL_Keycode SdlInput::TranslateNumPad(SDL_Keycode keycode, uint16_t modifiers)
{
    if ((modifiers & KMOD_NUM) == 0) {
        auto i = m_numpad.find(keycode);
        if (i != m_numpad.end())
        {
            return i->second;
        }
    }
    return keycode;
}

std::string SdlInput::TranslateKey(SDL_Keycode original, uint16_t modifiers)
{
    // Direction keys not modified by Ctrl or Alt are handled by HandleEventText
    if (IsDirectionKey(original) && (modifiers & KMOD_CTRL) == 0 && (modifiers & KMOD_ALT) == 0)
        return "";

    bool use = false;
    SDL_Keycode keycode = TranslateNumPad(original, modifiers);

    // Translate Alt+F9 on PC to 'F'
    if (!m_options.is_unix && (modifiers & KMOD_ALT) && keycode == SDLK_F9) {
        keycode = 'F';
        use = true;
    }

    auto i = m_keymap.find(keycode);
    if (i != m_keymap.end()) {
        keycode = i->second;
        use = true;
    }

    if (IsDirectionKey(keycode))
        return GetDirectionKey(keycode, modifiers, m_options.emulate_ctrl_controls, keycode == original);

    if ((modifiers & KMOD_CTRL) && IsLetterKey(keycode)) {
        keycode = CTRL(keycode);
        use = true;
    }

    if (use) {
        return std::string(1, keycode);
    }
    return "";
}

bool SdlInput::HandleEventKeyDown(const SDL_Event & e)
{
    if (m_replay_steps_remaining > 0) {
        if (e.key.keysym.sym == SDLK_RETURN || e.key.keysym.sym == SDLK_ESCAPE) {
            HandleInputReplay(e.key.keysym.sym);
        }
        return true;
    }

    std::string new_input = TranslateKey(e.key.keysym.sym, e.key.keysym.mod);
    if (!new_input.empty())
    {
        std::lock_guard<std::mutex> lock(m_input_mutex);
        std::copy(new_input.begin(), new_input.end(), std::back_inserter(m_buffer));
        m_input_cv.notify_all();
    }

    return true;
}

void SdlInput::SaveGame(std::ostream & file)
{
    std::unique_lock<std::mutex> lock(m_input_mutex);
    std::copy(m_keylog.begin(), m_keylog.end(), std::ostreambuf_iterator<char>(file));
}

void SdlInput::RestoreGame(std::istream & file)
{
    m_buffer.assign(std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>());
    m_replay_steps_remaining = (int)m_buffer.size();

    std::string value;
    if (m_current_env->Get("replay_paused", &value) && value == "true") {
        m_paused = true;
    }
    if (m_current_env->Get("replay_pause_at", &value)) {
        m_pause_at = atoi(value.c_str());
    }
    if (m_current_env->Get("replay_step_delay", &value)) {
        m_replay_sleep = atoi(value.c_str());
    }
}

bool SdlInput::HandleEvent(const SDL_Event & e)
{
    if (e.type == SDL_KEYDOWN) {
        return HandleEventKeyDown(e);
    }
    else if (e.type == SDL_TEXTINPUT) {
        return HandleEventText(e);
    }
    return false;
}

bool SdlInput::HandleEventText(const SDL_Event & e)
{
    //todo: when does string have more than 1 char?
    char ch = e.text.text[0];

    if (m_replay_steps_remaining > 0)
    {
        HandleInputReplay(ch);
        return true;
    }

    std::string new_input;
    new_input.push_back(ch);

    std::lock_guard<std::mutex> lock(m_input_mutex);
    std::copy(new_input.begin(), new_input.end(), std::back_inserter(m_buffer));
    m_input_cv.notify_all();
    return true;
}

bool SdlInput::GetRenderText(std::string* text)
{
    std::lock_guard<std::mutex> lock(m_input_mutex);
    if (!m_replay_steps_remaining)
        return false;

    std::ostringstream ss;
    if (m_paused)
        ss << "Paused ";
    else
        ss << "Replay ";
    ss << m_replay_steps_remaining;
    *text = ss.str();
    return true;
}

void SdlInput::HandleInputReplay(int ch)
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
        SdlDisplay::PostRenderMsg(1);
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

