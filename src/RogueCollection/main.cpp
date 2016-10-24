#include <memory>
#include <thread>
#include <fstream>
#include <SDL.h>
#include <SDL_ttf.h>
#include <Windows.h>
#include <input_interface.h>
#include <display_interface.h>
#include "sdl_rogue.h"
#include "text_provider.h"
#include "tile_provider.h"
#include "game_select.h"
#include "utility.h"
#include "environment.h"

DisplayInterface::~DisplayInterface() {}
InputInterface::~InputInterface() {}

namespace
{
    const int WINDOW_W = 640;
    const int WINDOW_H = 400;

    TileConfig pc_tiles =          { "tiles.bmp",       78, 2 };
    TileConfig atari_slime_tiles = { "atari.bmp",       78, 1 };
    TileConfig atari_snake_tiles = { "atari_snake.bmp", 78, 1 };

    TextConfig pc_text =         { "text.bmp",  { 256, 1 }, { 0x07 }, false };
    TextConfig pc_colored_text = { "text.bmp",  { 256, 1 }, { 0x07 }, true  };
    TextConfig alt_text =        { "16x16.bmp", { 16, 16 }, { 0x0F }, true  };
    TextConfig boxy_text =       { "boxy.bmp",  { 16, 16 }, { 0x0F }, true  };

    FontConfig placeholder = { "", 16 };

                                     //name          text_cfg          font_cfg      tile_cfg            unix    color  stndout animate     
    GraphicsConfig unix_gfx =        { "unix",       &pc_text,         &placeholder, 0,                  true,   false,  false,  false };
    GraphicsConfig color_unix_gfx =  { "unix_color", &pc_colored_text, &placeholder, 0,                  true,   true,   true,   true  };
    GraphicsConfig pc_gfx =          { "pc",         &pc_colored_text, &placeholder, 0,                  false,  true,   true,   true  };
    GraphicsConfig atari_slime_gfx = { "tiles",      &alt_text,        0,            &atari_slime_tiles, false,  false,  true,   false };
    GraphicsConfig atari_snake_gfx = { "tiles",      &alt_text,        0,            &atari_snake_tiles, false,  false,  true,   false };
    GraphicsConfig boxy_gfx =        { "boxy",       &boxy_text,       0,            0,                  false,  true,   true,   true  };
}

std::vector<GameConfig> s_options = {
    { "PC Rogue 1.48",    "Rogue_PC_1_48.dll", {80,25}, {40,25}, true,  false, { pc_gfx, atari_slime_gfx, boxy_gfx, unix_gfx, color_unix_gfx } },
    { "PC Rogue 1.1",     "Rogue_PC_1_48.dll", {80,25}, {40,25}, true,  false, { pc_gfx, atari_snake_gfx, boxy_gfx, unix_gfx, color_unix_gfx } },
    { "Unix Rogue 5.4.2", "Rogue_5_4_2.dll",   {80,25}, {80,24}, false, true,  { unix_gfx, color_unix_gfx, pc_gfx, atari_snake_gfx, boxy_gfx } },
    { "Unix Rogue 5.2.1", "Rogue_5_2_1.dll",   {80,25}, {70,22}, true,  true,  { unix_gfx, color_unix_gfx, pc_gfx, boxy_gfx } },
    { "Unix Rogue 3.6.3", "Rogue_3_6_3.dll",   {80,25}, {70,22}, true,  true,  { unix_gfx, color_unix_gfx, pc_gfx, boxy_gfx } },
};

namespace
{
    typedef int(*game_main)(int, char**, char**);
    typedef void(*init_game)(DisplayInterface*, InputInterface*, int lines, int cols);

    struct LibraryDeleter
    {
        typedef HMODULE pointer;
        void operator()(HMODULE h) { FreeLibrary(h); }
    };

