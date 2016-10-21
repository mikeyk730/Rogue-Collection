#include "game_select.h"
#include "utility.h"

GameSelect::GameSelect(SDL_Window * window, SDL_Renderer* renderer, const std::vector<Options>& options) :
    m_window(window), 
    m_renderer(renderer),
    m_options(options), 
    m_font(load_font(getResourcePath("fonts")+"Px437_IBM_BIOS.ttf", 16)),
    m_logo(loadImage(getResourcePath("")+"title3.png", renderer))
{
    SDL_ShowWindow(m_window);
}

bool GameSelect::Select(std::pair<int, std::string>& p)
{
    SDL_PumpEvents();
    SDL_FlushEvents(SDL_FIRSTEVENT, SDL_LASTEVENT);

    if (p.first < int(m_options.size()))
        return true;

    if (GetLoadPath(m_window, m_replay_path)) {
        p = std::make_pair(-1, m_replay_path);
        return true;
    }

    return false;
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
            if (e.key.keysym.sym == SDLK_RETURN && (e.key.keysym.mod & KMOD_ALT)) {
                ToggleFullscreen(m_window);
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

        SDL_RenderClear(m_renderer);
        SDL_RenderCopy(m_renderer, m_logo.get(), 0, 0);
        RenderText("Choose your Rogue:", { 17, 235 }, false);
        int i;
        for (i = 0; i < (int)m_options.size(); ++i)
            RenderOption(i, m_options[i].name, i == selection.first);
        RenderOption(i, "Restore Game", i == selection.first);
        SDL_RenderPresent(m_renderer);
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
