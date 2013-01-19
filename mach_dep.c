//Various installation dependent routines
//mach_dep.c  1.4 (A.I. Design) 12/1/84

#include <stdio.h>
#include <conio.h>

#include "rogue.h"
#include "curses.h"
#include "mach_dep.h"
#include "io.h"

#define TICK_ADDR  0x70
#define PC  0xff
#define XT  0xfe
#define JR  0xfd
#define AT  0xfc

static int clk_vec[2];
static int ocb;

#define C_LEFT    0x4b
#define C_RIGHT   0x4d
#define C_UP      0x48
#define C_DOWN    0x50
#define C_HOME    0x47
#define C_PGUP    0x49
#define C_END     0x4f
#define C_PGDN    0x51
#define C_ESCAPE  0x1b
#define C_INS     0x52
#define C_DEL     0x53
#define C_F1      0x3b
#define C_F2      0x3c
#define C_F3      0x3d
#define C_F4      0x3e
#define C_F5      0x3f
#define C_F6      0x40
#define C_F7      0x41
#define C_F8      0x42
#define C_F9      0x43
#define C_F10     0x44
#define ALT_F9    0x70


//Table for IBM extended key translation
static struct xlate
{
  byte keycode, keyis;
} xtab[] =
{
  C_HOME,  'y',
  C_UP,    'k',
  C_PGUP,  'u',
  C_LEFT,  'h',
  C_RIGHT, 'l',
  C_END,   'b',
  C_DOWN,  'j',
  C_PGDN,  'n',
  C_INS,   '>',
  C_DEL,   's',
  C_F1,    '?',
  C_F2,    '/',
  C_F3,    'a',
  C_F4,    CTRL('R'),
  C_F5,    'c',
  C_F6,    'D',
  C_F7,    'i',
  C_F8,    '^',
  C_F9,    CTRL('F'),
  C_F10,   '!',
  ALT_F9,  'F'
};

//setup: Get starting setup for all games
void setup()
{
  terse = FALSE;
  maxrow = 23;
  if (COLS==40) {maxrow = 22; terse = TRUE;}
  expert = terse;
}

void clock_on()
{
  //extern int _csval, clock(), (*cls_)(), no_clock();
  //int new_vec[2];

  //new_vec[0] = clock;
  //new_vec[1] = _csval;
  //dmain(clk_vec, 2, 0, TICK_ADDR);
  //dmaout(new_vec, 2, 0, TICK_ADDR);
  //cls_ = no_clock;
}

void no_clock()
{
  //dmaout(clk_vec, 2, 0, TICK_ADDR);
}

//flush_type: Flush typeahead for traps, etc.
void flush_type()
{
#ifdef CRASH_MACHINE
  regs->ax = 0xc06; //clear keyboard input
  regs->dx = 0xff; //set input flag
  swint(SW_DOS, regs);
#endif CRASH_MACHINE
  typeahead = "";
}

void credits()
{
  int i;
  char tname[25];

  cursor(FALSE);
  clear();
  brown();
  box(0, 0, LINES-1, COLS-1);
  bold();
  center(2, "ROGUE:  The Adventure Game");
  lmagenta();
  center(4, "The game of Rogue was designed by:");
  high();
  center(6, "Michael Toy and Glenn Wichman");
  lmagenta();
  center(9, "Various implementations by:");
  high();
  center(11, "Ken Arnold, Jon Lane and Michael Toy");
  lmagenta();
  center(14, "Adapted for the IBM PC by:");
  high();
  center(16, "A.I. Design");
  lmagenta();
  yellow();
  center(19, "(C)Copyright 1985");
  high();
  center(20, "Epyx Incorporated");
  standend();
  yellow();
  center(21, "All Rights Reserved");
  brown();
  for (i = 1; i<(COLS-1); i++) {move(22, i); putchr(205, 6);}
  mvaddch(22, 0, 204);
  mvaddch(22, COLS-1, 185);
  standend();
  mvaddstr(23, 2, "Rogue's Name? ");
  is_saved = TRUE; //status line hack
  high();
  getinfo(tname, 23);
  if (*tname && *tname!=ESCAPE) strcpy(whoami, tname);
  is_saved = FALSE;
  blot_out(23, 0, 24, COLS-1);
  brown();
  mvaddch(22, 0, 0xc8);
  mvaddch(22, COLS-1, 0xbc);
  standend();
}

int getkey()
{
  struct xlate *x;
  int key = _getch();
  if (key != 0 && key != 0xE0) return key;
   
  for (key = _getch(), x = xtab; x < xtab+(sizeof xtab)/sizeof *xtab; x++) 
  {
    if (key == x->keycode)
    {
      return x->keyis;
    }
  }
  return 0;
}

//readchar: Return the next input character, from the macro or from the keyboard.
int readchar()
{
  byte ch;

  if (*typeahead) {SIG2(); return(*typeahead++);}
  //while there are no characters in the type ahead buffer update the status line at the bottom of the screen
  do SIG2(); while (no_char()); //Rogue spends a lot of time here
  //Now read a character and translate it if it appears in the translation table
  ch = getkey();
  if (ch==ESCAPE) count = 0;
  return ch;
}

int bdos(int fnum, int dxval)
{
  //struct sw_regs *saveptr;

  //regs->ax = fnum<<8;
  //regs->bx = regs->cx = 0;
  //regs->dx = dxval;
  //saveptr = regs;
  //swint(SW_DOS, regs);
  //regs = saveptr;
  //return (0xff&regs->ax);
   return 0;
}

int swint(int intno, struct sw_regs *rp)
{
  //extern int _dsval;

  //rp->ds = rp->es = _dsval;
  //sysint(intno, rp, rp);
  //return rp->ax;
   return 0;
}

void one_tick()
{
  extern int tick;
  int otick = tick;
  int i = 0, j = 0;

  while (i++) while (j++)
  if (otick!=tick) return;
  else if (i>2) _halt();
}

#include <Windows.h>

int is_caps_lock_on()
{
   return LOBYTE(GetKeyState(VK_CAPITAL)) != 0;
}

int is_scroll_lock_on()
{
   return LOBYTE(GetKeyState(VK_SCROLL)) != 0;
}

int is_num_lock_on()
{
   return LOBYTE(GetKeyState(VK_NUMLOCK)) != 0;
}

int no_char(){ return !_kbhit(); }

