#include <iterator>
#include "sdl_input.h"
#include "sdl_display.h"
#include "sdl_utility.h"
#include "key_utility.h"

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
    ReplayableInput(current_env, game_env, options)
{ }

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
    if (!Options().is_unix && (modifiers & KMOD_ALT) && keycode == SDLK_F9) {
        keycode = 'F';
        use = true;
    }

    auto i = m_keymap.find(keycode);
    if (i != m_keymap.end()) {
        keycode = i->second;
        use = true;
    }

    if (IsDirectionKey(keycode))
        return GetDirectionKey(keycode, modifiers, Options().emulate_ctrl_controls, keycode == original);

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
    if (InReplay()) {
        if (e.key.keysym.sym == SDLK_RETURN || e.key.keysym.sym == SDLK_ESCAPE) {
            HandleInputReplay(e.key.keysym.sym);
        }
        return true;
    }

    std::string new_input = TranslateKey(e.key.keysym.sym, e.key.keysym.mod);
    if (!new_input.empty())
    {
        QueueInput(new_input);
    }

    return true;
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

    if (InReplay())
    {
        HandleInputReplay(ch);
        return true;
    }

    std::string new_input;
    new_input.push_back(ch);
    QueueInput(new_input);
    return true;
}

void SdlInput::HandleInputReplay(int ch)
{
    if (ch == SDLK_SPACE) {
        PauseReplay();
    }
    else if (ch == SDLK_RETURN) {
        ResumeReplay();
    }
    else if (ch == SDLK_ESCAPE) {
        CancelReplay();
        SdlDisplay::PostRenderMsg(1);
    }
    else if (ch == '-') {
        DecreaseReplaySpeed();
    }
    else if (ch == '+') {
        IncreaseReplaySpeed();
    }
    else if (ch == '0') {
        SetMaxReplaySpeed();
    }
    else if (ch >= '1' && ch <= '9') {
        SetReplaySpeed(ch - '0');
    }
}

