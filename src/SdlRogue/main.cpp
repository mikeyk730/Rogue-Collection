#include <memory>
#include <thread>
#include "SDL.h"
#include "sdl_rogue.h"
#include "utility.h"
#include "RogueCore/main.h"

void run_game(int argc, char **argv, std::shared_ptr<DisplayInterface> output, std::shared_ptr<InputInterface> input)
{
    game_main(argc, argv, output, input);
}

int main(int argc, char **argv)
{
    if (SDL_Init(SDL_INIT_VIDEO) != 0)
        throw_error("SDL_Init");

    std::shared_ptr<SdlRogue> output(new SdlRogue());

    //start rogue engine on a background thread
    std::thread rogue(run_game, argc, argv, output, output);
    rogue.detach();

    output->Run();

    SDL_Quit();
    return 0;
}
