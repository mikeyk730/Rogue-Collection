#include <iostream>
#include <memory>
#include <vector>
#include "SDL.h"
#include "SDL_ttf.h"
#include "res_path.h"

#define PASSAGE   (0xb1)
#define DOOR      (0xce)
#define FLOOR     (0xfa)
#define PLAYER    (0x01)
#define TRAP      (0x04)
#define STAIRS    (0xf0)
#define GOLD      (0x0f)
#define POTION    (0xad)
#define SCROLL    (0x0d)
#define MAGIC     '$'
#define BMAGIC    '+'
#define FOOD      (0x05)
#define STICK     (0xe7)
#define ARMOR     (0x08)
#define AMULET    (0x0c)
#define RING      (0x09)
#define WEAPON    (0x18)
#define MISSILE   '*'
#define VWALL   (0xba)
#define HWALL   (0xcd)
#define ULWALL  (0xc9)
#define URWALL  (0xbb)
#define LLWALL  (0xc8)
#define LRWALL  (0xbc)

struct Coord
{
    int x;
    int y;
};


const int H_PIXELS_PER_CHAR = 8;
const int V_PIXELS_PER_CHAR = 16;
const int LINES = 25;
const int COLS = 80;
const int SCALE = 1;
const int WINDOW_X = H_PIXELS_PER_CHAR * COLS * SCALE;
const int WINDOW_Y = V_PIXELS_PER_CHAR * LINES * SCALE;


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
        SDL_Color magenta()  { return { 170,   0, 170, 255 }; } //from dosbox
        SDL_Color l_magenta(){ return { 255,  25, 255, 255 }; } 
        SDL_Color yellow()   { return { 255, 255,  25, 255 }; } //from dosbox
        SDL_Color brown()    { return { 170,  85,   0, 255 }; } //from dosbox
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

SDL_Rect get_rect(int i)
{
    SDL_Rect r;
    r.x = 9 * (i % 32);
    r.y = 16 * (i / 32);
    r.h = 16;
    r.w = 8;
    return r;
}

SDL::Scoped::Texture texture_for_byte(int chr, bool in_room, SDL_Surface* surface, SDL_Renderer* renderer)
{
    SDL_Rect r = get_rect(chr);
    using namespace SDL::Colors;

        //if it is inside a room
        if (in_room) switch (chr)
        {
        case DOOR: case VWALL: case HWALL: case ULWALL: case URWALL: case LLWALL: case LRWALL:
            return painted_texture(surface, &r, brown(), black(), renderer);
            break;
        case FLOOR:
            return painted_texture(surface, &r, l_green(), black(), renderer);
            break;
        case STAIRS:
            return painted_texture(surface, &r, black(), green(), renderer);
            break;
        case TRAP:
            return painted_texture(surface, &r, magenta(), black(), renderer);
            break;
        case GOLD: case PLAYER:
            return painted_texture(surface, &r, yellow(), black(), renderer);
            break;
        case POTION: case SCROLL: case STICK: case ARMOR: case AMULET: case RING: case WEAPON:
            return painted_texture(surface, &r, l_blue(), black(), renderer);
            break;
        case FOOD:
            return painted_texture(surface, &r, red(), black(), renderer);
            break;
        default:
            return painted_texture(surface, &r, grey(), black(), renderer);
        }
        //if inside a passage or a maze
        else switch (chr)
        {
        case FOOD:
            return painted_texture(surface, &r, red(), grey(), renderer);
            break;
        case GOLD: case PLAYER:
            return painted_texture(surface, &r, yellow(), grey(), renderer);
            break;
        case POTION: case SCROLL: case STICK: case ARMOR: case AMULET: case RING: case WEAPON:
            return painted_texture(surface, &r, blue(), grey(), renderer);
            break;
        case STAIRS:
            return painted_texture(surface, &r, black(), grey(), renderer);
            break;
        default:
            return painted_texture(surface, &r, black(), grey(), renderer);
        }
}

void add(int c, int i, SDL_Surface* tiles, SDL_Renderer* renderer) {
    auto t1 = texture_for_byte(c, true, tiles, renderer);
    auto t2 = texture_for_byte(c, false, tiles, renderer);

    render_texture_at(t1.release(), renderer, { 8 * i, 0 }, nullptr);
    render_texture_at(t2.release(), renderer, { 8 * i, 16 }, nullptr);
}

