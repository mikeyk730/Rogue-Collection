#pragma once
#include <vector>
#include <SDL.h>
#include "sdl_rogue.h"
#include "utility.h"

struct GameSelect
{
    GameSelect(SDL_Window* window, SDL_Renderer* renderer, const std::vector<Options>& options);
    int GetSelection();
    void RenderOption(int i, bool is_selected);

private:
    SDL_Window* m_window;
    SDL_Renderer* m_renderer;
    SDL::Scoped::Font m_font;
    const std::vector<Options>& m_options;
};