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
    m_logo(loadImage(getResourcePath("") + "title3.png", renderer)),
    m_title_screen(loadImage(getResourcePath("") + "epyx.png", renderer)),
    m_current_env(current_env)
{
    m_current_env->get("savefile", &m_replay_path);
    std::string value;
    if (m_current_env->get("show_title_screen", &value) && value == "false")
    {
        m_show_title = false;
    }
    SDL_ShowWindow(m_window);
}

bool GameSelect::MakeSelection()
{
    SDL_PumpEvents();
    SDL_FlushEvents(SDL_FIRSTEVENT, SDL_LASTEVENT);

    if (m_selection < int(m_options.size()))
        return true;

    if (!m_replay_path.empty() || GetLoadPath(m_window, m_replay_path)) {
        m_selection = -1;
        return true;
    }

    return false;
}

void GameSelect::RenderMainMenu()
{
    SDL_RenderClear(m_renderer);
    SDL_RenderCopy(m_renderer, m_logo.get(), 0, 0);
    RenderText("Choose your Rogue:", { 17, 235 }, false);
    int i;
    for (i = 0; i < (int)m_options.size(); ++i)
        RenderOption(i, m_options[i].name, i == m_selection);
    RenderOption(i, "Restore Game", i == m_selection);
    SDL_RenderPresent(m_renderer);
}

void GameSelect::RenderTitleScreen()
{
    SDL_RenderClear(m_renderer);
    SDL_RenderCopy(m_renderer, m_title_screen.get(), 0, 0);
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

bool GameSelect::HandleMainMenu(const SDL_Event& e)
{
    if (e.type == SDL_KEYDOWN) {
        if (e.key.keysym.sym == SDLK_RETURN) {
            if (MakeSelection())
                return true;
        }
        else if (e.key.keysym.sym == SDLK_UP) {
            if (m_selection > 0)
                --m_selection;
            else
                m_selection = (int)m_options.size();
        }
        else if (e.key.keysym.sym == SDLK_DOWN) {
            if (m_selection < int(m_options.size()))
                ++m_selection;
            else
                m_selection = 0;
        }
        else if (e.key.keysym.sym >= 'a' && e.key.keysym.sym < int('a' + m_options.size() + 1))
        {
            m_selection = (e.key.keysym.sym - 'a');
            if (MakeSelection())
                return true;
        }
    }
    RenderMainMenu();
    return false;
}

bool GameSelect::HandleTitleScreen(const SDL_Event & e)
{
    if (e.type == SDL_KEYDOWN) {
        if (e.key.keysym.sym == SDLK_RALT || e.key.keysym.sym == SDLK_LALT || e.key.keysym.sym == SDLK_RCTRL ||
            e.key.keysym.sym == SDLK_LCTRL || e.key.keysym.sym == SDLK_LSHIFT || e.key.keysym.sym == SDLK_RSHIFT) {
            return false;
        }
        return true;
    }
    RenderTitleScreen();
    return false;
}

std::pair<int, std::string> GameSelect::GetSelection()
{
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
                    SetFullscreen(m_window, false);
                }
                else if (e.key.keysym.sym == SDLK_0)
                {
                    scale_window(INT_MAX);
                    SetFullscreen(m_window, false);
                }
                continue;
            }
        }

        if (m_in_main_menu) {
            if (HandleMainMenu(e)) {
                if (m_show_title && m_selection >= 0 && s_options[m_selection].name == "PC Rogue 1.48") {
                    m_in_main_menu = false;
                    RenderTitleScreen();
                }
                else {
                    return std::make_pair(m_selection, m_replay_path);
                }
            }
        }
        else if (HandleTitleScreen(e)) {
            return std::make_pair(m_selection, m_replay_path);
        }
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
