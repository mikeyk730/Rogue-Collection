#include <memory>
#include <thread>
#include "SDL.h"
#include "sdl_rogue.h"
#include "utility.h"
#include <input_interface.h>
#include <display_interface.h>
#include <curses.h>

DisplayInterface::~DisplayInterface() {}
InputInterface::~InputInterface() {}

extern "C"
{
    void init_curses(DisplayInterface* screen, InputInterface* input);
    void shutdow_curses();
    int game_main(int argc, char **argv);
}

namespace
{
    //void run_game(int argc, char **argv, std::shared_ptr<OutputInterface> output, std::shared_ptr<InputInterface> input)
    void run_game(int argc, char **argv, DisplayInterface* screen, InputInterface* input)
    {
        try {
            init_curses(screen, input);
            game_main(argc, argv);
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
}

int main(int argc, char **argv)
{
    std::shared_ptr<SdlRogue> sdl_rogue;
    //std::shared_ptr<OutputInterface> output;
    
    try {
        if (SDL_Init(SDL_INIT_VIDEO) != 0)
            throw_error("SDL_Init");

        //sdl_rogue.reset(new SdlRogue(pc_text, 0));
        //sdl_rogue.reset(new SdlRogue(pc_text, &pc_tiles));
        //sdl_rogue.reset(new SdlRogue(pc_colored_text, &atari_tiles));
        
        sdl_rogue.reset(new SdlRogue(pc_colored_text, 0));
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
    std::thread rogue(run_game, argc, argv, sdl_rogue.get(), sdl_rogue.get());
    rogue.detach();

    sdl_rogue->Run();

    SDL_Quit();
    return 0;
}
