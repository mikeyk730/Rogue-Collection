#pragma once
#include <memory>
#include <string>

struct Coord;
struct SDL_Window;
struct SDL_Renderer;
struct SDL_Surface;
struct SDL_Texture;
struct SDL_Rect;
struct SDL_Color;
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
std::string getResourcePath(const std::string &subDir = "");

uint32_t getpixel(SDL_Surface *surface, int x, int y);
void putpixel(SDL_Surface *surface, int x, int y, uint32_t pixel);

SDL::Scoped::Texture loadImage(const std::string &file, SDL_Renderer *ren);
SDL::Scoped::Font load_font(const std::string& filename, int size);
SDL::Scoped::Surface load_bmp(const std::string& filename);
SDL::Scoped::Texture create_texture(SDL_Surface* surface, SDL_Renderer* renderer);
SDL::Scoped::Surface load_text(const std::string& s, _TTF_Font* font, SDL_Color color, SDL_Color bg);

bool GetLoadPath(std::string& path);
bool GetSavePath(std::string& path);

void ErrorBox(const std::string& msg);
void delay(int ms);

template <typename T>
std::ostream& write(std::ostream& out, T t) {
    out.write((char*)&t, sizeof(T));
    return out;
}

template <typename T>
std::istream& read(std::istream& in, T* t) {
    in.read((char*)t, sizeof(T));
    return in;
}

std::ostream& write_short_string(std::ostream& out, const std::string& s);
std::istream& read_short_string(std::istream& in, std::string* s);
