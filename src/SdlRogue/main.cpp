#include <memory>
#include <thread>
#include <fstream>
#include <SDL.h>
#include <SDL_ttf.h>
#include <Windows.h>
#include <input_interface.h>
#include "../Shared/display_interface.h"
#include "sdl_rogue.h"
#include "game_select.h"
#include "utility.h"
#include "environment.h"

DisplayInterface::~DisplayInterface() {}
InputInterface::~InputInterface() {}

namespace
{
    typedef int(*game_main)(int, char**, char**);
    typedef void(*init_game)(DisplayInterface*, InputInterface*, int lines, int cols);

    struct LibraryDeleter
    {
        typedef HMODULE pointer;
        void operator()(HMODULE h) { FreeLibrary(h); }
    };

    void run_game(const std::string& lib, int argc, char** argv, DisplayInterface* screen, InputInterface* input, int lines, int cols)
    {
        std::unique_ptr<HMODULE, LibraryDeleter> dll(LoadLibrary(lib.c_str()));
        try {
            if (!dll) {
                throw_error("Couldn't load dll " + lib);
            }

            init_game init = (init_game)GetProcAddress(dll.get(), "init_game");
            if (!init) {
                throw_error("Couldn't load init_game from " + lib);
            }

            game_main game = (game_main)GetProcAddress(dll.get(), "rogue_main");
            if (!game) {
                throw_error("Couldn't load rogue_main from " + lib);
            }

            (*init)(screen, input, lines, cols);
            (*game)(argc, argv, environ);
        }
        catch (const std::runtime_error& e)
        {
            std::string s(e.what());
            SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR,
                "Error",
                e.what(),
                NULL);
            exit(1);
        }
    }

    const int WINDOW_W = 640;
    const int WINDOW_H = 400;

    TileConfig pc_tiles = { "tiles.bmp", 78, 2 };
    TileConfig atari_slime_tiles = { "atari.bmp", 78, 1 };
    TileConfig atari_snake_tiles = { "atari_snake.bmp", 78, 1 };

    TextConfig pc_text = { "text.bmp", { 256, 1 }, { 0x07 } };
    TextConfig alt_text = { "16x16.bmp", { 16, 16 }, { 0x07 } };
    TextConfig pc_colored_text = { "text_colored.bmp", {256, 1}, {
        0x07, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
        0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
        0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0xA0,
        0x78, 0x79, 0x7a, 0x7b, 0x7c, 0x7d, 0x7e, 0x7f }
    };
    TextConfig cutesy_text = { "text_16x16.bmp", {256, 1}, {
        0x07, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
        0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
        0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0xA0,
        0x78, 0x79, 0x7a, 0x7b, 0x7c, 0x7d, 0x7e, 0x7f }
    };

    GraphicsConfig unix_gfx = { "unix", &pc_text, 0, true, false };
    GraphicsConfig color_unix_gfx = { "unix_color", &pc_colored_text, 0, true, true };
    GraphicsConfig pc_gfx = { "pc", &pc_colored_text, 0, false, true };
    GraphicsConfig atari_slime_gfx = { "tiles", &alt_text, &atari_slime_tiles, false, true };
    GraphicsConfig atari_snake_gfx = { "tiles", &alt_text, &atari_snake_tiles, false, true };
    GraphicsConfig cutesy_gfx = { "boxy", &cutesy_text, 0, false, true };

    std::vector<Options> s_options = {
        { "PC Rogue 1.48",    "Rogue_PC_1_48.dll", {80,25}, {40,25}, true,  true,  { pc_gfx, unix_gfx, color_unix_gfx, atari_slime_gfx, cutesy_gfx } },
        { "Unix Rogue 5.4.2", "Rogue_5_4_2.dll",   {80,25}, {80,24}, false, false, { unix_gfx, color_unix_gfx, pc_gfx, atari_snake_gfx, cutesy_gfx } },
        { "Unix Rogue 5.2.1", "Rogue_5_2_1.dll",   {80,25}, {70,22}, true,  false, { unix_gfx, color_unix_gfx, pc_gfx, cutesy_gfx } },
        { "Unix Rogue 3.6.3", "Rogue_3_6_3.dll",   {80,25}, {70,22}, true,  false, { unix_gfx, color_unix_gfx, pc_gfx, cutesy_gfx } },
    };
}



int main(int argc, char** argv)
{
    int i = -1;

    std::ifstream env_file("rogue.opt");
    std::unique_ptr<Environment> env(new Environment());
    env->from_file(env_file);

    std::string game;
    if (env->get("game", &game)) {
        if (game == "a" || game == "b" || game == "c" || game == "d") {
            i = game[0] - 'a';
        }
    }

    if (argc > 1) {
        std::string game(argv[1]);
        if (game == "a" || game == "b" || game == "c" || game == "d") {
            i = game[0] - 'a';
            --argc;
            ++argv;
        }
    }

    SDL::Scoped::Window window(nullptr, SDL_DestroyWindow);
    SDL::Scoped::Renderer renderer(nullptr, SDL_DestroyRenderer);
    std::shared_ptr<SdlRogue> sdl_rogue;
    try {
        if (SDL_Init(SDL_INIT_VIDEO) != 0) {
            throw_error("SDL_Init");
        }

        if (TTF_Init() != 0) {
            throw_error("TTF_Init");
        }

        window = SDL::Scoped::Window(SDL_CreateWindow("Rogue Collection", 100, 100, WINDOW_W, WINDOW_H, SDL_WINDOW_ALLOW_HIGHDPI | SDL_WINDOW_HIDDEN), SDL_DestroyWindow);
        if (window == nullptr)
            throw_error("SDL_CreateWindow");

        renderer = SDL::Scoped::Renderer(SDL_CreateRenderer(window.get(), -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC), SDL_DestroyRenderer);
        if (renderer == nullptr)
            throw_error("SDL_CreateRenderer");

        if (i == -1) {
            GameSelect select(window.get(), renderer.get(), s_options);
            i = select.GetSelection();
        }
        
        if (i != -1) {
            Options& option = s_options[i];

            if (!env->write_to_os())
                throw_error("Couldn't write environment");

            std::string screen;
            Coord dims = option.screen;
            if (env->get("small_screen", &screen) && screen == "true")
            {
                dims = option.small_screen;
            }
            env->cols(dims.x);
            env->lines(dims.y);

            sdl_rogue.reset(new SdlRogue(window.get(), renderer.get(), option, env.get()));
        }
    }
    catch (const std::runtime_error& e)
    {
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR,
            "Error",
            e.what(),
            NULL);
        return 1;
    }

    if (sdl_rogue) {
        //start rogue engine on a background thread
        std::thread rogue(run_game, s_options[i].dll_name, argc, argv, sdl_rogue.get(), sdl_rogue.get(), env->lines(), env->cols());
        rogue.detach();

        sdl_rogue->Run();
    }

    renderer.release();
    window.release();
    SDL_Quit();
    return 0;
}
