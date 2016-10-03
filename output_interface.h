#pragma once

#include "rogue.h"
#include "curses.h"

struct OutputInterface
{
    virtual void clear() = 0;

    virtual void putchr(int c, int attr) = 0;

    //Turn cursor on and off
    virtual bool cursor(bool ison) = 0;

    //get current cursor position
    virtual void getrc(int *r, int *c) = 0;

    virtual void clrtoeol() = 0;

    virtual void mvaddstr(int r, int c, const char *s) = 0;

    virtual void mvaddch(int r, int c, char chr) = 0;

    virtual int mvinch(int r, int c) = 0;

    virtual int addch(byte chr) = 0;

    virtual void addstr(const char *s) = 0;

    virtual void set_attr(int bute) = 0;

    //virtual void error(int mline, char *msg, int a1, int a2, int a3, int a4, int a5) = 0;

    //winit(win_name): initialize window -- open disk window -- determine type of monitor -- determine screen memory location for dma
    virtual void winit(bool) = 0;

    virtual void forcebw() = 0;

    //wdump(windex): dump the screen off to disk, the window is saved so that it can be retrieved using windex
    virtual void wdump() = 0;

    //wrestor(windex): restore the window saved on disk
    virtual void wrestor() = 0;

    //Some general drawing routines
    virtual void box(int ul_r, int ul_c, int lr_r, int lr_c) = 0;

    //virtual void vbox(const byte box[BX_SIZE], int ul_r, int ul_c, int lr_r, int lr_c) = 0;

    //center a string according to how many columns there really are
    virtual void center(int row, const char *string) = 0;

    virtual void printw(const char *msg, ...) = 0;

    //virtual void scroll_up(int start_row, int end_row, int nlines) = 0;

    //virtual void scroll_dn(int start_row, int end_row, int nlines) = 0;

    virtual void scroll() = 0;

    //blot_out region (upper left row, upper left column) (lower right row, lower right column)
    virtual void blot_out(int ul_row, int ul_col, int lr_row, int lr_col) = 0;

    virtual void repchr(int chr, int cnt) = 0;

    //try to fixup screen after we get a control break
    //virtual void fixup() = 0;

    //Clear the screen in an interesting fashion
    virtual void implode() = 0;

    //drop_curtain: Close a door on the screen and redirect output to the temporary buffer
    virtual void drop_curtain() = 0;

    virtual void raise_curtain() = 0;

    virtual void move(short y, short x) = 0;

    virtual char curch() = 0;

    virtual void mvaddch(Coord p, byte c) = 0;

    virtual int lines() const = 0;
    virtual int columns() const = 0;

    //virtual bool small_screen_mode() const = 0;
 };
