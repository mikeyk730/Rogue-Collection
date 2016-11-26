#include <memory>
#include <thread>
#include <fstream>
#include <SDL.h>
#include <SDL_ttf.h>
#include <input_interface.h>
#include <display_interface.h>
#include "sdl_rogue.h"
#include "text_provider.h"
#include "tile_provider.h"
#include "game_select.h"
#include "sdl_utility.h"
#include "environment.h"
#include "game_config.h"
#include "run_game.h"
#include "args.h"

int main(int argc, char** argv)
{
    Args args(argc, argv);
    std::shared_ptr<Environment> current_env(new Environment(args));
    InitGameConfig(current_env.get());
    
    int i = -1;
    std::string replay_path;
    std::string game;
    current_env->Get("game", &game);
    if (!game.empty())
    {
        if (game.size() == 1 && (game[0] >= 'a' && game[0] < 'a' + (int)s_options.size()))
        {
            i = game[0] - 'a';
        }
        else {
            replay_path = game;
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
        
        Coord window_size = GetScaledCoord({ kWindowWidth, kWindowHeight }, current_env->WindowScaling());
        window = SDL::Scoped::Window(SDL_CreateWindow(SdlRogue::kWindowTitle, 100, 100, window_size.x, window_size.y, SDL_WINDOW_ALLOW_HIGHDPI | SDL_WINDOW_SHOWN), SDL_DestroyWindow);
        if (window == nullptr)
            throw_error("SDL_CreateWindow");

        renderer = SDL::Scoped::Renderer(SDL_CreateRenderer(window.get(), -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC), SDL_DestroyRenderer);
        if (renderer == nullptr)
            throw_error("SDL_CreateRenderer");
        SDL_RenderSetLogicalSize(renderer.get(), kWindowWidth, kWindowHeight);
        //SDL_RenderSetIntegerScale(renderer.get(), 1);

        if (current_env->Fullscreen())
        {
            SetFullscreen(window.get(), true);
        }

        if (i == -1 && replay_path.empty()) {
            GameSelect select(window.get(), renderer.get(), s_options, current_env.get());
            auto selection = select.GetSelection();
            i = selection.first;
            replay_path = selection.second;
        }

        if (i >= 0 && s_options[i].name == "PC Rogue 1.48") {
            std::string value;
            if (!current_env->Get("show_title_screen", &value) || value != "false")
            {
                TitleScreen title(window.get(), renderer.get(), s_options, current_env.get());
                if (!title.Run()) {
                    i = -1;
                    replay_path.clear();
                }
            }
        }

        if (i >= 0) {
            sdl_rogue.reset(new SdlRogue(window.get(), renderer.get(), current_env, i));
        }
        else if (!replay_path.empty()) {
            sdl_rogue.reset(new SdlRogue(window.get(), renderer.get(), current_env, replay_path));
        }

        if (sdl_rogue) {
            //start rogue engine on a background thread
            std::thread rogue(RunGame<SdlRogue>, sdl_rogue->Options().dll_name, argc, argv, sdl_rogue.get());
            rogue.detach();

            sdl_rogue->Run();
        }
    }
    catch (const std::runtime_error& e)
    {
        DisplayMessage(SDL_MESSAGEBOX_ERROR, "Fatal Error", e.what());
        return 1;
    }

    renderer.release();
    window.release();
    SDL_Quit();
    return 0;
}

DisplayInterface::~DisplayInterface() {}
InputInterface::~InputInterface() {}
