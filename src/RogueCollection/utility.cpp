#include <iostream>
#include <cassert>
#include <chrono>
#include <thread>
#include <algorithm>
#include <nfd.h>
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include "utility.h"
#include <coord.h>

namespace SDL
{
    const int FULLSCREEN_FLAG = SDL_WINDOW_FULLSCREEN_DESKTOP;
    //const int FULLSCREEN_FLAG = SDL_WINDOW_FULLSCREEN;

    namespace Colors
    {
        SDL_Color black()    { return {   0,   0,   0, 255 }; } //from dosbox
        SDL_Color white()    { return { 255, 255, 255, 255 }; } //from dosbox
        SDL_Color grey()     { return { 170, 170, 170, 255 }; } //from dosbox
        SDL_Color d_grey()   { return {  65,  65,  65, 255 }; }
        SDL_Color l_grey()   { return { 205, 205, 205, 255 }; }
        SDL_Color red()      { return { 170,   0,   0, 255 }; } //from dosbox
        SDL_Color l_red()    { return { 255,  85,  85, 255 }; }
        SDL_Color green()    { return {   0, 170,   0, 255 }; } //from dosbox
        SDL_Color l_green()  { return {  85,  255, 85, 255 }; } //from dosbox
        SDL_Color blue()     { return {   0,   0, 170, 255 }; } //from dosbox
        SDL_Color l_blue()   { return {  85,  85, 255, 255 }; } //from dosbox
        SDL_Color cyan()     { return {   0, 170, 170, 255 }; }
        SDL_Color l_cyan()   { return {  25, 255, 255, 255 }; }
        SDL_Color magenta()  { return { 170,   0, 170, 255 }; } //from dosbox
        SDL_Color l_magenta(){ return { 255,  25, 255, 255 }; } 
        SDL_Color yellow()   { return { 255, 255,  25, 255 }; } //from dosbox
        SDL_Color brown()    { return { 170,  85,   0, 255 }; } //from dosbox
        SDL_Color orange()   { return { 234, 118,   2, 255 }; } 
    }
}

bool GetLoadPath(std::string& path)
{
    nfdchar_t *p = NULL;
    nfdresult_t result = NFD_OpenDialog("sav", NULL, &p);
    if (result == NFD_OKAY)
    {
        path = p;
        free(p);
        return true;
    }
    return false;
}

bool GetLoadPath(SDL_Window* window, std::string& path) 
{
    bool fullscreen = IsFullscreen(window);
    SetFullscreen(window, false);
    bool success = GetLoadPath(path);
    SetFullscreen(window, fullscreen);
    return success;
}

bool GetSavePath(std::string& path)
{
    nfdchar_t *p = NULL;
    nfdresult_t result = NFD_SaveDialog("sav", NULL, &p);
    if (result == NFD_OKAY)
    {
        path = p;
        free(p);
        return true;
    }
    return false;
}

bool GetSavePath(SDL_Window* window, std::string& path) 
{
    bool fullscreen = IsFullscreen(window);
    SetFullscreen(window, false);
    bool success = GetSavePath(path);
    SetFullscreen(window, fullscreen);
    return success;
}

void ErrorBox(const std::string & msg)
{
    SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error", msg.c_str(), NULL);
}

