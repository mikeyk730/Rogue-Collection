//Various installation dependent routines
//mach_dep.c  1.4 (A.I. Design) 12/1/84

#include <fstream>
#include <Windows.h>
#include <stdio.h>
#include <conio.h>

#include "rogue.h"
#include "curses.h"
#include "mach_dep.h"
#include "io.h"
#include "hero.h"

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

namespace
{
    struct InputDriver
    {
        virtual char GetNextChar() = 0;
        virtual std::string GetNextString(int size) = 0;
    };

    struct KeyboardInput : public InputDriver
    {
        virtual char GetNextChar();
        virtual std::string GetNextString(int size);
    };

    static InputDriver* s_input_driver(new KeyboardInput);
//    std::ofstream s_file("game.log", std::ios::binary | std::ios::out);
}

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

int is_direction_key(int key)
{
  return key == C_HOME
    || key == C_UP
    || key == C_PGUP
    || key == C_LEFT
    || key == C_RIGHT
    || key ==  C_END
    || key ==  C_DOWN
    || key ==  C_PGDN;
}

//setup: Get starting setup for all games
void setup()
{
  set_small_screen_mode(false);
  maxrow = 23;
  if (COLS==40) {
    maxrow = 22; 
    set_small_screen_mode(true);
  }
  set_brief_mode(in_small_screen_mode());
}

//flush_type: Flush typeahead for traps, etc.
void flush_type()
{
  typeahead = "";
}

BOOL CtrlHandler(DWORD fdwCtrlType) 
{
  return TRUE;
}

void credits()
{
  char tname[25];

  SetConsoleCtrlHandler((PHANDLER_ROUTINE)CtrlHandler, TRUE);

  cursor(false);
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
  move(22, 1);
  repchr(205, COLS - 2);
  mvaddch(22, 0, (char)204);
  mvaddch(22, COLS-1, (char)185);
  standend();
  mvaddstr(23, 2, "Rogue's Name? ");

  high();
  getinfo(tname, 23);
  if (*tname && *tname!=ESCAPE) strcpy(whoami, tname);
  set_name(whoami);

  blot_out(23, 0, 24, COLS-1);
  brown();
  mvaddch(22, 0, (char)0xc8);
  mvaddch(22, COLS-1, (char)0xbc);
  standend();
}

int getkey()
{
  struct xlate *x;
  int key = _getch();
  if (key != 0 && key != 0xE0) return key;

  key = _getch();
  if (is_shift_pressed() && is_direction_key(key))
    fastmode = !fastmode;

  for (x = xtab; x < xtab+(sizeof xtab)/sizeof *xtab; x++) 
  {
    if (key == x->keycode)
    {
      return x->keyis;
    }
  }
  return 0;
}

byte readchar_impl(){
    //while there are no characters in the type ahead buffer update the status line at the bottom of the screen
    do SIG2(); while (!_kbhit()); //Rogue spends a lot of time here
    //Now read a character and translate it if it appears in the translation table
    return getkey();
}


//readchar: Return the next input character, from the macro or from the keyboard.
int readchar()
{
  byte ch;

  if (*typeahead) {SIG2(); return(*typeahead++);}
  
  ch = s_input_driver->GetNextChar();

  if (ch==ESCAPE) count = 0;
  return ch;
}

int getinfo(char *str, int size)
{
    std::string s = s_input_driver->GetNextString(size);
    strcpy_s(str, size, s.c_str());
    return s[0]; //todo
}

bool is_caps_lock_on()
{
  return LOBYTE(GetKeyState(VK_CAPITAL)) != 0;
}

bool is_scroll_lock_on()
{
  return LOBYTE(GetKeyState(VK_SCROLL)) != 0;
}

bool is_num_lock_on()
{
  return LOBYTE(GetKeyState(VK_NUMLOCK)) != 0;
}

bool is_shift_pressed()
{
  return (GetAsyncKeyState(VK_LSHIFT) & 0x8000)
    || (GetAsyncKeyState(VK_RSHIFT) & 0x8000);
}

void beep()
{
  Beep(750, 300);
}

void tick_pause()
{
  Sleep(50);
}

char KeyboardInput::GetNextChar() { 
    return readchar_impl(); 
}

std::string KeyboardInput::GetNextString(int size) { 
    char buf[512];
    getinfo_impl(buf, size-1);
    return buf; 
}

