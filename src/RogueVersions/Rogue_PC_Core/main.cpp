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
#include <sstream>
#include <cstdlib>
#include <ctime>
#include <cstdio>
#include <cstdarg>

#include <display_interface.h>
#include "random.h"
#include "game_state.h"
#include "main.h"
#include "daemons.h"
#include "daemon.h"
#include "output_shim.h"
#include "io.h"
#include "misc.h"
#include "rip.h"
#include "save.h"
#include "command.h"
#include "things.h"
#include "ring.h"
#include "stick.h"
#include "potion.h"
#include "scroll.h"
#include "hero.h"
#include "monsters.h"
#include "game_state.h"
#include "random.h"
#include "level.h"
#include "rooms.h"
#include "input_interface_ex.h"
#include "mach_dep.h"

int get_seed()
{
    int t = (int)time(0);
    srand(t);
    return t;
}

//setup: Get starting setup for all games
void setup_screen()
{
    bool narrow_screen(game->options.narrow_screen());
    game->screen().winit(narrow_screen);
    if (game->options.monochrome())
        game->screen().forcebw();

    set_small_screen_mode(narrow_screen);
}

void print_credits_v11()
{
    game->screen().bold();
    game->screen().center(2, "ROGUE:  The Adventure Game");
    game->screen().lmagenta();
    game->screen().center(4, "The game of Rogue was originated by:");
    game->screen().high();
    game->screen().center(6, "Michael C. Toy");
    game->screen().standend();
    game->screen().center(7, "and");
    game->screen().high();
    game->screen().center(8, "Kenneth C.R.C. Arnold");
    game->screen().lmagenta();
    game->screen().center(10, "Adapted for the IBM PC by:");
    game->screen().high();
    game->screen().center(12, "Jon Lane");
    game->screen().lmagenta();
    game->screen().center(14, "Significant design contributions by:");
    game->screen().high();
    game->screen().center(16, "Glenn Wichman and scores of others");
    game->screen().yellow();
    game->screen().center(18, "(C) Copyright 1983");
    game->screen().high();
    game->screen().center(19, "Artificial Intelligence Design");
    game->screen().yellow();
    game->screen().center(20, "All Rights Reserved");
}

void print_credits()
{
    game->screen().bold();
    game->screen().center(2, "ROGUE:  The Adventure Game");
    game->screen().lmagenta();
    game->screen().center(4, "The game of Rogue was designed by:");
    game->screen().high();
    game->screen().center(6, "Michael Toy and Glenn Wichman");
    game->screen().lmagenta();
    game->screen().center(9, "Various implementations by:");
    game->screen().high();
    game->screen().center(11, "Ken Arnold, Jon Lane and Michael Toy");
    game->screen().lmagenta();
    game->screen().center(14, "Adapted for the IBM PC by:");
    game->screen().high();
    game->screen().center(16, "A.I. Design");
    game->screen().lmagenta();
    game->screen().yellow();
    game->screen().center(19, "(C)Copyright 1985");
    game->screen().high();
    game->screen().center(20, "Epyx Incorporated");
    game->screen().standend();
    game->screen().yellow();
    game->screen().center(21, "All Rights Reserved");
}

void credits()
{
    const int COLS = game->screen().columns();
    const int LINES = game->screen().lines();
    char tname[25];

    game->screen().cursor(false);
    game->screen().clear();
    game->screen().brown();
    game->screen().box(0, 0, LINES - 1, COLS - 1);

    if (game->options.act_like_v1_1())
        print_credits_v11();
    else
        print_credits();

    game->screen().brown();
    game->screen().move(22, 1);
    game->screen().repchr(205, COLS - 2);
    game->screen().add_text({ 0,22 }, (char)204);
    game->screen().add_text({ COLS - 1,22 }, (char)185);
    game->screen().standend();
    game->screen().mvaddstr({ 2, 23 }, "Rogue's Name? ");

    game->screen().high();
    getinfo(tname, 23);
    bool set_name(*tname && *tname != ESCAPE);
    if (set_name)
        game->hero().set_name(tname);

    if (game->options.act_like_v1_1()) {
        game->screen().high();
        game->screen().mvaddstr({ 2, 23 }, "        Hello ");
        game->screen().addstr(game->hero().get_name().c_str());
        game->screen().standend();
        pause(250);
    }
    else {
        game->screen().blot_out(23, 0, 24, COLS - 1);
        game->screen().brown();
        game->screen().add_text({ 0,22 }, (char)0xc8);
        game->screen().add_text({ COLS - 1,22 }, (char)0xbc);
        game->screen().standend();

        game->screen().drop_curtain();
    }
}

