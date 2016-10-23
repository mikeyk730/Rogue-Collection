#pragma once
#include <SDL.h>

struct Environment;

struct WindowSizer
{
    WindowSizer(SDL_Window* window, SDL_Renderer* renderer, Environment* current_env);
    bool ConsumeEvent(const SDL_Event& e);

    void SetWindowSize(int w, int h);
private:
    void SetWindowSize(int w, int h, int scale);
    void ScaleWindow(int scale);

    SDL_Window* m_window;
    SDL_Renderer* m_renderer;
    Environment* m_current_env;
};
