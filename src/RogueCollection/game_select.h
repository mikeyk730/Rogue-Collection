#pragma once
#include <string>
#include <vector>
#include <SDL.h>
#include "sdl_rogue.h"
#include "utility.h"

struct GameSelect
{
    GameSelect(SDL_Window* window, SDL_Renderer* renderer, const std::vector<Options>& options);
    std::pair<int, std::string> GetSelection();
    void RenderOption(int i, const std::string& text, bool is_selected);
    void RenderText(const std::string& text, Coord p, bool highlight);

private:
    bool Select(std::pair<int, std::string>& p);

    SDL_Window* m_window;
    SDL_Renderer* m_renderer;
    SDL::Scoped::Font m_font;
    SDL::Scoped::Texture m_logo;
    const std::vector<Options>& m_options;
    std::string m_replay_path;
};