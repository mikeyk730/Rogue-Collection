#include <iostream>
#include <memory>
#include <thread>
#include <vector>
#include "SDL.h"
#include "SDL_ttf.h"
#include "res_path.h"
#include "utility.h"

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

#include "RogueCore\coord.h"

const int H_PIXELS_PER_CHAR = 8;
const int V_PIXELS_PER_CHAR = 16;
const int LINES = 25;
const int COLS = 80;
const int SCALE = 1;
const int WINDOW_X = H_PIXELS_PER_CHAR * COLS * SCALE;
const int WINDOW_Y = V_PIXELS_PER_CHAR * LINES * SCALE;


namespace SDL
{
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
    std::cerr << SDL_GetError() << std::endl;
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

SDL::Scoped::Surface load_text(const std::string& s, TTF_Font* font, SDL_Color color, SDL_Color bg)
{
    SDL::Scoped::Surface surface(TTF_RenderText_Shaded(font, s.c_str(), color, bg), SDL_FreeSurface);
    if (surface == nullptr)
        throw_error("TTF_RenderText");

    return std::move(surface);
}


SDL_Rect get_rect(unsigned char i)
{
    SDL_Rect r;
    r.x = 9 * (i % 32);
    r.y = 16 * (i / 32);
    r.h = 16;
    r.w = 8;
    return r;
}

std::pair<SDL_Color, SDL_Color> color_for_byte(int chr, bool in_room)
{
    using namespace SDL::Colors;

        //if it is inside a room
        if (in_room) switch (chr)
        {
        case DOOR: case VWALL: case HWALL: case ULWALL: case URWALL: case LLWALL: case LRWALL:
            return std::make_pair(brown(), black());
            break;
        case FLOOR:
            return std::make_pair(l_green(), black());
            break;
        case STAIRS:
            return std::make_pair(black(), green());
            break;
        case TRAP:
            return std::make_pair(magenta(), black());
            break;
        case GOLD: case PLAYER:
            return std::make_pair(yellow(), black());
            break;
        case POTION: case SCROLL: case STICK: case ARMOR: case AMULET: case RING: case WEAPON:
            return std::make_pair(l_blue(), black());
            break;
        case FOOD:
            return std::make_pair(red(), black());
            break;
        default:
            return std::make_pair(grey(), black());
        }
        //if inside a passage or a maze
        else switch (chr)
        {
        case FOOD:
            return std::make_pair(red(), grey());
            break;
        case GOLD: case PLAYER:
            return std::make_pair(yellow(), grey());
            break;
        case POTION: case SCROLL: case STICK: case ARMOR: case AMULET: case RING: case WEAPON:
            return std::make_pair(blue(), grey());
            break;
        case STAIRS:
            return std::make_pair(black(), grey());
            break;
        default:
            return std::make_pair(black(), grey());
        }
}


SDL::Scoped::Texture texture_for_byte(int chr, bool in_room, SDL_Surface* surface, SDL_Renderer* renderer)
{
    SDL_Rect r = get_rect(chr);
    auto p = color_for_byte(chr, in_room);
    return painted_texture(surface, &r, p.first, p.second, renderer);
}



void add_txt(int c, int i, SDL_Surface* tiles, SDL_Renderer* renderer) {
    auto t1 = texture_for_byte(c, true, tiles, renderer);
    auto t2 = texture_for_byte(c, false, tiles, renderer);

    render_texture_at(t1.release(), renderer, { 8 * i, 0 }, nullptr);
    render_texture_at(t2.release(), renderer, { 8 * i, 16 }, nullptr);
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

std::string get_print_chars()
{
    std::string s;
    for (int i = 0; i < 256; ++i) {
        s.push_back(i);
    }
    s.push_back(PASSAGE);
    s.push_back(0xcc);
    s.push_back(0xb9);
    s.push_back('A');
    return s;
}

void screenshot(SDL_Renderer* renderer, SDL_Window* window, int x, int y) {
    // Create an empty RGB surface that will be used to create the screenshot bmp file
    SDL_Surface* pScreenShot = SDL_CreateRGBSurface(0, x, y, 32, 0x00ff0000, 0x0000ff00, 0x000000ff, 0xff000000);

    if (pScreenShot)
    {
        // Read the pixels from the current render target and save them onto the surface
        SDL_RenderReadPixels(renderer, NULL, SDL_GetWindowPixelFormat(window), pScreenShot->pixels, pScreenShot->pitch);

        // Create the bmp screenshot file
        SDL_SaveBMP(pScreenShot, "Screenshot.bmp");

        // Destroy the screenshot surface
        SDL_FreeSurface(pScreenShot);
    }
}


void test()
{
    using namespace SDL::Scoped;

    try {
        if (TTF_Init() != 0)
            throw_error("TTF_Init");

        Window window(SDL_CreateWindow("Rogue", 100, 50, 8 * 256, 16*16, SDL_WINDOW_SHOWN), SDL_DestroyWindow);
        if (window == nullptr)
            throw_error("SDL_CreateWindow");

        Renderer renderer(SDL_CreateRenderer(window.get(), -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC), SDL_DestroyRenderer);
        if (renderer == nullptr)
            throw_error("SDL_CreateRenderer");

        Surface tiles(load_bmp(getResourcePath("") + "dos.bmp"));
        Font font(load_font("C:\\WINDOWS\\Fonts\\CONSOLA.TTF", 19));

        using namespace SDL::Colors;
        std::vector<Texture> textures;

        auto s = get_print_chars();

        bool text = false;        
        if (!text) {
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
        }
        else
        {
            textures.push_back(std::move(create_texture(load_text(s, font.get(), grey(), black()).get(), renderer.get())));
            textures.push_back(std::move(create_texture(load_text(s, font.get(), green(), black()).get(), renderer.get())));
            textures.push_back(std::move(create_texture(load_text(s, font.get(), cyan(), black()).get(), renderer.get())));
            textures.push_back(std::move(create_texture(load_text(s, font.get(), red(), black()).get(), renderer.get())));
            textures.push_back(std::move(create_texture(load_text(s, font.get(), magenta(), black()).get(), renderer.get())));
            textures.push_back(std::move(create_texture(load_text(s, font.get(), brown(), black()).get(), renderer.get())));
            textures.push_back(std::move(create_texture(load_text(s, font.get(), d_grey(), black()).get(), renderer.get())));
            textures.push_back(std::move(create_texture(load_text(s, font.get(), l_blue(), black()).get(), renderer.get())));
            textures.push_back(std::move(create_texture(load_text(s, font.get(), l_grey(), black()).get(), renderer.get())));
            textures.push_back(std::move(create_texture(load_text(s, font.get(), l_red(), black()).get(), renderer.get())));
            textures.push_back(std::move(create_texture(load_text(s, font.get(), l_magenta(), black()).get(), renderer.get())));
            textures.push_back(std::move(create_texture(load_text(s, font.get(), yellow(), black()).get(), renderer.get())));
            textures.push_back(std::move(create_texture(load_text(s, font.get(), grey(), black()).get(), renderer.get())));
            textures.push_back(std::move(create_texture(load_text(s, font.get(), blue(), black()).get(), renderer.get())));
            textures.push_back(std::move(create_texture(load_text(s, font.get(), black(), grey()).get(), renderer.get())));
            textures.push_back(std::move(create_texture(load_text(s, font.get(), white(), black()).get(), renderer.get())));
        }

        SDL_StartTextInput();

        //output_tilemap(tiles.get(), renderer.get());

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

            if (!text) {
                for (int i = 0; i < s.length(); ++i) {
                    SDL_Rect r = get_rect(s[i]);
                    int k = 0;
                    for (auto it = textures.begin(); it != textures.end(); ++it)
                        render_texture_at((*it).get(), renderer.get(), { 8 * i, 16 * k++ }, r);
                }
            }
            else {

                int k = 0;
                int w, h;
                SDL_QueryTexture(textures[0].get(), NULL, NULL, &w, &h);

                for (auto it = textures.begin(); it != textures.end(); ++it) {
                    render_texture_at(it->get(), renderer.get(), { 0, h * k++ }, nullptr);
                    SDL_SetWindowSize(window.get(), w, 16 * h);
                }
            }

            SDL_RenderPresent(renderer.get());

            screenshot(renderer.get(), window.get(), 8 * 256, 16 * 16);
            for (;;);
        }

        SDL_Quit();
    }
    catch (const std::runtime_error& e)
    {
        std::cout << e.what();
    }
}


#include <memory>
#include "sdl_keyboard_input.h"
#include "sdl_window.h"
#include "RogueCore\main.h"

void run_game(int argc, char **argv, std::shared_ptr<DisplayInterface> output, std::shared_ptr<InputInterface> input)
{
    game_main(argc, argv, output, input);
}

int main(int argc, char **argv)
{
    if (SDL_Init(SDL_INIT_VIDEO) != 0)
        throw_error("SDL_Init");

    std::shared_ptr<SdlWindow> output(new SdlWindow());
    std::shared_ptr<SdlKeyboardInput> input(new SdlKeyboardInput());

    //start rogue engine on a background thread
    std::thread rogue(run_game, argc, argv, output, input);
    rogue.detach();

    output->Run();

    SDL_Quit();
    return 0;
}
