#include "args.h"

Args::Args(int argc, char **argv)
{
    for (int i = 1; i < argc; ++i) {
        std::string arg(argv[i]);
        std::string next(i + 1 < argc ? argv[i + 1] : "");
        if (LoadArg(arg, next))
            ++i;
    }
}

Args::Args(std::vector<std::string> args)
{
    for (int i = 1; i < (int)args.size(); ++i) {
        std::string arg(args[i]);
        std::string next(i + 1 < (int)args.size() ? args[i + 1] : "");
        if (LoadArg(arg, next))
            ++i;
    }
}

bool Args::LoadArg(const std::string& arg, const std::string& next)
{
    if (arg == "--restore" || arg == "-r") {
        savefile = "rogue.sav";
    }
    else if (arg == "--score" || arg == "-s") {
        print_score = true;
    }
    else if (arg == "--paused" || arg == "-p") {
        start_paused = true;
    }
    else if (arg == "--rogomatic") {
        rogomatic = true;
    }
    else if (arg == "--rogomatic-player") {
        rogomatic_player = true;
    }
    else if (arg == "--trogue-fd") {
        trogue_fd = next;
        return true;
    }
    else if (arg == "--frogue-fd") {
        frogue_fd = next;
        return true;
    }
    else if (arg == "--pause-at") {
        pause_at = next;
        return true;
    }
    else if (arg == "--seed") {
        seed = next;
        return true;
    }
    else if (arg == "--genes") {
        genes = next;
        return true;
    }
    else if (arg == "--small-screen" || arg == "-n") {
        small_screen = true;
    }
    else if (arg == "--graphics" || arg == "-g") {
        gfx = next;
        return true;
    }
    else if (arg == "--optfile" || arg == "-o") {
        optfile = next;
        return true;
    }
    else if (arg == "--font" || arg == "-f") {
        fontfile = next;
        return true;
    }
    else if (arg[0] != '-' && savefile == ""){
        savefile = arg;
    }
    //else {
    //    throw std::runtime_error("Unknown arg: " + arg);
    //}

    return false;
}

int Args::GetDescriptorToRogue() const
{
    return atoi(trogue_fd.c_str());
}

int Args::GetDescriptorFromRogue() const
{
    return atoi(frogue_fd.c_str());
}

