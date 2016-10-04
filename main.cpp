// ###   ###   ###  #   # #####
// #  # #   # #   # #   # #
// #  # #   # #   # #   # #
// ###  #   # #     #   # ###
// #  # #   # #  ## #   # #
// #  # #   # #   # #   # #
// #  #  ###   ###   ###  #####
//
//Exploring the Dungeons of Doom
//Copyright (C) 1981 by Michael Toy, Ken Arnold, and Glenn Wichman
//Copyright (C) 1983 by Mel Sibony, Jon Lane (AI Design update for the IBMPC)
//All rights reserved
//main.c      1.4 (A.I. Design) 11/28/84

#include <memory>
#include <iostream>
#include <stdlib.h>
#include <time.h>
#include <stdio.h>
#include <stdarg.h>

#include "rogue.h"
#include "game_state.h"
#include "main.h"
#include "daemons.h"
#include "daemon.h"
#include "mach_dep.h"
#include "output_interface.h"
#include "io.h"
#include "misc.h"
#include "rip.h"
#include "save.h"
#include "env.h"
#include "command.h"
#include "things.h"
#include "rings.h"
#include "sticks.h"
#include "potions.h"
#include "scrolls.h"
#include "hero.h"
#include "monsters.h"
#include "game_state.h"
#include "random.h"
#include "level.h"
#include "rooms.h"

namespace 
{
    int get_seed()
    {
        int t = (int)time(0);
        srand(t);
        return t;
    }
}

struct Args
{
    std::string savefile;
    std::string monsterfile;
    std::string optfile;
    bool show_replay;
    bool print_score;
    bool bw;
};

Args process_args(int argc, char**argv)
{
    Args a = Args();
    a.show_replay = true;
    a.optfile = "rogue.opt";

    for (int i = 1; i < argc; ++i) {
        std::string s(argv[i]);
        if (s == "/r") {
            a.savefile = "rogue.sav";
        }
        else if (s == "/bw") {
            a.bw = true;
        }
        else if (s == "/s") {
            a.print_score = true;
        }
        else if (s == "/q") {
            a.show_replay = false;
        }
        else if (s == "/m") {
            if (++i < argc)
                a.monsterfile = argv[i];
        }
        else if (s == "/o") {
            if (++i < argc)
                a.optfile = argv[i];
        }
        else {
            a.savefile = s;
        }
    }
    return a;
}

//main: The main program, of course
int main(int argc, char **argv)
{
    int seed = get_seed();
    g_random = new Random(seed);

    Args args = process_args(argc, argv);

    //args.savefile = "tests\\orc.sav";
    //args.savefile = "tests\\monster_picks_up_item.sav";
    //args.savefile = "saves\\level9.sav";
    //args.savefile = "steal_gold_v5.sav";
     
    try {
        if (!args.savefile.empty()) {
            game = new GameState(g_random, args.savefile, args.show_replay);
        }
        else {
            game = new GameState(seed);
            setenv(args.optfile.c_str());
        }
        load_monster_cfg(args.monsterfile);
    }
    catch (const std::runtime_error& e)
    {
        std::cout << e.what() << std::endl;
        exit(1);
    }

    if (args.bw)
        game->set_environment("screen", "bw");
    
    setup_screen();

    if (args.print_score) {
        score(0, 0, 0);
        return 0;
    }

    credits();

    init_things(); //Set up probabilities of things    

    game->screen().drop_curtain();
    game->level().new_level(false); //Draw current level

    //Start up daemons and fuses
    daemon(doctor, 0);
    fuse(start_wander, 0, WANDER_TIME);
    daemon(stomach, 0);
    daemon(run_monsters, 0);

    msg("Hello %s%s.", game->hero().get_name().c_str(), noterse(".  Welcome to the Dungeons of Doom"));
    game->screen().raise_curtain();

    while (true) {
        advance_game();
    }

    delete game;
}

//rnd: Pick a very random number.
int rnd(int range)
{
    return g_random->rnd(range);
}

//roll: Roll a number of dice
int roll(int number, int sides)
{
    int dtotal = 0;
    while (number--)
        dtotal += rnd(sides) + 1;
    return dtotal;
}

//do_quit: Have player make certain, then exit.
bool do_quit()
{
    int oy, ox;
    byte answer;
    static bool should_quit = false;

    //if they try to interrupt with a control C while in this routine blow them away!
    if (should_quit)
        leave();
    should_quit = true;
    reset_msg_position();
    game->screen().getrc(&oy, &ox);
    game->screen().move(0, 0);
    game->screen().clrtoeol();
    game->screen().move(0, 0);
    if (!in_small_screen_mode())
        game->screen().addstr("Do you wish to ");
    str_attr("end your quest now (%Yes/%No) ?");
    look(false);
    answer = readchar();
    if (answer == 'y' || answer == 'Y')
    {
        game->screen().clear();
        game->screen().move(0, 0);
        game->screen().printw("You quit with %u gold pieces\n", game->hero().get_purse());
        score(game->hero().get_purse(), 1, 0);
        fatal("");
    }
    else
    {
        game->screen().move(0, 0);
        game->screen().clrtoeol();
        update_status_bar();
        game->screen().move(oy, ox);
        reset_msg_position();
        game->cancel_repeating_cmd();
    }
    should_quit = false;

    return false;
}

//leave: Leave quickly, but courteously
void leave()
{
    const int LINES = game->screen().lines();
    look(false);
    game->screen().move(LINES - 1, 0);
    game->screen().clrtoeol();
    game->screen().move(LINES - 2, 0);
    game->screen().clrtoeol();
    game->screen().move(LINES - 2, 0);
    fatal("Ok, if you want to leave that badly\n");
}

//fatal: exit with a message
void fatal(char *format, ...)
{
    char dest[1024 * 16];
    va_list argptr;
    va_start(argptr, format);
    vsprintf(dest, format, argptr);
    va_end(argptr);

    game->screen().printw(dest);
    exit(0);
}
