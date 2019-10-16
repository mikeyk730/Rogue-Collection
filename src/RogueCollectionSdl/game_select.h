#pragma once
#include <string>
#include <vector>
#include <SDL.h>
#include "sdl_rogue.h"
#include "sdl_utility.h"
#include "window_sizer.h"

struct TitleScreen
{
    TitleScreen(SDL_Window* window, SDL_Renderer* renderer, Environment* current_env);
    bool Run();

private:
    void RenderTitleScreen();
    bool HandleTitleScreen(const SDL_Event& e);

    SDL_Window* m_window;
    SDL_Renderer* m_renderer;
    SDL::Scoped::Texture m_title_screen;
    WindowSizer m_sizer;
};

struct GameSelect
{
    GameSelect(SDL_Window* window, SDL_Renderer* renderer, const std::vector<GameConfig>& options, Environment* current_env);
    std::pair<int, std::string> GetSelection();
    void RenderOption(int i, const std::string& text, bool is_selected);
    void RenderText(const std::string& text, Coord p, bool highlight);

private:
    bool MakeSelection();
    void RenderMainMenu();
    bool FinishMainMenu(const SDL_Event& e);

    SDL_Window* m_window;
    SDL_Renderer* m_renderer;
    SDL::Scoped::Font m_font;
    SDL::Scoped::Texture m_logo;
    const std::vector<GameConfig>& m_options;
    std::string m_replay_path;
    int m_selection = 0;
    Environment* m_current_env;
    WindowSizer m_sizer;
};