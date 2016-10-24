#pragma once
#include <mutex>
#include <deque>
#include <vector>
#include <map>
#include <SDL.h>
#include <input_interface.h>
#include "sdl_rogue.h"

struct Environment;

struct SdlInput : public InputInterface
{
public:
    SdlInput(Environment* current_env, Environment* game_env, const GameConfig& options);

    //input interface
    virtual char GetChar(bool block, bool for_string, bool *is_replay) override;
    virtual void Flush() override;

    void SaveGame(std::ostream& file);
    void RestoreGame(std::istream& file);

    bool HandleEvent(const SDL_Event& e);

    bool GetRenderText(std::string* text);

private:
    bool HandleEventText(const SDL_Event& e);
    bool HandleEventKeyDown(const SDL_Event& e);

    void HandleInputReplay(int ch);

    SDL_Keycode TranslateNumPad(SDL_Keycode keycode, uint16_t modifiers);
    std::string TranslateKey(SDL_Keycode keycode, uint16_t modifiers);

    Environment* m_current_env = 0;
    Environment* m_game_env = 0;
    GameConfig m_options;

    std::deque<unsigned char> m_buffer;
    std::vector<unsigned char> m_keylog;
    int m_replay_steps_remaining = 0;
    int m_steps_to_take = 0;
    int m_replay_sleep = 0;
    int m_pause_at = 0;
    bool m_paused = false;

    std::mutex m_input_mutex;
    std::condition_variable m_input_cv;

    static std::map<SDL_Keycode, SDL_Keycode> m_numpad;
    static std::map<SDL_Keycode, unsigned char> m_keymap;
};
