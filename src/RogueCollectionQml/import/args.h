#pragma once
#include <string>
#include <vector>

struct Args
{
    Args(int argc, char**argv);
    Args(std::vector<std::string> args);

    std::string savefile;
    std::string optfile = "rogue.opt";
    std::string fontfile;
    std::string gfx;
    bool print_score = false;
    bool start_paused = false;
    bool small_screen = false;
};


