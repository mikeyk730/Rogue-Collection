#pragma once
#include <string>

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

Args LoadArgs(int argc, char**argv);

