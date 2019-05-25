#include <algorithm>
#include <sstream>
#include <climits>
#include <SDL.h>
#include <coord.h>
#include "window_sizer.h"
#include "environment.h"

namespace SDL
{
    const int FULLSCREEN_FLAG = SDL_WINDOW_FULLSCREEN_DESKTOP;
}

WindowSizer::WindowSizer(SDL_Window * window, SDL_Renderer * renderer, Environment * current_env) :
    m_window(window),
    m_renderer(renderer),
    m_current_env(current_env)
{
}

bool WindowSizer::ConsumeEvent(const SDL_Event & e)
{
    if (e.type == SDL_KEYDOWN) {
        if (e.key.keysym.mod & KMOD_ALT) {
            if (e.key.keysym.sym == SDLK_RETURN) {
                ToggleFullscreen(m_window);
                return true;
            }
            else if (e.key.keysym.sym >= SDLK_1 && e.key.keysym.sym <= SDLK_9)
            {
                ScaleWindow(e.key.keysym.sym - SDLK_0);
                SetFullscreen(m_window, false);
                return true;
            }
            else if (e.key.keysym.sym == SDLK_0)
            {
                ScaleWindow(INT_MAX);
                SetFullscreen(m_window, false);
                return true;
            }
        }
    }
    return false;
}

void WindowSizer::SetWindowSize(int w, int h)
{
    int scale = INT_MAX;
    std::string value;
    if (m_current_env->Get("window_scaling", &value))
    {
        scale = atoi(value.c_str());
    }
    SetWindowSize(w, h, scale);
}

void WindowSizer::SetWindowSize(int w, int h, int scale)
{
    Coord window_size = ::GetScaledCoord({ w, h }, scale);
    window_size.x = std::max(window_size.x, 342); //mdk: 342 empirically determined to be min window width
    SDL_SetWindowSize(m_window, window_size.x, window_size.y);
    SDL_RenderSetLogicalSize(m_renderer, w, h);
}

void WindowSizer::ScaleWindow(int scale)
{
    std::ostringstream ss;
    ss << scale;
    m_current_env->Set("window_scaling", ss.str());

    int w, h;
    SDL_RenderGetLogicalSize(m_renderer, &w, &h);
    SetWindowSize(w, h, scale);
}

int GetScalingFactor(Coord logical_size)
{
    SDL_DisplayMode mode;
    if (SDL_GetCurrentDisplayMode(0, &mode) == 0) {
        int xscale = mode.w / logical_size.x;
        int yscale = mode.h / logical_size.y;

        return std::min(xscale, yscale);
    }
    return 2;
}

Coord GetScaledCoord(Coord logical_size, int scale_factor)
{
    int max_factor = GetScalingFactor(logical_size);
    scale_factor = std::min(scale_factor, max_factor);
    return{ logical_size.x * scale_factor, logical_size.y * scale_factor };
}

bool IsFullscreen(SDL_Window* w)
{
    return (SDL_GetWindowFlags(w) & SDL::FULLSCREEN_FLAG) != 0;
}

void SetFullscreen(SDL_Window* w, bool enable)
{
    SDL_SetWindowFullscreen(w, enable ? SDL::FULLSCREEN_FLAG : 0);
}

void ToggleFullscreen(SDL_Window* w)
{
    SetFullscreen(w, !IsFullscreen(w));
}