void delay(int ms)
{
    std::this_thread::sleep_for(std::chrono::milliseconds(ms));
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

std::string getResourcePath(const std::string &subDir) {
    //We need to choose the path separator properly based on which
    //platform we're running on, since Windows uses a different
    //separator than most systems
#ifdef _WIN32
    const char PATH_SEP = '\\';
#else
    const char PATH_SEP = '/';
#endif
    //This will hold the base resource path: Lessons/res/
    //We give it static lifetime so that we'll only need to call
    //SDL_GetBasePath once to get the executable path
    static std::string baseRes;
    if (baseRes.empty()) {
        //SDL_GetBasePath will return NULL if something went wrong in retrieving the path
        char *basePath = SDL_GetBasePath();
        if (basePath) {
            baseRes = basePath;
            SDL_free(basePath);
        }
        else {
            std::cerr << "Error getting resource path: " << SDL_GetError() << std::endl;
            return "";
        }
        //We replace the last bin/ with res/ to get the the resource path
        //size_t pos = baseRes.rfind("bin");
        //baseRes = baseRes.substr(0, pos) + "res" + PATH_SEP;
        baseRes = baseRes + "res" + PATH_SEP;
    }
    //If we want a specific subdirectory path in the resource directory
    //append it to the base path. This would be something like Lessons/res/Lesson0
    return subDir.empty() ? baseRes : baseRes + subDir + PATH_SEP;
}


SDL::Scoped::Font load_font(const std::string& filename, int size)
{
    SDL::Scoped::Font font(TTF_OpenFont(filename.c_str(), size), TTF_CloseFont);
    if (font == nullptr)
        throw_error("TTF_OpenFont");

    return std::move(font);
}

SDL::Scoped::Surface load_text(const std::string& s, _TTF_Font* font, SDL_Color color, SDL_Color bg)
{
    SDL::Scoped::Surface surface(TTF_RenderText_Shaded(font, s.c_str(), color, bg), SDL_FreeSurface);
    if (surface == nullptr)
        throw_error("TTF_RenderText");

    return std::move(surface);
}

/*
* Return the pixel value at (x, y)
* NOTE: The surface must be locked before calling this!
*/
uint32_t getpixel(SDL_Surface *surface, int x, int y)
{
    int bpp = surface->format->BytesPerPixel;
    /* Here p is the address to the pixel we want to retrieve */
    Uint8 *p = (Uint8 *)surface->pixels + y * surface->pitch + x * bpp;

    switch (bpp) {
    case 1:
        return *p;

    case 2:
        return *(Uint16 *)p;

    case 3:
        if (SDL_BYTEORDER == SDL_BIG_ENDIAN)
            return p[0] << 16 | p[1] << 8 | p[2];
        else
            return p[0] | p[1] << 8 | p[2] << 16;

    case 4:
        return *(Uint32 *)p;

    default:
        return 0;       /* shouldn't happen, but avoids warnings */
    }
}

/*
* Set the pixel at (x, y) to the given value
* NOTE: The surface must be locked before calling this!
*/
void putpixel(SDL_Surface *surface, int x, int y, uint32_t pixel)
{
    int bpp = surface->format->BytesPerPixel;
    /* Here p is the address to the pixel we want to set */
    Uint8 *p = (Uint8 *)surface->pixels + y * surface->pitch + x * bpp;

    switch (bpp) {
    case 1:
        *p = pixel;
        break;

    case 2:
        *(Uint16 *)p = pixel;
        break;

    case 3:
        if (SDL_BYTEORDER == SDL_BIG_ENDIAN) {
            p[0] = (pixel >> 16) & 0xff;
            p[1] = (pixel >> 8) & 0xff;
            p[2] = pixel & 0xff;
        }
        else {
            p[0] = pixel & 0xff;
            p[1] = (pixel >> 8) & 0xff;
            p[2] = (pixel >> 16) & 0xff;
        }
        break;

    case 4:
        *(Uint32 *)p = pixel;
        break;
    }
}

int get_scaling_factor(Coord logical_size)
{
    SDL_DisplayMode mode;
    if (SDL_GetCurrentDisplayMode(0, &mode) == 0) {
        int xscale = mode.w / logical_size.x;
        int yscale = mode.h / logical_size.y;

        return std::min(xscale, yscale);
    }
    return 2;
}

Coord get_scaled_coord(Coord logical_size, int scale_factor)
{
    int max_factor = get_scaling_factor(logical_size);
    scale_factor = std::min(scale_factor, max_factor);
    return { logical_size.x * scale_factor, logical_size.y * scale_factor };
}

void throw_error(const std::string &msg)
{
    throw std::runtime_error(msg);
}

SDL::Scoped::Texture loadImage(const std::string &file, SDL_Renderer *ren) {
    SDL::Scoped::Texture texture(IMG_LoadTexture(ren, file.c_str()), SDL_DestroyTexture);
    if (texture == nullptr)
        throw_error("Couldn't open file " + file);
    return std::move(texture);
}

SDL::Scoped::Surface load_bmp(const std::string& filename)
{
    SDL::Scoped::Surface bmp(SDL_LoadBMP(filename.c_str()), SDL_FreeSurface);
    if (bmp == nullptr)
        throw_error("Couldn't open file " + filename);

    return std::move(bmp);
}

SDL::Scoped::Texture create_texture(SDL_Surface* surface, SDL_Renderer* renderer)
{
    SDL::Scoped::Texture texture(SDL_CreateTextureFromSurface(renderer, surface), SDL_DestroyTexture);
    if (texture == nullptr)
        throw_error("CreateTextureFromSurface");
    return std::move(texture);
}


std::ostream& write_short_string(std::ostream& out, const std::string& s) 
{
    assert(s.length() < 255);
    unsigned char length = static_cast<unsigned char>(s.length());
    write<unsigned char>(out, length);

    out.write(s.c_str(), length);
    return out;
}

std::istream& read_short_string(std::istream& in, std::string* s)
{
    unsigned char length;
    read<unsigned char>(in, &length);

    char buf[255];
    memset(buf, 0, 255);
    in.read(buf, length);
    *s = buf;

    return in;
}
