#pragma once
#include <atomic>
#include <stdexcept>
#ifdef _WIN32
#include <Windows.h>
#else
#include <unistd.h>
#include <dlfcn.h>
#endif
#include <display_interface.h>
#include <input_interface.h>
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

template <typename T>
void RunGame(const std::string& lib, int, char**, T* r, std::atomic<bool>& finished, const std::string& rogomatic_version)
{
    try {
#ifdef _WIN32
        std::unique_ptr<HMODULE, LibraryDeleter> dll(LoadLibraryA(lib.c_str()));

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
#else
        std::string path = "./" + lib;
        void* handle = dlopen(path.c_str(), RTLD_LAZY);

        if (!handle) {
            throw_error("Couldn't load library: " + lib + "\n" + dlerror());
        }

        init_game Init = (init_game)dlsym(handle, "init_game");
        if (!Init) {
            throw_error("Couldn't load init_game from: " + lib + "\n" + dlerror());
        }

        game_main game = (game_main)dlsym(handle, "rogue_main");
        if (!game) {
            throw_error("Couldn't load rogue_main from: " + lib + "\n" + dlerror());
        }
#endif

        (*Init)(r->Display(), r->Input(), r->Lines(), r->Columns());
        const char* argv[6] = { "player.exe", "ipc.txt", "0", "0,0,1,1,0,0,0,0", "Mikey", rogomatic_version.c_str() }; //todo:mdk
        bool rogomatic = lib == "Rogomatic_Player.dll";
        (*game)(rogomatic ? 6 : 0, rogomatic ? (char**)argv : nullptr, environ);

    }
    catch (const std::runtime_error& e)
    {
        std::string s(e.what());
        DisplayMessage("Error", "Fatal Error", s.c_str());
    }
    finished = true;
    QuitApplication();
}
