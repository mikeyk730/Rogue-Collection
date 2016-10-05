#include <iostream>
#include <memory>
#include "SDL.h"
#include "SDL_ttf.h"
#include "res_path.h"

const int H_PIXELS_PER_CHAR = 8;
const int V_PIXELS_PER_CHAR = 16;
const int LINES = 25;
const int COLS = 80;
const int SCALE = 1;
const int WINDOW_X = H_PIXELS_PER_CHAR * COLS * SCALE;
const int WINDOW_Y = V_PIXELS_PER_CHAR * LINES * SCALE;

struct Coord
{
    int x;
    int y;
};

namespace SDL
{
    namespace Scoped
    {
        typedef std::unique_ptr<SDL_Window, void(*)(SDL_Window*)> Window;
        typedef std::unique_ptr<SDL_Renderer, void(*)(SDL_Renderer*)> Renderer;
        typedef std::unique_ptr<SDL_Surface, void(*)(SDL_Surface*)> Surface;
        typedef std::unique_ptr<SDL_Texture, void(*)(SDL_Texture*)> Texture;
        typedef std::unique_ptr<TTF_Font, void(*)(TTF_Font*)> Font;
    }
}

void throw_error(const std::string &msg)
{
    throw std::runtime_error(msg + " error: " + SDL_GetError());
}

/*
* Return the pixel value at (x, y)
* NOTE: The surface must be locked before calling this!
*/
Uint32 getpixel(SDL_Surface *surface, int x, int y)
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
void putpixel(SDL_Surface *surface, int x, int y, Uint32 pixel)
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

SDL::Scoped::Texture create_texture(SDL_Surface* surface, SDL_Renderer* renderer)
{
    SDL::Scoped::Texture texture(SDL_CreateTextureFromSurface(renderer, surface), SDL_DestroyTexture);
    if (texture == nullptr)
        throw_error("CreateTextureFromSurface");
    return std::move(texture);
}

SDL::Scoped::Surface load_bmp(const std::string& filename)
{
    SDL::Scoped::Surface bmp(SDL_LoadBMP(filename.c_str()), SDL_FreeSurface);
    if (bmp == nullptr)
        throw_error("SDL_LoadBMP");

    return std::move(bmp);
}

void paint_surface(SDL_Surface* surface, SDL_Color fg, SDL_Color bg)
{
    int n = surface->w * surface->h;
    for (int i = 0; i < n; ++i)
    {
        int x = i % surface->w;
        int y = i / surface->w;
        Uint32 p = getpixel(surface, x, y);

        Uint8 r, g, b;
        SDL_GetRGB(p, surface->format, &r, &g, &b);
        if (r || g || b) {
            p = SDL_MapRGB(surface->format, fg.r, fg.g, fg.b);
        }
        else {
            p = SDL_MapRGB(surface->format, bg.r, bg.g, bg.b);
        }
        putpixel(surface, x, y, p);
    }

}

SDL::Scoped::Surface blit_surface(SDL_Surface* surface, SDL_Rect* r = 0)
{
    int w = surface->w;
    int h = surface->h;
    if (r)
    {
        w = r->w;
        h = r->h;
    }

    SDL::Scoped::Surface tile(SDL_CreateRGBSurface(0, w, h, surface->format->BitsPerPixel, 255, 255, 255, 255), SDL_FreeSurface);
    if (tile == nullptr)
        throw_error("SDL_CreateRGBSurface");

    if (SDL_BlitSurface(surface, r, tile.get(), 0))
        throw_error("SDL_BlitSurface");

    return std::move(tile);
}

SDL::Scoped::Texture painted_texture(SDL_Surface* surface, SDL_Rect* r, SDL_Color fg, SDL_Color bg, SDL_Renderer* renderer)
{
    SDL::Scoped::Surface tile(blit_surface(surface, r));
    paint_surface(tile.get(), fg, bg);
    return create_texture(tile.get(), renderer);
}

SDL::Scoped::Font load_font(const std::string& filename, int size)
{
    std::string font_file(getResourcePath("") + "CLACON.TTF");
    SDL::Scoped::Font font(TTF_OpenFont(filename.c_str(), size), TTF_CloseFont);
    if (font == nullptr)
        throw_error("TTF_OpenFont");

    return std::move(font);
}

SDL::Scoped::Surface load_text(const std::string& s, TTF_Font* font, SDL_Color color)
{
    SDL::Scoped::Surface surface(TTF_RenderText_Blended(font, s.c_str(), color), SDL_FreeSurface);
    if (surface == nullptr)
        throw_error("TTF_RenderText");

    return std::move(surface);
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

int main(int argc, char** argv)
{
    using namespace SDL::Scoped;

    try {

        if (SDL_Init(SDL_INIT_VIDEO) != 0) //todo:scope guard this
            throw_error("SDL_Init");

        //if (TTF_Init() != 0)
        //    throw_error("TTF_Init");

        Window win(SDL_CreateWindow("Rogue", 100, 50, WINDOW_X, WINDOW_Y, SDL_WINDOW_SHOWN), SDL_DestroyWindow);
        if (win == nullptr)
            throw_error("SDL_CreateWindow");

        Renderer renderer(SDL_CreateRenderer(win.get(), -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC), SDL_DestroyRenderer);
        if (renderer == nullptr)
            throw_error("SDL_CreateRenderer");

        Surface tiles(load_bmp(getResourcePath("") + "dos24.bmp"));

        Texture yellow(painted_texture(tiles.get(), 0, { 255,225,0,255 }, { 0,0,0,255 }, renderer.get()));
        Texture blue_standout(painted_texture(tiles.get(), 0, { 0,0,255,255 }, { 0xc0,0xc0,0xc0,255 }, renderer.get()));

        SDL_StartTextInput();

        char key = 1;
        //Our event structure
        SDL_Event e;
        bool quit = false;
        while (!quit) {
            while (SDL_PollEvent(&e)) {
                if (e.type == SDL_QUIT) {
                    quit = true;
                }
                else if (e.type == SDL_TEXTINPUT) {
                    std::cout << "ch:" << e.text.text << std::endl;
                    key = e.text.text[0];
                }
                else if (e.type == SDL_KEYUP) {
                    //handle esc, backspace, directions, numpad, function keys, return, ctrl-modified, ins, del
                    //how to read scroll lock, etc?
                }
            }

            SDL_RenderClear(renderer.get());
            for (int i = 0; i < 256; ++i) {
                SDL_Rect r;
                r.x = 9 * (i % 32);
                r.y = 16 * (i / 32);
                r.h = 16;
                r.w = 8;

                render_texture_at(yellow.get(), renderer.get(), { 16 + 8 * (i % 32), 16 + 16 * (i / 32) }, r);
                render_texture_at(blue_standout.get(), renderer.get(), { 16 + 8 * (i % 32), 180 + 16 * (i / 32) }, r);
            }
            SDL_RenderPresent(renderer.get());
        }

        SDL_Quit();
    }
    catch (const std::runtime_error& e)
    {
        std::cout << e.what();
    }

    return 0;
}
