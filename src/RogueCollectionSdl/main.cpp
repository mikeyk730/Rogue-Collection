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
    void CreateProcessOrExit(const std::string& command, LPPROCESS_INFORMATION pi)
    {
        STARTUPINFO si;
        ZeroMemory(&si, sizeof(si));
        si.cb = sizeof(si);

        ZeroMemory(pi, sizeof(*pi));

        if (!CreateProcessA(
            NULL,
            (char*)command.c_str(),
            NULL,
            NULL,
            FALSE,
            0,
            "\0",//TODO: manage env
            NULL,
            &si,
            pi))
        {
            printf("Could not launch Rogue");
            exit(1);
        }
    }

    void CloseHandles(PROCESS_INFORMATION pi)
    {
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
    }

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
        int fdpipe[2];
        if (_pipe(fdpipe, 256, O_BINARY) == -1)
            exit(1);

        char read_fd[20];
        _itoa_s(fdpipe[0], read_fd, sizeof(read_fd), 10);
        args.trogue_fd = read_fd;

        char write_fd[20];
        _itoa_s(fdpipe[1], write_fd, sizeof(write_fd), 10);
        int pid;
        //todo:mdk: consider timing on startup with frogue, gene and seed args
        if ((pid = _spawnl(P_NOWAIT, argv[0], argv[0], "g", "--rogomatic-player", "5.2", "--trogue-fd", write_fd, NULL)) == -1)
            printf("Spawn failed");

        auto value = start(args);

        /* Wait until spawned program is done processing. */
        int termstat;
        _cwait(&termstat, pid, WAIT_CHILD);
        if (termstat & 0x0)
            printf("Child failed\n");

        _close(fdpipe[1]);
        _close(fdpipe[0]);

        return value;
    }

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

            //if (i >= 0 && args.rogomatic)
            //{
                //rogomatic_process.reset(new PROCESS_INFORMATION());

                //auto pipe_input = dynamic_cast<PipeInput*>(sdl_rogue->Input());
                //std::string pipe_write_fd = pipe_input ? std::to_string(pipe_input->GetWriteFd()) : "";

                //auto command = "RogueCollection.exe g --rogomatic-player \"" + s_options[i].name + "\" --trogue-fd " + pipe_write_fd;
                //if (!args.seed.empty())
                //    command += " --seed " + args.seed;
                //if (!args.genes.empty())
                //    command += " --genes \"" + args.genes + "\"";

                //CreateProcessOrExit(
                //    command,
                //    rogomatic_process.get());
            //}

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

    if (rogomatic_process)
    {
        WaitForSingleObject(rogomatic_process->hProcess, INFINITE);
        CloseHandles(*rogomatic_process);
    }

    renderer.release();
    window.release();
    SDL_Quit();
    return 0;
}

DisplayInterface::~DisplayInterface() {}
InputInterface::~InputInterface() {}
