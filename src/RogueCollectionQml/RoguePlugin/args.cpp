#include "args.h"

bool LoadArg(Args& a, const std::string& arg, const std::string& next)
{
    if (arg == "--restore" || arg == "-r") {
        a.savefile = "rogue.sav";
    }
    else if (arg == "--score" || arg == "-s") {
        a.print_score = true;
    }
    else if (arg == "--paused" || arg == "-p") {
        a.start_paused = true;
    }
    else if (arg == "--pause-at") {
        a.pause_at = next;
        return true;
    }
    else if (arg == "--small-screen" || arg == "-n") {
        a.small_screen = true;
    }
    else if (arg == "--graphics" || arg == "-g") {
        a.gfx = next;
        return true;
    }
    else if (arg == "--optfile" || arg == "-o") {
        a.optfile = next;
        return true;
    }
    else if (arg == "--font" || arg == "-f") {
        a.fontfile = next;
        return true;
    }
    else if (arg == "--profile") {
        //reserved for Retro Rogue
        return true;
    }
    else if (arg[0] != '-'){
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

