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
#include <fstream>
#include <stdlib.h>
#include <time.h>
#include <stdio.h>
#include <stdarg.h>

#include "rogue.h"
#include "game_state.h"
#include "main.h"
#include "daemons.h"
#include "daemon.h"
#include "chase.h"
#include "mach_dep.h"
#include "curses.h"
#include "io.h"
#include "new_leve.h"
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

int bwflag = false;

//main: The main program, of course
int main(int argc, char **argv)
{
    int seed = get_seed();
    g_random = new Random(seed);

    //todo: process args
    bool replay = true;
    //replay = false;
    std::ifstream in("v0.sav", std::ios::binary | std::ios::in);
    game = replay ? new GameState(g_random, in) : new GameState(seed);

    //game->hero().invunerable = true;
    if (!replay)
        setenv("rogue.opt");
    if ("bw" == game->get_environment("scorefile"))
        bwflag = true;
    load_monster_cfg(game->get_environment("monstercfg"));

    winit();
    if (bwflag)
        forcebw();

    credits();
    
    init_things(); //Set up probabilities of things    
    setup();
    drop_curtain();
    new_level(false); //Draw current level
    //Start up daemons and fuses
    daemon(doctor, 0);
    fuse(swander, 0, WANDER_TIME);
    daemon(stomach, 0);
    daemon(runners, 0);
    msg("Hello %s%s.", game->hero().get_name().c_str(), noterse(".  Welcome to the Dungeons of Doom"));
    raise_curtain();

    playit(0);
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

  while (number--) dtotal += rnd(sides)+1;
  return dtotal;
}

//playit: The main loop of the program.  Loop until the game is over, refreshing things and looking at the proper times.
void playit(char *sname)
{
  if (sname)
  {
    restore_game(sname);
    if (bwflag) 
        forcebw();
    setup();
    cursor(false);
  }
  else {oldpos.x = game->hero().pos.x; oldpos.y = game->hero().pos.y; oldrp = get_room_from_position(&game->hero().pos);}
  while (playing) command(); //Command execution
  endit();
}

//quit: Have player make certain, then exit.
void quit()
{
  int oy, ox;
  byte answer;
  static bool should_quit = false;

  //if they try to interrupt with a control C while in this routine blow them away!
  if (should_quit) 
      leave();
  should_quit = true;
  msg_position = 0;
  getrc(&oy, &ox);
  move(0, 0);
  clrtoeol();
  move(0, 0);
  if (!in_small_screen_mode()) addstr("Do you wish to ");
  str_attr("end your quest now (%Yes/%No) ?");
  look(false);
  answer = readchar();
  if (answer=='y' || answer=='Y')
  {
    clear();
    move(0, 0);
    printw("You quit with %u gold pieces\n", game->hero().get_purse());
    score(game->hero().get_purse(), 1, 0);
    fatal("");
  }
  else
  {
    move(0, 0);
    clrtoeol();
    status();
    move(oy, ox);
    msg_position = 0;
    repeat_cmd_count = 0;
  }
  should_quit = false;
}

//leave: Leave quickly, but courteously
void leave()
{
  look(false);
  move(LINES-1, 0);
  clrtoeol();
  move(LINES-2, 0);
  clrtoeol();
  move(LINES-2, 0);
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

  printw(dest);
  exit(0);
}