    void RunGame(const std::string& lib, int argc, char** argv, SdlRogue* r)
    {
        std::unique_ptr<HMODULE, LibraryDeleter> dll(LoadLibrary(lib.c_str()));
        try {
            if (!dll) {
                throw_error("Couldn't load dll: " + lib);
            }

            init_game Init = (init_game)GetProcAddress(dll.get(), "init_game");
            if (!Init) {
                throw_error("Couldn't load init_game from: " + lib);
            }

            game_main game = (game_main)GetProcAddress(dll.get(), "rogue_main");
            if (!game) {
                throw_error("Couldn't load rogue_main from: " + lib);
            }

            (*Init)(r->Display(), r->Input(), r->GameEnv()->Lines(), r->GameEnv()->Columns());
            (*game)(0, 0, environ);
            r->PostQuit();
        }
        catch (const std::runtime_error& e)
        {
            std::string s(e.what());
            DisplayMessage(SDL_MESSAGEBOX_ERROR, "Fatal Error", s.c_str());
            exit(1);
        }
    }
}

struct Args
{
    std::string savefile;
    std::string optfile;
    std::string fontfile;
    std::string gfx;
    bool print_score = false;
    bool start_paused = false;
    bool small_screen = false;
};

Args ProcessArgs(int argc, char**argv)
{
    Args a = Args();
    a.optfile = "rogue.opt";

    for (int i = 1; i < argc; ++i) {
        std::string s(argv[i]);
        if (s == "/r" || s == "-r") {
            a.savefile = "rogue.sav";
        }
        else if (s == "/s" || s == "-s") {
            a.print_score = true;
        }
        else if (s == "/p" || s == "-p") {
            a.start_paused = true;
        }
        else if (s == "/n" || s == "-n") {
            a.small_screen = true;
        }
        else if (s == "/g" || s == "-g") {
            if (++i < argc)
                a.gfx = argv[i];
        }
        else if (s == "/o" || s == "-o") {
            if (++i < argc)
                a.optfile = argv[i];
        }
        else if (s == "/f" || s == "-f") {
            if (++i < argc)
                a.fontfile = argv[i];
        }
        else {
            a.savefile = s;
        }
    }
    return a;
}

int main(int argc, char** argv)
{
    Args args = ProcessArgs(argc, argv);

    std::shared_ptr<Environment> current_env(new Environment());
    current_env->LoadFromFile(args.optfile);
    if (args.small_screen)
        current_env->Set("small_screen", "true");
    if (!args.gfx.empty())
        current_env->Set("gfx", args.gfx);
    if (!args.savefile.empty())
        current_env->Set("game", args.savefile);
    if (!args.fontfile.empty())
        current_env->Set("font", args.fontfile);
    if (args.start_paused)
        current_env->Set("replay_paused", "true");

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

        std::string value;
        if (current_env->Get("font", &value)) {
            placeholder.fontfile = value;
            if (current_env->Get("font_size", &value)) {
                int size = atoi(value.c_str());
                if (size) {
                    placeholder.size = size;
                }
            }
        }
        
        int scale = INT_MAX;
        if (current_env->Get("window_scaling", &value))
        {
            scale = atoi(value.c_str());
        }
        Coord window_size = GetScaledCoord({ WINDOW_W, WINDOW_H }, scale);
        window = SDL::Scoped::Window(SDL_CreateWindow(SdlRogue::kWindowTitle, 100, 100, window_size.x, window_size.y, SDL_WINDOW_ALLOW_HIGHDPI | SDL_WINDOW_SHOWN), SDL_DestroyWindow);
        if (window == nullptr)
            throw_error("SDL_CreateWindow");

        renderer = SDL::Scoped::Renderer(SDL_CreateRenderer(window.get(), -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC), SDL_DestroyRenderer);
        if (renderer == nullptr)
            throw_error("SDL_CreateRenderer");
        SDL_RenderSetLogicalSize(renderer.get(), WINDOW_W, WINDOW_H);
        //SDL_RenderSetIntegerScale(renderer.get(), 1);

        if (current_env->Get("fullscreen", &value) && value == "true")
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
            std::thread rogue(RunGame, sdl_rogue->Options().dll_name, argc, argv, sdl_rogue.get());

            sdl_rogue->Run();

            exit(0);
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
