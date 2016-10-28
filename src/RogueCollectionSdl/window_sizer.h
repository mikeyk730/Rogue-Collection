#pragma once
#include <SDL.h>

struct Environment;
struct Coord;

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

int GetScalingFactor(Coord logical_size);
Coord GetScaledCoord(Coord logical_size, int scale_factor);

bool IsFullscreen(SDL_Window* Window);
void SetFullscreen(SDL_Window* Window, bool enable);
void ToggleFullscreen(SDL_Window* Window);
