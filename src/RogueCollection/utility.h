#pragma once
#include <memory>
#include <string>
#include <coord.h>
#include <SDL.h>
#include <SDL_ttf.h>

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

    namespace Colors
    {
        SDL_Color black();
        SDL_Color white();
        SDL_Color grey();
        SDL_Color d_grey();
        SDL_Color l_grey();
        SDL_Color red();
        SDL_Color l_red();
        SDL_Color green();
        SDL_Color l_green();
        SDL_Color blue();
        SDL_Color l_blue();
        SDL_Color cyan();
        SDL_Color l_cyan();
        SDL_Color magenta();
        SDL_Color l_magenta();
        SDL_Color yellow();
        SDL_Color brown();
        SDL_Color orange();
    }
}

int GetScalingFactor(Coord logical_size);
Coord GetScaledCoord(Coord logical_size, int scale_factor);

void throw_error(const std::string &msg);

/*
* Get the resource path for resources located in res/subDir
* It's assumed the project directory is structured like:
* bin/
*  the executable
* res/
*  Lesson1/
*  Lesson2/
*
* Paths returned will be Lessons/res/subDir
*/
std::string GetResourcePath(const std::string &subDir = "");

//todo: encapsulate into class
uint32_t GetPixel(SDL_Surface *surface, int x, int y);
void PutPixel(SDL_Surface *surface, int x, int y, uint32_t pixel);
void PaintSurface(SDL_Surface* surface, SDL_Color fg, SDL_Color bg);
SDL::Scoped::Surface BlitSurface(SDL_Surface* surface, SDL_Rect* r = 0);
SDL::Scoped::Texture PaintedTexture(SDL_Surface* surface, SDL_Rect* r, SDL_Color fg, SDL_Color bg, SDL_Renderer* renderer);

SDL::Scoped::Texture LoadImage(const std::string &file, SDL_Renderer *ren);
SDL::Scoped::Font LoadFont(const std::string& filename, int size);
SDL::Scoped::Surface LoadBmp(const std::string& filename);
SDL::Scoped::Texture CreateTexture(SDL_Surface* surface, SDL_Renderer* renderer);
SDL::Scoped::Surface LoadText(const std::string& s, _TTF_Font* font, SDL_Color color, SDL_Color bg);

bool IsFullscreen(SDL_Window* Window);
void SetFullscreen(SDL_Window* Window, bool enable);
void ToggleFullscreen(SDL_Window* Window);

bool GetLoadPath(SDL_Window* window, std::string& path);
bool GetSavePath(SDL_Window* window, std::string& path);

void DisplayMessage(int type, const std::string& title, const std::string& msg);
void Delay(int ms);

template <typename T>
std::ostream& Write(std::ostream& out, T t) {
    out.write((char*)&t, sizeof(T));
    return out;
}

template <typename T>
std::istream& Read(std::istream& in, T* t) {
    in.read((char*)t, sizeof(T));
    return in;
}

std::ostream& WriteShortString(std::ostream& out, const std::string& s);
std::istream& ReadShortString(std::istream& in, std::string* s);
