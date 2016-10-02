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

//main: The main program, of course
int main(int argc, char **argv)
{
    int seed = get_seed();
    g_random = new Random(seed);

    std::string filename;
    //filename = "tests\\empty.sav";
    filename = "saves\\level9.sav";
    //filename = "invis_bug.sav";
    if (argc > 1)
        filename = argv[1];
    
    try {
        if (!filename.empty()) {
            game = new GameState(g_random, filename);
        }
        else {
            game = new GameState(seed);
            setenv("rogue.opt");
        }
    }
    catch (const std::runtime_error& e)
    {
        std::cout << e.what() << std::endl;
        exit(1);
    }
    
    load_monster_cfg(game->get_environment("monsterfile"));

    game->screen().winit(game->options.narrow_screen());
    if (game->options.monochrome())
        game->screen().forcebw();

    credits();

    init_things(); //Set up probabilities of things    
    setup();
    game->screen().drop_curtain();
    game->level().new_level(false); //Draw current level
    //Start up daemons and fuses
    daemon(doctor, 0);
    fuse(swander, 0, WANDER_TIME);
    daemon(stomach, 0);
    daemon(runners, 0);
    msg("Hello %s%s.", game->hero().get_name().c_str(), noterse(".  Welcome to the Dungeons of Doom"));
    game->screen().raise_curtain();

    playit();
    delete game;
}

//endit: Exit the program abnormally.
void endit()
{
    fatal("Ok, if you want to exit that badly, I'll have to allow it\n");
}

//rnd: Pick a very random number.
int rnd(int range)
{
    return g_random->rnd(range);
}

int get_seed()
{
    int t = (int)time(0);
    srand(t);
    return t;
}

//roll: Roll a number of dice
int roll(int number, int sides)
{
    int dtotal = 0;
    while (number--)
        dtotal += rnd(sides) + 1;
    return dtotal;
}

//playit: The main loop of the program.  Loop until the game is over, refreshing things and looking at the proper times.
void playit()
{
    game->oldpos = game->hero().m_position;
    game->oldrp = get_room_from_position(game->hero().m_position);

    while (true)
        advance_game();

    endit();
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
