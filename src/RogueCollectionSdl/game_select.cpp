#include <algorithm>
#include <sstream>
#include "game_select.h"
#include "environment.h"
#include "sdl_utility.h"

GameSelect::GameSelect(SDL_Window * window, SDL_Renderer* renderer, const std::vector<GameConfig>& options, Environment* current_env) :
    m_window(window),
    m_renderer(renderer),
    m_font(LoadFont(GetResourcePath("fonts")+"Px437_IBM_BIOS.ttf", 16)),
    m_logo(LoadImage(GetResourcePath("") + "title3.png", renderer)),
    m_options(options),
    m_current_env(current_env),
    m_sizer(window, renderer, current_env)
{
    m_current_env->Get("savefile", &m_replay_path);
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
    RenderText("Choose your Rogue:", { 17, 218 }, false);
    int i;
    for (i = 0; i < (int)m_options.size(); ++i)
        RenderOption(i, m_options[i].name, i == m_selection);
    RenderOption(i, "Restore Game", i == m_selection);
    SDL_RenderPresent(m_renderer);
}

void TitleScreen::RenderTitleScreen()
{
    SDL_RenderClear(m_renderer);
    SDL_RenderCopy(m_renderer, m_title_screen.get(), 0, 0);
    SDL_RenderPresent(m_renderer);
}


bool GameSelect::FinishMainMenu(const SDL_Event& e)
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

bool TitleScreen::HandleTitleScreen(const SDL_Event & e)
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
        if (m_sizer.ConsumeEvent(e))
            continue;
        if (FinishMainMenu(e)) {
            return std::make_pair(m_selection, m_replay_path);
        }
    }
    throw;
}

void GameSelect::RenderOption(int i, const std::string& text, bool is_selected)
{
    std::string title = std::string(1, i+'a') + ") " + text;
    RenderText(title, { 34, 218 + 22 * (i + 1) }, is_selected);
}

void GameSelect::RenderText(const std::string& text, Coord p, bool highlight)
{
    SDL_Color color = SDL::Colors::white();
    if (highlight)
    {
        color = SDL::Colors::orange();
    }

    auto surface = LoadText(text, m_font.get(), color, SDL::Colors::black());
    auto texture = CreateTexture(surface.get(), m_renderer);

    SDL_Rect r;
    r.x = p.x;
    r.y = p.y;
    r.h = surface->h;
    r.w = surface->w;

    SDL_RenderCopy(m_renderer, texture.get(), 0, &r);
}

TitleScreen::TitleScreen(SDL_Window * window, SDL_Renderer * renderer, Environment * current_env) :
    m_window(window),
    m_renderer(renderer),
    m_title_screen(LoadImage(GetResourcePath("") + "epyx.png", renderer)),
    m_sizer(window, renderer, current_env)
{
}

bool TitleScreen::Run()
{
    SDL_Event e;
    while (SDL_WaitEvent(&e)) {
        if (e.type == SDL_QUIT) {
            return false;
        }

        if (m_sizer.ConsumeEvent(e))
            continue;

        if (HandleTitleScreen(e)) {
            return true;
        }
    }
    throw;
}
