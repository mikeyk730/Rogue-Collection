#include <algorithm>
#include <sstream>
#include "game_select.h"
#include "environment.h"
#include "utility.h"

GameSelect::GameSelect(SDL_Window * window, SDL_Renderer* renderer, const std::vector<Options>& options, Environment* current_env) :
    m_window(window), 
    m_renderer(renderer),
    m_options(options), 
    m_font(load_font(getResourcePath("fonts")+"Px437_IBM_BIOS.ttf", 16)),
    m_logo(loadImage(getResourcePath("")+"title3.png", renderer)),
    m_current_env(current_env)
{
    m_current_env->get("savefile", &m_replay_path);
    SDL_ShowWindow(m_window);
}

bool GameSelect::Select(std::pair<int, std::string>& p)
{
    SDL_PumpEvents();
    SDL_FlushEvents(SDL_FIRSTEVENT, SDL_LASTEVENT);

    if (p.first < int(m_options.size()))
        return true;

    if (!m_replay_path.empty() || GetLoadPath(m_window, m_replay_path)) {
        p = std::make_pair(-1, m_replay_path);
        return true;
    }

    return false;
}

void GameSelect::Render(int selection)
{
    SDL_RenderClear(m_renderer);
    SDL_RenderCopy(m_renderer, m_logo.get(), 0, 0);
    RenderText("Choose your Rogue:", { 17, 235 }, false);
    int i;
    for (i = 0; i < (int)m_options.size(); ++i)
        RenderOption(i, m_options[i].name, i == selection);
    RenderOption(i, "Restore Game", i == selection);
    SDL_RenderPresent(m_renderer);
}

void GameSelect::set_window_size(int w, int h, int scale)
{
    Coord window_size = ::get_scaled_coord({ w, h }, scale);
    window_size.x = std::max(window_size.x, 342); //mdk: 342 empirically determined to be min window width
    SDL_SetWindowSize(m_window, window_size.x, window_size.y);
    SDL_RenderSetLogicalSize(m_renderer, w, h);
}

void GameSelect::scale_window(int scale)
{
    std::ostringstream ss;
    ss << scale;
    m_current_env->set("window_scaling", ss.str());

    int w, h;
    SDL_RenderGetLogicalSize(m_renderer, &w, &h);
    set_window_size(w, h, scale);
}

std::pair<int, std::string> GameSelect::GetSelection()
{
    std::pair<int, std::string> selection = { 0, "" };
    SDL_Event e;
    while (SDL_WaitEvent(&e)) {
        if (e.type == SDL_QUIT) {
            return std::make_pair(-1, "");
        }
        else if (e.type == SDL_KEYDOWN) {
            if (e.key.keysym.mod & KMOD_ALT) {
                if (e.key.keysym.sym == SDLK_RETURN) {
                    ToggleFullscreen(m_window);
                }
                else if (e.key.keysym.sym >= SDLK_1 && e.key.keysym.sym <= SDLK_9)
                {
                    scale_window(e.key.keysym.sym - SDLK_0);
                }
                else if (e.key.keysym.sym == SDLK_0)
                {
                    scale_window(INT_MAX);
                }
            }
            else if (e.key.keysym.sym == SDLK_RETURN) {
                if (Select(selection))
                    return selection;
            }
            else if (e.key.keysym.sym == SDLK_UP) {
                if (selection.first > 0)
                    --selection.first;
                else
                    selection.first = (int)m_options.size();
            }
            else if (e.key.keysym.sym == SDLK_DOWN) {
                if (selection.first < int(m_options.size()))
                    ++selection.first;
                else
                    selection.first = 0;
            }
            else if (e.key.keysym.sym >= 'a' && e.key.keysym.sym < int('a' + m_options.size() + 1))
            {
                selection.first = (e.key.keysym.sym - 'a');
                if (Select(selection))
                    return selection;
            }
        }
        Render(selection.first);
    }
    throw;
}

void GameSelect::RenderOption(int i, const std::string& text, bool is_selected)
{
    std::string title = std::string(1, i+'a') + ") " + text;
    RenderText(title, { 34, 235 + 22 * (i + 1) }, is_selected);
}

void GameSelect::RenderText(const std::string& text, Coord p, bool highlight)
{
    SDL_Color color = SDL::Colors::white();
    if (highlight)
    {
        color = SDL::Colors::orange();
    }

    auto surface = load_text(text, m_font.get(), color, SDL::Colors::black());
    auto texture = create_texture(surface.get(), m_renderer);

    SDL_Rect r;
    r.x = p.x;
    r.y = p.y;
    r.h = surface->h;
    r.w = surface->w;

    SDL_RenderCopy(m_renderer, texture.get(), 0, &r);
}
