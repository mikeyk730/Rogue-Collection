#include <memory>
#include <thread>
#include <vector>
#include "SDL.h"
#include "sdl_rogue.h"
#include "utility.h"
#include <input_interface.h>
#include <display_interface.h>
#include <Windows.h>

DisplayInterface::~DisplayInterface() {}
InputInterface::~InputInterface() {}

namespace
{
    std::vector<std::string> dlls = { "Rogue_3_6_3.dll", "Rogue_5_2_1.dll", "Rogue_5_4_2.dll", "Rogue_PC_1_48.dll" };

    typedef int(*game_main)(int, char**, char**);
    typedef void(*init_game)(DisplayInterface*, InputInterface*);

    //void run_game(int argc, char **argv, std::shared_ptr<OutputInterface> output, std::shared_ptr<InputInterface> input)
    void run_game(int index, int argc, char** argv, DisplayInterface* screen, InputInterface* input)
    {
        try {
            HINSTANCE dll = LoadLibrary(dlls[index].c_str());
            if (!dll) {
                throw_error("Couldn't load dll " + dlls[index]);
            }

            init_game init = (init_game)GetProcAddress(dll, "init_game");
            if (!init) {
                throw_error("Couldn't load init_game from " + dlls[index]);
            }

            game_main game = (game_main)GetProcAddress(dll, "rogue_main");
            if (!game) {
                throw_error("Couldn't load rogue_main from " + dlls[index]);
            }

            (*init)(screen, input);
            (*game)(argc, argv, environ);
        }
        catch (const std::runtime_error& e)
        {
            SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR,
                "Error",
                e.what(),
                NULL);
            exit(1);
        }
    }

    TileConfig pc_tiles = { "tiles.bmp", 78, 2 };
    TileConfig atari_tiles = { "atari.bmp", 78, 1 };

    TextConfig pc_text = { "text.bmp",{ 0x07 } };
    TextConfig pc_colored_text = { "text_colored.bmp",{
        0x07, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
        0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
        0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0xA0,
        0x78, 0x79, 0x7a, 0x7b, 0x7c, 0x7d, 0x7e, 0x7f }
    };
    TextConfig square_text = { "text_16x16.bmp",{
        0x07, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
        0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
        0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0xA0,
        0x78, 0x79, 0x7a, 0x7b, 0x7c, 0x7d, 0x7e, 0x7f }
    };
}

int main(int argc, char** argv)
{
    int index = 4-1;
    if (argc > 1) {
        std::string arg(argv[1]);
        if (arg == "1" || arg == "2" || arg == "3" || arg == "4") {
            index = atoi(arg.c_str())-1;
            --argc;
            ++argv;
        }
    }
    std::shared_ptr<SdlRogue> sdl_rogue;
    
    try {
        if (SDL_Init(SDL_INIT_VIDEO) != 0) {
            throw_error("SDL_Init");
        }

        Options options;
        if (index == 0 || index == 1 || index == 3) {
            options.emulate_alt_controls = true;
        }

        //sdl_rogue.reset(new SdlRogue(pc_text, 0));
        //sdl_rogue.reset(new SdlRogue(pc_text, &pc_tiles));
        //sdl_rogue.reset(new SdlRogue(square_text, &atari_tiles, options));
        
        sdl_rogue.reset(new SdlRogue(pc_colored_text, 0, options));
        //sdl_rogue.reset(new SdlRogue(square_text, 0, options));
        //sdl_rogue.reset(new SdlRogue(pc_colored_text, &pc_tiles));

        //output = CreateScreenOutput(sdl_rogue);
    }
    catch (const std::runtime_error& e)
    {
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR,
            "Error",
            e.what(),
            NULL);
        return 1;
    }

    //start rogue engine on a background thread
    std::thread rogue(run_game, index, argc, argv, sdl_rogue.get(), sdl_rogue.get());
    rogue.detach();

    sdl_rogue->Run();

    SDL_Quit();
    return 0;
}
