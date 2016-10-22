#pragma once
#include <string>
#include <vector>
#include <SDL.h>
#include "sdl_rogue.h"
#include "utility.h"

struct GameSelect
{
    GameSelect(SDL_Window* window, SDL_Renderer* renderer, const std::vector<Options>& options, Environment* current_env);
    std::pair<int, std::string> GetSelection();
    void RenderOption(int i, const std::string& text, bool is_selected);
    void RenderText(const std::string& text, Coord p, bool highlight);

private:
    bool MakeSelection();
    void RenderMainMenu();
    void RenderTitleScreen();

    void set_window_size(int w, int h, int scale);
    void scale_window(int scale);

    bool HandleMainMenu(const SDL_Event& e);
    bool HandleTitleScreen(const SDL_Event& e);

    SDL_Window* m_window;
    SDL_Renderer* m_renderer;
    SDL::Scoped::Font m_font;
    SDL::Scoped::Texture m_logo;
    SDL::Scoped::Texture m_title_screen;
    const std::vector<Options>& m_options;
    std::string m_replay_path;
    int m_selection = 0;
    Environment* m_current_env;
    bool m_in_main_menu = true;
    bool m_show_title = true;
};