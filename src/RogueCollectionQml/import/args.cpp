#include "args.h"

bool LoadArg(Args& a, const std::string& arg, const std::string& next)
{
    if (arg == "/r" || arg == "-r") {
        a.savefile = "rogue.sav";
    }
    else if (arg == "/s" || arg == "-s") {
        a.print_score = true;
    }
    else if (arg == "/p" || arg == "-p") {
        a.start_paused = true;
    }
    else if (arg == "/n" || arg == "-n") {
        a.small_screen = true;
    }
    else if (arg == "/g" || arg == "-g") {
        a.gfx = next;
        return true;
    }
    else if (arg == "/o" || arg == "-o") {
        a.optfile = next;
        return true;
    }
    else if (arg == "/f" || arg == "-f") {
        a.fontfile = next;
        return true;
    }
    else {
        a.savefile = arg;
    }
    return false;
}

Args::Args(int argc, char **argv)
{
    for (int i = 1; i < argc; ++i) {
        std::string arg(argv[i]);
        std::string next(i+1 < argc ? argv[i+1] : "");
        if (LoadArg(*this, arg, next))
            ++i;
    }
}

Args::Args(std::vector<std::string> args)
{
    for (int i = 1; i < args.size(); ++i) {
        std::string arg(args[i]);
        std::string next(i+1 < args.size() ? args[i+1] : "");
        if (LoadArg(*this, arg, next))
            ++i;
    }
}

