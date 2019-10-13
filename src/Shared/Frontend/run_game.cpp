#pragma once
#ifdef _WIN32
#include <Windows.h>
#else
#include <unistd.h>
#include <dlfcn.h>
#endif
#include <string>
#include <memory>
#include "args.h"
#include "display_interface.h"
#include "input_interface.h"
#include "environment.h"
#include "utility.h"

typedef int(*game_main)(int, char**, char**);
typedef void(*init_game)(DisplayInterface*, InputInterface*, int lines, int cols);

#ifdef _WIN32
struct LibraryDeleter
{
    typedef HMODULE pointer;
    void operator()(HMODULE h) { FreeLibrary(h); }
};
#endif

void RunGame(const std::string& lib, DisplayInterface* display, InputInterface* input, Environment* environment, const Args& args)
{
    try {
#ifdef _WIN32
        std::unique_ptr<HMODULE, LibraryDeleter> library(LoadLibrary(lib.c_str()));
        if (!library) {
            throw_error("Couldn't load dll: " + lib);
        }

        init_game Init = (init_game)GetProcAddress(library.get(), "init_game");
        if (!Init) {
            throw_error("Couldn't load init_game from: " + lib);
        }

        game_main game = (game_main)GetProcAddress(library.get(), "rogue_main");
        if (!game) {
            throw_error("Couldn't load rogue_main from: " + lib);
        }
#else
        std::string path = "./" + lib;
        void* library = dlopen(path.c_str(), RTLD_LAZY);
        if (!library) {
            throw_error("Couldn't load library: " + lib + "\n" + dlerror());
        }

        init_game Init = (init_game)dlsym(library, "init_game");
        if (!Init) {
            throw_error("Couldn't load init_game from: " + lib + "\n" + dlerror());
        }

        game_main game = (game_main)dlsym(library, "rogue_main");
        if (!game) {
            throw_error("Couldn't load rogue_main from: " + lib + "\n" + dlerror());
        }
#endif

        (*Init)(display, input, environment->Lines(), environment->Columns());

        int frogue = atoi(args.frogue_fd.c_str());
        int trogue = atoi(args.trogue_fd.c_str());
        std::string rogomatic_fds = "aa";
        rogomatic_fds[0] += frogue;
        rogomatic_fds[1] += trogue;

        const char* argv[5] = {
            "player.exe", 
            rogomatic_fds.c_str(),
            "0",
            "0,0,1,1,0,0,0,0", 
            "Mikey", 
        }; //todo:mdk
        bool rogomatic = lib == "Rogomatic_Player.dll";
        (*game)(rogomatic ? 5 : 0, rogomatic ? (char**)argv : 0, environ);
    }
    catch (const std::runtime_error& e)
    {
        display->DisplayMessage(e.what());
        exit(1);
    }
}
