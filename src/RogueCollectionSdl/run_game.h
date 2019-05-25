#pragma once
#ifdef _WIN32 //todo:support linux
#include <Windows.h>
#else
#include <unistd.h>
#include <dlfcn.h>
#endif

typedef int(*game_main)(int, char**, char**);
typedef void(*init_game)(DisplayInterface*, InputInterface*, int lines, int cols);

#ifdef _WIN32 //todo:support linux
struct LibraryDeleter
{
    typedef HMODULE pointer;
    void operator()(HMODULE h) { FreeLibrary(h); }
};
#endif

template <typename T>
void RunGame(const std::string& lib, int argc, char** argv, T* r)
{
#ifdef _WIN32 //todo:support linux
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
#else
    void* handle = dlopen(lib.c_str(), RTLD_LAZY);
    try {
        if (!handle) {
            throw_error("Couldn't load library: " + lib);
        }

        init_game Init = (init_game)dlsym(handle, "init_game");
        if (!Init) {
            throw_error("Couldn't load init_game from: " + lib);
        }

        game_main game = (game_main)dlsym(handle, "rogue_main");
        if (!game) {
            throw_error("Couldn't load rogue_main from: " + lib);
        }

        (*Init)(r->Display(), r->Input(), r->GameEnv()->Lines(), r->GameEnv()->Columns());
        (*game)(0, 0, environ);
        r->PostQuit();
    }
#endif
    catch (const std::runtime_error& e)
    {
        std::string s(e.what());
        DisplayMessage(SDL_MESSAGEBOX_ERROR, "Fatal Error", s.c_str());
        exit(1);
    }
}
