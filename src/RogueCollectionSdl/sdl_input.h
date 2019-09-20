#pragma once
#include <map>
#include <SDL.h>
#include "replayable_input.h"

struct SdlInput : public ReplayableInput
{
    SdlInput(Environment* current_env, Environment* game_env, const GameConfig& options);
    virtual bool HandleEvent(const SDL_Event& e) override;

private:
    bool HandleEventText(const SDL_Event& e);
    bool HandleEventKeyDown(const SDL_Event& e);
    void HandleInputReplay(int ch);

    SDL_Keycode TranslateNumPad(SDL_Keycode keycode, uint16_t modifiers);
    std::string TranslateKey(SDL_Keycode keycode, uint16_t modifiers);

    static std::map<SDL_Keycode, SDL_Keycode> m_numpad;
    static std::map<SDL_Keycode, unsigned char> m_keymap;
};
