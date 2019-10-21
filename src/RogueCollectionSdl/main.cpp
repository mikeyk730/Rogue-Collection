#include <memory>
#include <thread>
#include <SDL.h>
#include <SDL_ttf.h>
#include <input_interface.h>
#include <display_interface.h>
#include "sdl_rogue.h"
#include "sdl_display.h"
#include "text_provider.h"
#include "tile_provider.h"
#include "game_select.h"
#include "sdl_utility.h"
#include "environment.h"
#include "game_config.h"
#include "run_game.h"
#include "args.h"
#include "pipe_input.h"
#include "utility.h"
#include "start_process.h"

const int kWindowWidth = 640;
const int kWindowHeight = 400;
void InitGameConfig(Environment* env);

int RunMain(int argc, char** argv);

int main(int argc, char** argv)
{
    return StartProcess(RunMain, argc, argv);
}

int RunMain(int argc, char** argv)
{
    Args args(argc, argv);
    std::shared_ptr<Environment> current_env(new Environment(args));

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

        if (current_env->Fullscreen()) {
            SetFullscreen(window.get(), true);
        }

        InitGameConfig(current_env.get());

        std::unique_ptr<GameConfig> game;
        std::string replay_path;

        if (args.rogomatic_player) {
            game.reset(new GameConfig(GetRogomaticGameConfig()));
        }
        else {
            std::string value;
            current_env->Get("game", &value);
            if (!value.empty())
            {
                if (value.size() == 1 && (value[0] >= 'a' && value[0] < 'a' + (int)s_options.size())) {
                    int i = value[0] - 'a';
                    game.reset(new GameConfig(GetGameConfig(i)));
                }
                else {
                    replay_path = value;
                }
            }
        }

        if (!game && replay_path.empty()) {
            GameSelect select(window.get(), renderer.get(), s_options, current_env.get());
            auto selection = select.GetSelection();
            if (selection.first >= 0) {
                game.reset(new GameConfig(GetGameConfig(selection.first)));
            }

            replay_path = selection.second;
        }

        if (game && HasTitleScreen(*game)) {
            std::string value;
            if (!current_env->Get("show_title_screen", &value) || value != "false") {
                TitleScreen title(window.get(), renderer.get(), current_env.get());
                if (!title.Run()) {
                    game.reset();
                    replay_path.clear();
                }
            }
        }

        if (game) {
            if (args.rogomatic && !game->supports_rogomatic) {
                DisplayMessage(
                    SDL_MESSAGEBOX_WARNING,
                    "Rogomatic", "Rogomatic does not support " + game->name);
                args.rogomatic &= game->supports_rogomatic;
            }

            if (args.rogomatic)
            {
                current_env->SetRogomaticValues();
            }

            sdl_rogue.reset(
                new SdlRogue(
                    window.get(),
                    renderer.get(),
                    current_env,
                    *game,
                    args));
        }
        else if (!replay_path.empty()) {
            sdl_rogue.reset(new SdlRogue(window.get(), renderer.get(), current_env, replay_path));
        }

        if (sdl_rogue) {
            //start rogue engine on a background thread
            Coord dims = sdl_rogue->Display()->GetDimensions();
            std::thread rogue([&] {
                RunGame(
                    sdl_rogue->Options().dll_name,
                    sdl_rogue->Display(),
                    sdl_rogue->Input(),
                    current_env.get(),
                    dims.y,
                    dims.x,
                    args);
                sdl_rogue->PostQuit();
            });
            rogue.detach();

            sdl_rogue->Run();
            if (game && args.rogomatic)
            {
                sdl_rogue->SaveGame(game->name + GetTimestamp() + ".sav", false);
            }
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