void output_tilemap(SDL_Surface* tiles, SDL_Renderer* renderer)
{
    SDL_RenderClear(renderer);
    int n = 0;
    std::vector<int> v = { 'A','B','C','D','E','F','G','H','I','J','K','L','M','N','O','P','Q','R','S','T','U','V','W','X','Y','Z',
        PLAYER, ULWALL, URWALL, LLWALL, LRWALL, HWALL, VWALL, FLOOR, FLOOR, DOOR, STAIRS, TRAP, AMULET, FOOD, GOLD, POTION, RING,
        SCROLL, STICK, WEAPON, WEAPON,WEAPON,WEAPON,WEAPON,WEAPON,WEAPON,WEAPON,WEAPON,WEAPON,ARMOR, ARMOR, ARMOR, ARMOR, ARMOR,
        ARMOR, ARMOR, ARMOR, MAGIC, BMAGIC, '\\', '/', '-', '|', '\\', '/', '-', '|', '\\', '/', '-', '|',MISSILE };
    for (auto i = v.begin(); i != v.end(); ++i)
    {
        add(*i, n++, tiles, renderer);
    }
    SDL_RenderPresent(renderer);
}


int main(int argc, char** argv)
{
    using namespace SDL::Scoped;

    int a;
    std::cin >> a;
    std::cout << a;

    try {

        if (SDL_Init(SDL_INIT_VIDEO) != 0) //todo:scope guard this
            throw_error("SDL_Init");

        //if (TTF_Init() != 0)
        //    throw_error("TTF_Init");

        Window win(SDL_CreateWindow("Rogue", 100, 50, 8*128, WINDOW_Y, SDL_WINDOW_SHOWN), SDL_DestroyWindow);
        if (win == nullptr)
            throw_error("SDL_CreateWindow");

        Renderer renderer(SDL_CreateRenderer(win.get(), -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC), SDL_DestroyRenderer);
        if (renderer == nullptr)
            throw_error("SDL_CreateRenderer");

        Surface tiles(load_bmp(getResourcePath("") + "dos24.bmp"));

        using namespace SDL::Colors;
        std::vector<Texture> textures;
        textures.push_back(std::move(create_texture(tiles.get(), renderer.get())));
        textures.push_back(std::move(painted_texture(tiles.get(), 0, green(), black(), renderer.get())));
        textures.push_back(std::move(painted_texture(tiles.get(), 0, cyan(), black(), renderer.get())));
        textures.push_back(std::move(painted_texture(tiles.get(), 0, red(), black(), renderer.get())));
        textures.push_back(std::move(painted_texture(tiles.get(), 0, magenta(), black(), renderer.get())));
        textures.push_back(std::move(painted_texture(tiles.get(), 0, brown(), black(), renderer.get())));
        textures.push_back(std::move(painted_texture(tiles.get(), 0, d_grey(), black(), renderer.get())));
        textures.push_back(std::move(painted_texture(tiles.get(), 0, l_blue(), black(), renderer.get())));
        textures.push_back(std::move(painted_texture(tiles.get(), 0, l_grey(), black(), renderer.get())));
        textures.push_back(std::move(painted_texture(tiles.get(), 0, l_red(), black(), renderer.get())));
        textures.push_back(std::move(painted_texture(tiles.get(), 0, l_magenta(), black(), renderer.get())));
        textures.push_back(std::move(painted_texture(tiles.get(), 0, yellow(), black(), renderer.get())));
        textures.push_back(std::move(painted_texture(tiles.get(), 0, grey(), black(), renderer.get())));
        textures.push_back(std::move(painted_texture(tiles.get(), 0, blue(), black(), renderer.get())));
        textures.push_back(std::move(painted_texture(tiles.get(), 0, black(), grey(), renderer.get())));
        textures.push_back(std::move(painted_texture(tiles.get(), 0, white(), black(), renderer.get())));
        //textures.push_back(std::move(painted_texture(tiles.get(), 0, black(), grey(), renderer.get())));

        SDL_StartTextInput();

        output_tilemap(tiles.get(), renderer.get());

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

            //render_texture_at(standard.get(), renderer.get(), { 16, 48 }, 0);
            for (int i = 0x20, n = 0; i < 128; ++i, ++n) {
                SDL_Rect r = get_rect(i);
                int k = 0;
                for (auto it = textures.begin(); it != textures.end(); ++it)
                    render_texture_at((*it).get(), renderer.get(), { 8*n, 16*k++}, r);
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
