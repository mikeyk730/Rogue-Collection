#include <memory>
#include <thread>
#include <SDL.h>
#include <SDL_ttf.h>
#include "sdl_rogue.h"
#include "game_select.h"
#include "utility.h"
#include <input_interface.h>
#include "../Shared/display_interface.h"
#include <Windows.h>

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

    void run_game(const std::string& lib, int argc, char** argv, DisplayInterface* screen, InputInterface* input)
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

            (*init)(screen, input, 25, 80);
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

    TileConfig pc_tiles = { "tiles.bmp", 78, 2 };
    TileConfig atari_tiles = { "atari.bmp", 78, 1 };

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

    GraphicsConfig unix_gfx = { &pc_text, 0, true, false };
    GraphicsConfig color_unix_gfx = { &pc_colored_text, 0, true, true };
    GraphicsConfig pc_gfx = { &pc_colored_text, 0, false, true };
    GraphicsConfig atari_gfx = { &cutesy_text, &atari_tiles, false, true };
    GraphicsConfig cutesy_gfx = { &alt_text, 0, false, true };

    std::vector<Options> s_options = {
        { "PC Rogue 1.48",    "Rogue_PC_1_48.dll", true,  true,{ pc_gfx, unix_gfx, color_unix_gfx, atari_gfx, cutesy_gfx } },
        { "Unix Rogue 5.4.2", "Rogue_5_4_2.dll",   false, false,{ unix_gfx, color_unix_gfx, pc_gfx, cutesy_gfx } },
        { "Unix Rogue 5.2.1", "Rogue_5_2_1.dll",   true,  false,{ unix_gfx, color_unix_gfx, pc_gfx, cutesy_gfx } },
        { "Unix Rogue 3.6.3", "Rogue_3_6_3.dll",   true,  false, { unix_gfx, color_unix_gfx, pc_gfx, cutesy_gfx } },
    };
}

int main(int argc, char** argv)
{
    int i = -1;
    if (argc > 1) {
        std::string arg(argv[1]);
        if (arg == "1" || arg == "2" || arg == "3" || arg == "0") {
            i = atoi(arg.c_str());
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

        window = SDL::Scoped::Window(SDL_CreateWindow("Rogue", 100, 100, 80*8, 25*16, SDL_WINDOW_SHOWN), SDL_DestroyWindow);
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

            //sdl_rogue.reset(new SdlRogue(pc_text, 0));
            //sdl_rogue.reset(new SdlRogue(pc_text, &pc_tiles));
            //sdl_rogue.reset(new SdlRogue(square_text, &atari_tiles, options));

            sdl_rogue.reset(new SdlRogue(window.get(), renderer.get(), option));
            //sdl_rogue.reset(new SdlRogue(square_text, 0, options));
            //sdl_rogue.reset(new SdlRogue(pc_colored_text, &pc_tiles));
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
        std::thread rogue(run_game, s_options[i].dll_name, argc, argv, sdl_rogue.get(), sdl_rogue.get());
        rogue.detach();

        sdl_rogue->Run();
    }

    renderer.release();
    window.release();
    SDL_Quit();
    return 0;
}
