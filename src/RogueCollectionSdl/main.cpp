#include <memory>
#include <thread>
#include <fstream>
#include <io.h>
#include <fcntl.h>
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
#include "pipe_input.h"

namespace
{
    std::string GetTimestamp()
    {
        time_t now;
        time(&now);

        tm t;
        localtime_s(&t, &now);

        char str[200];
        strftime(str, 200, "%FT%H-%M-%S", &t);
        return str;
    }
}

int start(Args& args);

int main(int argc, char** argv)
{
    Args args(argc, argv);

    if (args.rogomatic) //game process
    {
        int trogue_pipe[2];
        if (_pipe(trogue_pipe, 256, O_BINARY) == -1)
            exit(1);

        args.trogue_fd = std::to_string(trogue_pipe[0]);
        std::string trogue_write_fd = std::to_string(trogue_pipe[1]);

        int frogue_pipe[2];
        if (_pipe(frogue_pipe, 65536, O_BINARY) == -1)
            exit(1);

        args.frogue_fd = std::to_string(frogue_pipe[1]);
        std::string frogue_read_fd = std::to_string(frogue_pipe[0]);

        int pid;
        if ((pid = _spawnl(
            P_NOWAIT,
            argv[0],
            argv[0],
            "g",
            "--rogomatic-player", "5.2", //todo:mdk remove hardcoding
            "--trogue-fd", trogue_write_fd.c_str(),
            "--frogue-fd", frogue_read_fd.c_str(),
            "--seed", (args.seed.empty() ? "\"\"" : args.seed.c_str()),
            "--genes", (args.genes.empty() ? "\"\"" : args.genes.c_str()),
            NULL)) == -1)
        {
            printf("Spawn failed");
        }

        auto value = start(args);

        /* Wait until spawned program is done processing. */
        int termstat;
        _cwait(&termstat, pid, WAIT_CHILD);
        if (termstat & 0x0)
        {
            printf("Child failed\n");
        }

        _close(trogue_pipe[1]);
        _close(trogue_pipe[0]);
        _close(frogue_pipe[1]);
        _close(frogue_pipe[0]);

        return value;
    }

    printf("genes:%s\n", args.genes.c_str());
    printf("seed:%s\n", args.seed.c_str());
    return start(args);
}

int start(Args& args)
{
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
    std::unique_ptr<PROCESS_INFORMATION> rogomatic_process;
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
            args.rogomatic &= s_options[i].supports_rogomatic;
            if (args.rogomatic | args.rogomatic_server)
            {
                current_env->Set("name", "rogomatic");
                current_env->Set("fruit", "apricot");
                current_env->Set("terse", "true");
                current_env->Set("jump", "true");
                current_env->Set("step", "true");
                current_env->Set("seefloor", "true");
                current_env->Set("flush", "false");
                current_env->Set("askme", "false");
                current_env->Set("passgo", "false");
                current_env->Set("inven", "slow");
                current_env->Set("showac", "");
            }

            sdl_rogue.reset(
                new SdlRogue(
                    window.get(),
                    renderer.get(),
                    current_env,
                    i,
                    args));
        }
        else if (!replay_path.empty()) {
            sdl_rogue.reset(new SdlRogue(window.get(), renderer.get(), current_env, replay_path));
        }

        if (sdl_rogue) {
            //start rogue engine on a background thread
            std::thread rogue(RunGame<SdlRogue>, sdl_rogue->Options().dll_name, 0, nullptr, sdl_rogue.get(), args);
            rogue.detach();

            sdl_rogue->Run();
            if (i >= 0 && args.rogomatic)
            {
                sdl_rogue->SaveGame(s_options[i].name + GetTimestamp() + ".sav", false);
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
