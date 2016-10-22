#pragma once

struct Environment;
struct SDL_Window;
struct SDL_Renderer;
union SDL_Event;

struct WindowSizer
{
    WindowSizer(SDL_Window* window, SDL_Renderer* renderer, Environment* current_env);
    bool ConsumeEvent(const SDL_Event& e);

    void SetWindowSize(int w, int h);
private:
    void set_window_size(int w, int h, int scale);
    void scale_window(int scale);

    SDL_Window* m_window;
    SDL_Renderer* m_renderer;
    Environment* m_current_env;
};
