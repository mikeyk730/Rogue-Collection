//Various installation dependent routines
//mach_dep.c  1.4 (A.I. Design) 12/1/84

#include <fstream>
#include <memory>
#include <cassert>
#include <stdio.h>
#include <Windows.h>
#include "rogue.h"
#include "curses.h"
#include "mach_dep.h"
#include "io.h"
#include "hero.h"
#include "game_state.h"
#include "input_interface.h"


//setup: Get starting setup for all games
void setup_screen()
{
    bool narrow_screen(game->options.narrow_screen());
    game->screen().winit(narrow_screen, {0,0});
    if (game->options.monochrome())
        game->screen().forcebw();

    set_small_screen_mode(narrow_screen);
}

//clear_typeahead_buffer: Flush typeahead for traps, etc.
void clear_typeahead_buffer()
{
    game->typeahead.clear();
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
    game->screen().mvaddstr({ 2, 23 }, "Rogue's Name? ");

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


//readchar: Return the next input character, from the macro or from the keyboard.
int readchar()
{
    byte ch;

    if (!game->typeahead.empty()) { 
        handle_key_state();
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


void beep()
{
    if (!game->in_replay())
        Beep(750, 300);
}

void tick_pause()
{
    if(!game->in_replay())
        Sleep(50);
}


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
