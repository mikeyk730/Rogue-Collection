#include <iostream>
#include "utility.h"
#include <SDL.h>
#include <display_interface_types.h>

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

void throw_error(const std::string &msg)
{
    std::cerr << SDL_GetError() << std::endl;
    throw std::runtime_error(msg + " error: " + SDL_GetError());
}

SDL::Scoped::Surface load_bmp(const std::string& filename)
{
    SDL::Scoped::Surface bmp(SDL_LoadBMP(filename.c_str()), SDL_FreeSurface);
    if (bmp == nullptr)
        throw_error("SDL_LoadBMP");

    return std::move(bmp);
}

SDL::Scoped::Texture create_texture(SDL_Surface* surface, SDL_Renderer* renderer)
{
    SDL::Scoped::Texture texture(SDL_CreateTextureFromSurface(renderer, surface), SDL_DestroyTexture);
    if (texture == nullptr)
        throw_error("CreateTextureFromSurface");
    return std::move(texture);
}

void render_texture_at(SDL_Texture* texture, SDL_Renderer* renderer, Coord position, SDL_Rect* clip)
{
    SDL_Rect r;
    r.x = position.x;
    r.y = position.y;
    if (clip != nullptr)
    {
        r.w = clip->w;
        r.h = clip->h;
    }
    else {
        SDL_QueryTexture(texture, NULL, NULL, &r.w, &r.h);
    }
    SDL_RenderCopy(renderer, texture, clip, &r);
}

void render_texture_at(SDL_Texture* texture, SDL_Renderer* renderer, Coord position, SDL_Rect clip)
{
    render_texture_at(texture, renderer, position, &clip);
}
