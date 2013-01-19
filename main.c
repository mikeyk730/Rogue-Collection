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

#include <stdlib.h>
#include <time.h>

#include "rogue.h"
#include "main.h"
#include "daemons.h"
#include "daemon.h"
#include "chase.h"
#include "mach_dep.h"
#include "curses.h"
#include "io.h"
#include "init.h"
#include "load.h"
#include "new_leve.h"
#include "misc.h"
#include "rip.h"
#include "save.h"
#include "env.h"
#include "command.h"


#define is_key(s)  ((*s=='-')||(*s=='/'))
#define is_char(c1,c2)  ((c1==c2)||((c1+'a'-'A')==c2))
#define RN  (((seed=seed*11109L+13849L)>>16)&0xffff)

int bwflag = FALSE;
char do_force = FALSE;

#ifdef LOGFILE
int log_read, log_write;
#endif

//Permanent stack data
struct sw_regs *regs;



//main: The main program, of course
int main(int argc, char **argv)
{
  char *curarg, *savfile = 0;
  struct sw_regs _treg;
  long junk = 0L;
  int sl;

  regs = &_treg;
  dmaout((char*)&junk, 2, 0, 4);
  clock_on();
  epyx_yuck();
  init_ds();
  setenv(ENVFILE);
  //protect(find_drive());
  //Parse the screen environment variable.  if the string starts with "bw", then we force black and white mode.  If it ends with "fast" then we disable retrace checking
  if (strncmp(s_screen, "bw", 2)==0) bwflag = TRUE;
  if ((sl = strlen(s_screen))>=4 && strncmp(&s_screen[sl-4], "fast", 4)==0) do_force = TRUE;
  dnum = 0;
  while (--argc)
  {
    curarg = *(++argv);
    if (*curarg=='-' || *curarg=='/')
    {
      switch (curarg[1])
      {
        case 'R': case 'r': savfile = s_save; break;
        case 's': case 'S': winit(); noscore = TRUE; is_saved = TRUE; score(0, 0, 0); fatal(""); break;
#ifdef LOGFILE
        case 'l': log_write = -1; dnum = 100; break;
        case 'k': log_read = -1; dnum = 100; break;
#endif LOGFILE
      }
    }
    else if (savfile==0) savfile = curarg;
  }
  if (savfile==0)
  {
    savfile = 0;
    winit();
    if (bwflag) forcebw();
    if (no_check==0) no_check = do_force;
    credits();
    if (dnum==0) dnum = srand2();
    seed = dnum;
    init_player(); //Set up initial player stats
    init_things(); //Set up probabilities of things
    init_names(); //Set up names of scrolls
    init_colors(); //Set up colors of potions
    init_stones(); //Set up stone settings of rings
    init_materials(); //Set up materials of wands
    setup();
    drop_curtain();
    new_level(); //Draw current level
    //Start up daemons and fuses
    daemon(doctor, 0);
    fuse(swander, 0, WANDERTIME);
    daemon(stomach, 0);
    daemon(runners, 0);
    msg("Hello %s%s.", whoami, noterse(".  Welcome to the Dungeons of Doom"));
    raise_curtain();
  }
  playit(savfile);
}

//endit: Exit the program abnormally.
void endit()
{
  fatal("Ok, if you want to exit that badly, I'll have to allow it\n");
}

//Random number generator - adapted from the FORTRAN version in "Software Manual for the Elementary Functions" by W.J. Cody, Jr and William Waite.
long ran()
{
  seed *= 125;
  seed -= (seed/2796203)*2796203;
  return seed;
}

//rnd: Pick a very random number.
int rnd(int range)
{
  return range<1?0:((ran()+ran())&0x7fffffffl)%range;
}

int srand2()
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
    extern int iscuron;

    restore(sname);
    if (bwflag) forcebw();
    if (no_check==0) no_check = do_force;
    setup();
    iscuron = TRUE;
    cursor(FALSE);
  }
  else {oldpos.x = hero.x; oldpos.y = hero.y; oldrp = roomin(&hero);}
#ifdef ME
  is_me = (strcmp("wizard", whoami)==0);
#endif
  while (playing) command(); //Command execution
  endit();
}

//quit: Have player make certain, then exit.
void quit()
{
  int oy, ox;
  byte answer;
  static int qstate = FALSE;

  //if they try to interrupt with a control C while in this routine blow them away!
  if (qstate==TRUE) leave();
  qstate = TRUE;
  mpos = 0;
  getyx(eatme, oy, ox);
  move(0, 0);
  clrtoeol();
  move(0, 0);
  if (!terse) addstr("Do you wish to ");
  str_attr("end your quest now (%Yes/%No) ?");
  look(FALSE);
  answer = readchar();
  if (answer=='y' || answer=='Y')
  {
    clear();
    move(0, 0);
    printw("You quit with %u gold pieces\n", purse);
    score(purse, 1, 0);
    fatal("");
  }
  else
  {
    move(0, 0);
    clrtoeol();
    status();
    move(oy, ox);
    mpos = 0;
    count = 0;
  }
  qstate = FALSE;
}

//leave: Leave quickly, but courteously
void leave()
{
  look(FALSE);
  move(LINES-1, 0);
  clrtoeol();
  move(LINES-2, 0);
  clrtoeol();
  move(LINES-2, 0);
  fatal("Ok, if you want to leave that badly\n");
}

//fatal: exit with a message
void fatal(char *msg, int arg)
{
  wclose();
  printw(msg, arg);
  exit(0);
}
