#include <iostream>
#include "utility.h"
#include "SDL.h"
#include "RogueCore\coord.h"

std::string getResourcePath(const std::string &subDir = "") {
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
        size_t pos = baseRes.rfind("bin");
        baseRes = baseRes.substr(0, pos) + "res" + PATH_SEP;
    }
    //If we want a specific subdirectory path in the resource directory
    //append it to the base path. This would be something like Lessons/res/Lesson0
    return subDir.empty() ? baseRes : baseRes + subDir + PATH_SEP;
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
