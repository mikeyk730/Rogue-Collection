#pragma once
#include <Windows.h>
#include <display_interface.h>
#include <input_interface.h>
#include "utility.h"
#include <QDir>

typedef int(*game_main)(int, char**, char**);
typedef void(*init_game)(DisplayInterface*, InputInterface*, int lines, int cols);

struct LibraryDeleter
{
    typedef HMODULE pointer;
    void operator()(HMODULE h) { FreeLibrary(h); }
};

template <typename T>
void RunGame(const std::string& lib, int argc, char** argv, T* r)
{
    std::unique_ptr<HMODULE, LibraryDeleter> dll(LoadLibraryA(lib.c_str()));
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

        (*Init)(r->Display(), r->Input(), r->Lines(), r->Columns());
        (*game)(0, 0, environ);
    }
    catch (const std::runtime_error& e)
    {
        std::string s(e.what());
        DisplayMessage("Error", "Fatal Error", s.c_str());
    }
    QuitApplication();
}
