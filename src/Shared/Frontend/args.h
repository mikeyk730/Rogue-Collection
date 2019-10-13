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
    bool rogomatic = false;
    bool rogomatic_server = false;
    std::string seed;
    std::string genes;
    bool print_score = false;
    bool start_paused = false;
    std::string pause_at;
    bool small_screen = false;
    std::string trogue_fd;
    std::string frogue_fd;
};


