//Various installation dependent routines
//mach_dep.c  1.4 (A.I. Design) 12/1/84

#include <fstream>
#include <memory>
#include <cassert>
#include <Windows.h>
#include <stdio.h>
#include <conio.h>

#include "rogue.h"
#include "output_interface.h"
#include "mach_dep.h"
#include "io.h"
#include "hero.h"
#include "game_state.h"
#include "keyboard_input.h"

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
    byte keycode, keyis; //todo: need to translate all of these
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
        || key == C_END
        || key == C_DOWN
        || key == C_PGDN;
}

//setup: Get starting setup for all games
void setup()
{ 
    //TODO:now
    set_small_screen_mode(false);
    const int COLS = game->screen().columns();
    if (COLS == 40) {
        set_small_screen_mode(true);
    }
    set_brief_mode(in_small_screen_mode());
}

//clear_typeahead_buffer: Flush typeahead for traps, etc.
void clear_typeahead_buffer()
{
    game->typeahead.clear();
}

BOOL CtrlHandler(DWORD fdwCtrlType)
{
    return TRUE;
}

void credits()
{
    const int COLS = game->screen().columns();
    const int LINES = game->screen().lines();
    char tname[25];

    SetConsoleCtrlHandler((PHANDLER_ROUTINE)CtrlHandler, TRUE);

    game->screen().cursor(false);
    game->screen().clear();
    game->screen().brown();
    game->screen().box(0, 0, LINES - 1, COLS - 1);
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
    game->screen().brown();
    game->screen().move(22, 1);
    game->screen().repchr(205, COLS - 2);
    game->screen().mvaddch({ 0,22 }, (char)204);
    game->screen().mvaddch({ COLS - 1,22 }, (char)185);
    game->screen().standend();
    game->screen().mvaddstr(23, 2, "Rogue's Name? ");

    game->screen().high();
    getinfo(tname, 23);
    if (*tname && *tname != ESCAPE)
        game->hero().set_name(tname);
    else
        game->hero().set_name(game->get_environment("name"));

    game->screen().blot_out(23, 0, 24, COLS - 1);
    game->screen().brown();
    game->screen().mvaddch({ 0,22 }, (char)0xc8);
    game->screen().mvaddch({ COLS - 1,22 }, (char)0xbc);
    game->screen().standend();
}

int getkey()
{
    struct xlate *x;
    int key = _getch();
    if (key != 0 && key != 0xE0) return key;

    key = _getch();
    for (x = xtab; x < xtab + (sizeof xtab) / sizeof *xtab; x++)
    {
        if (key == x->keycode)
        {
            if (is_shift_pressed() && is_direction_key(key))
                return toupper(x->keyis);
            return x->keyis;
        }
    }
    return 0;
}

/*
SHORT s_keyboard[256];
int s_key;

bool my_kbhit()
{
    for (int i = 0; i <= 255; ++i) {
        s_keyboard[i] = GetAsyncKeyState(i);
    }
    for (int i = 0; i <= 255; ++i) {
        switch (i){
        case VK_SHIFT:
        case VK_CONTROL:
        case VK_MENU:
        case VK_LSHIFT:
        case VK_LCONTROL:
        case VK_LMENU:
        case VK_RSHIFT:
        case VK_RCONTROL:
        case VK_RMENU:
            continue;
        }
        if (LOBYTE(s_keyboard[i]) != 0) {
            s_key = i;
            return true;
        }
    }
    s_key = 0;
    return false;
}

char my_getkey()
{
    char key = my_translate(s_key);
    while (_kbhit()) _getch();
    if (is_shift_pressed() && is_direction_key(key))
        game->modifiers.m_fast_mode = !game->modifiers.m_fast_mode;
    return key;
}*/

/*
std::vector<int> direction_vk = { VK_PRIOR, VK_NEXT, VK_END, VK_HOME, VK_LEFT, VK_UP, VK_RIGHT, VK_DOWN };
char s_key;

char translate(char c)
{
    switch (c) {
    case VK_PRIOR:
        return 'u';
    case VK_NEXT:
        return 'n';
    case VK_END:
        return 'b';
    case VK_HOME:
        return 'y';
    case VK_LEFT:
        return 'h';
    case VK_UP:
        return 'k';
    case VK_RIGHT:
        return 'l';
    case VK_DOWN:
        return 'j';
    }
    return c;
}


bool ex_kbhit() {
    s_key = 0;
    for (auto i = direction_vk.begin(); i != direction_vk.end(); ++i) {
    //for (int i = 8; i <= 255; ++i){
        switch (*i) {
        case VK_SHIFT:
        case VK_CONTROL:
        case VK_MENU:
        case VK_LSHIFT:
        case VK_LCONTROL:
        case VK_LMENU:
        case VK_RSHIFT:
        case VK_RCONTROL:
        case VK_RMENU:
            continue;
        }
        SHORT state = GetAsyncKeyState(*i);
        if (LOBYTE(state) != 0) {
            s_key = translate(*i);
        }
    }

   if (_kbhit()) {
        s_key = 0;
        return true;
    }

    return s_key != 0;
}

char ex_getkey()
{
    if (s_key != 0)
        return s_key;
    return getkey();
}
*/

byte readchar_impl() {
    //while there are no characters in the type ahead buffer update the status line at the bottom of the screen
    do
        SIG2();
    while (!_kbhit()); //Rogue spends a lot of time here
    //Now read a character and translate it if it appears in the translation table
    return getkey();
}


//readchar: Return the next input character, from the macro or from the keyboard.
int readchar()
{
    byte ch;

    if (!game->typeahead.empty()) { 
        SIG2();
        ch = game->typeahead.back();
        game->typeahead.pop_back();
        return ch;
    }

    ch = game->input_interface().GetNextChar();

    if (ch == ESCAPE) 
        game->cancel_repeating_cmd();
    return ch;
}

int getinfo(char *str, int size)
{
    std::string s = game->input_interface().GetNextString(size);
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

bool KeyboardInput::HasMoreInput()
{
    return true;
}

char KeyboardInput::GetNextChar() {
    return readchar_impl();
}

std::string KeyboardInput::GetNextString(int size) {
    char buf[512];
    getinfo_impl(buf, size - 1);
    return buf;
}

void KeyboardInput::Serialize(std::ostream& out)
{ }

std::ostream& write_string(std::ostream& out, const std::string& s) {
    write(out, s.length());
    out.write(s.c_str(), s.length());
    return out;
}

std::istream& read_string(std::istream& in, std::string* s) {
    int length;
    read(in, &length);
    assert(length < 255);

    char buf[255];
    memset(buf, 0, 255);
    in.read(buf, length);
    *s = buf;

    return in;
}