struct Args
{
    std::string savefile;
    std::string monsterfile;
    std::string optfile;
    bool hide_replay;
    bool print_score;
    bool start_paused;
    bool bw;
};

Args process_args(int argc, char**argv)
{
    Args a = Args();
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
            a.hide_replay = true;
        }
        else if (s == "/p") {
            a.start_paused = true;
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


//game_main: The main program, of course
int game_main(int argc, char **argv, std::shared_ptr<OutputInterface> output, std::shared_ptr<InputInterfaceEx> input)
{
    Args args = process_args(argc, argv);

    Options main_options;
    {
        std::ifstream in(args.optfile);
        main_options.from_file(in);
    }
    args.start_paused |= main_options.start_replay_paused();
    args.hide_replay |= main_options.hide_replay();

    //args.savefile = "etc\\tests\\updated\\all_sticks_setup.rsf";
    //args.savefile = "etc\\saves\\blevel8.rsf";
    //args.savefile = "rogue.sav";

    int seed = get_seed();
    if (getenv("SEED") != nullptr)
    {
        seed = atoi(getenv("SEED"));
    }
    g_random = new Random(seed);

    if (!args.savefile.empty()) {
        game = new GameState(g_random, args.savefile, !args.hide_replay, args.start_paused, output, input);
        game->set_logfile(main_options.get_environment("logfile"));
    }
    else {
        game = new GameState(seed, output, input);
        char* env = getenv("ROGUEOPTS");
        if (env)
        {
            std::istringstream rogueopts(env);
            game->options.from_file(rogueopts, ';');
        }
        else {
            std::ifstream in(args.optfile);
            game->options.from_file(in);
        }
        game->set_logfile(main_options.get_environment("logfile"));
        game->process_environment();
    }

    if (game->options.act_like_v1_1()) {
        set_monsters_v1_1();
        game->options.set_environment("scorefile", "roguepc11.scr");
    }
    load_monster_cfg(args.monsterfile);

    if (args.bw)
        game->options.set_environment("screen", "bw");

    setup_screen();

    if (args.print_score) {
        score(0, 0, 0);
        return 0;
    }

    game->CreateHero(game->options.get_environment("name"));
    if (game->options.prompt_for_name()) {
        credits();
    }

    init_things(); //Set up probabilities of things
    game->screen().clear();
    game->level().new_level(false); //Draw current level

    //Start up daemons and fuses
    daemon(doctor, 0);
    fuse(start_wander, 0, WANDER_TIME);
    daemon(stomach, 0);
    daemon(run_monsters, 0);

    msg("Hello %s%s.", game->hero().get_name().c_str(), noterse(".  Welcome to the Dungeons of Doom"));
    game->screen().raise_curtain();

    try {
        while (true) {
            advance_game();
        }
    }
    catch (ExitGame&) {}

    delete game;
    return 0;
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
        exit_game(0);
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
void fatal(const char *format, ...)
{
    char dest[1024 * 16];
    va_list argptr;
    va_start(argptr, format);
    vsprintf(dest, format, argptr);
    va_end(argptr);

    game->screen().printw(dest);
    exit(0);
}

DisplayInterface::~DisplayInterface() {}
InputInterfaceEx::~InputInterfaceEx() {}