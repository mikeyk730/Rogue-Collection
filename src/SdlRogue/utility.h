#pragma once
#include <memory>
#include <string>

struct Coord;
struct SDL_Window;
struct SDL_Renderer;
struct SDL_Surface;
struct SDL_Texture;
struct SDL_Rect;
struct _TTF_Font;

namespace SDL
{
    namespace Scoped
    {
        typedef std::unique_ptr<SDL_Window, void(*)(SDL_Window*)> Window;
        typedef std::unique_ptr<SDL_Renderer, void(*)(SDL_Renderer*)> Renderer;
        typedef std::unique_ptr<SDL_Surface, void(*)(SDL_Surface*)> Surface;
        typedef std::unique_ptr<SDL_Texture, void(*)(SDL_Texture*)> Texture;
        typedef std::unique_ptr<_TTF_Font, void(*)(_TTF_Font*)> Font;
    }
}

void throw_error(const std::string &msg);

SDL::Scoped::Surface load_bmp(const std::string& filename);
SDL::Scoped::Texture create_texture(SDL_Surface* surface, SDL_Renderer* renderer);
void render_texture_at(SDL_Texture* texture, SDL_Renderer* renderer, Coord position, SDL_Rect* clip);
void render_texture_at(SDL_Texture* texture, SDL_Renderer* renderer, Coord position, SDL_Rect clip);
