#include "args.h"

Args LoadArgs(int argc, char**argv)
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
