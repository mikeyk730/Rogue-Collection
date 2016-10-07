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
}

int main(int argc, char **argv)
{
    std::shared_ptr<SdlRogue> output;
    try {
        if (SDL_Init(SDL_INIT_VIDEO) != 0)
            throw_error("SDL_Init");

        output.reset(new SdlRogue());
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
