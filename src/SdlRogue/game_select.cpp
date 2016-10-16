#include "game_select.h"
#include "utility.h"

GameSelect::GameSelect(SDL_Window * window, SDL_Renderer* renderer, const std::vector<Options>& options) :
    m_window(window), 
    m_renderer(renderer),
    m_options(options), 
    m_font(load_font("C:\\Users\\mikeyk730\\src\\rogue\\Game-Rogue\\res\\fonts\\Vera.ttf", 20))
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
        }

        SDL_RenderClear(m_renderer);
        for (size_t i = 0; i < m_options.size(); ++i)
            RenderOption(i, i == selection);
        SDL_RenderPresent(m_renderer);
    }
    throw;
}

void GameSelect::RenderOption(int i, bool is_selected)
{
    std::string title = m_options[i].name;
    SDL_Color color = SDL::Colors::grey();
    if (is_selected)
    {
        color = SDL::Colors::brown();
    }

    auto surface = load_text(title, m_font.get(), color, SDL::Colors::black());
    auto texture = create_texture(surface.get(), m_renderer);
    SDL_Rect r;
    r.x = 50;
    r.y = (surface->h + 20) * (i+1);
    r.h = surface->h;
    r.w = surface->w;
    SDL_RenderCopy(m_renderer, texture.get(), 0, &r);
}
