#include "game_select.h"
#include "utility.h"

GameSelect::GameSelect(SDL_Window * window, SDL_Renderer* renderer, const std::vector<Options>& options) :
    m_window(window), 
    m_renderer(renderer),
    m_options(options), 
    m_font(load_font(getResourcePath("fonts")+"Px437_IBM_BIOS.ttf", 12)),
    m_logo(loadImage(getResourcePath("")+"title.png", renderer))
{
}

int GameSelect::GetSelection()
{
    int selection = 0;
    SDL_Event e;
    while (SDL_WaitEvent(&e)) {
        if (e.type == SDL_QUIT) {
            return -1;
        }
        else if (e.type == SDL_KEYDOWN) {
            if (e.key.keysym.sym == SDLK_RETURN) {
                SDL_PumpEvents();
                SDL_FlushEvents(SDL_FIRSTEVENT, SDL_LASTEVENT);
                return selection;
            }
            else if (e.key.keysym.sym == SDLK_UP) {
                if (selection > 0)
                    --selection;
            }
            else if (e.key.keysym.sym == SDLK_DOWN) {
                if (selection < int(m_options.size()-1))
                    ++selection;
            }
            else if (e.key.keysym.sym >= 'a' && e.key.keysym.sym < int('a' + m_options.size()))
            {
                SDL_PumpEvents();
                SDL_FlushEvents(SDL_FIRSTEVENT, SDL_LASTEVENT);
                return e.key.keysym.sym - 'a';
            }
        }

        SDL_RenderClear(m_renderer);
        SDL_Rect r;
        SDL_QueryTexture(m_logo.get(), 0, 0, &r.w, &r.h);
        r.x = (640 - r.w)/2;
        r.y = 10;
 
        SDL_RenderCopy(m_renderer, m_logo.get(), 0, 0);
        //RenderText("Rogue Collection v1.0", { 25, 25 }, false);
        RenderText("Choose your Rogue:", { 33, 275 }, false);
        for (size_t i = 0; i < m_options.size(); ++i)
            RenderOption(i, i == selection);
        SDL_RenderPresent(m_renderer);
    }
    throw;
}

void GameSelect::RenderOption(int i, bool is_selected)
{
    std::string title = std::string(1, i+'a') + ") " + m_options[i].name;
    RenderText(title, { 58, 275 + 20 * (i + 1) }, is_selected);
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
