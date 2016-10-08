#include <memory>
#include <thread>
#include "SDL.h"
#include "sdl_rogue.h"
#include "utility.h"
#include "RogueCore/main.h"

namespace
{
    void run_game(int argc, char **argv, std::shared_ptr<DisplayInterface> output, std::shared_ptr<InputInterface> input)
    {
        game_main(argc, argv, output, input);
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
    std::shared_ptr<SdlRogue> output;
    try {
        if (SDL_Init(SDL_INIT_VIDEO) != 0)
            throw_error("SDL_Init");

        //output.reset(new SdlRogue(pc_text, 0));
        //output.reset(new SdlRogue(pc_text, &pc_tiles));
        //output.reset(new SdlRogue(pc_colored_text, &atari_tiles));
        
        output.reset(new SdlRogue(pc_colored_text, 0));
        //output.reset(new SdlRogue(pc_colored_text, &pc_tiles));
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
    std::thread rogue(run_game, argc, argv, output, output);
    rogue.detach();

    output->Run();

    SDL_Quit();
    return 0;
}
