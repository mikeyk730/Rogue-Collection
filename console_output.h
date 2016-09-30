#pragma once
#include "output_interface.h"

struct ConsoleOutput : public OutputInterface
{
    ConsoleOutput(Coord origin);

    virtual void clear();

    virtual void putchr(int c, int attr);

    //Turn cursor on and off
    virtual bool cursor(bool ison);

    //get current cursor position
    virtual void getrc(int *r, int *c);

    virtual void clrtoeol();

    virtual void mvaddstr(int r, int c, const char *s);

    virtual void mvaddch(int r, int c, char chr);

    virtual int mvinch(int r, int c);

    virtual int addch(byte chr);

    virtual void addstr(const char *s);

    virtual void set_attr(int bute);

    virtual void error(int mline, char *msg, int a1, int a2, int a3, int a4, int a5);

    //winit(win_name): initialize window -- open disk window -- determine type of monitor -- determine screen memory location for dma
    virtual void winit(bool);

    virtual void forcebw();

    //wdump(windex): dump the screen off to disk, the window is saved so that it can be retrieved using windex
    virtual void wdump();

    //wrestor(windex): restore the window saved on disk
    virtual void wrestor();

    //Some general drawing routines
    virtual void box(int ul_r, int ul_c, int lr_r, int lr_c);

    virtual void vbox(const byte box[BX_SIZE], int ul_r, int ul_c, int lr_r, int lr_c);

    //center a string according to how many columns there really are
    virtual void center(int row, const char *string);

    virtual void printw(const char *msg, ...);

    virtual void scroll_up(int start_row, int end_row, int nlines);

    virtual void scroll_dn(int start_row, int end_row, int nlines);

    virtual void scroll();

    //blot_out region (upper left row, upper left column) (lower right row, lower right column)
    virtual void blot_out(int ul_row, int ul_col, int lr_row, int lr_col);

    virtual void repchr(int chr, int cnt);

    //try to fixup screen after we get a control break
    virtual void fixup();

    //Clear the screen in an interesting fashion
    virtual void implode();

    //drop_curtain: Close a door on the screen and redirect output to the temporary buffer
    virtual void drop_curtain();

    virtual void raise_curtain();

    virtual void move(short y, short x);

    virtual char curch();

    virtual void mvaddch(Coord p, byte c);

    virtual int lines() const;
    virtual int columns() const;

    virtual bool small_screen_mode() const;

private:
    //screen size
    short LINES = 25;
    short COLS = 80;

    int ch_attr = 0x7;
    
    //points to either color or monochrom attribute table
    const byte *at_table;

    //cursor position
    short c_row;
    short c_col;

    Coord m_origin;
    Coord translated_position();

    HANDLE hConsole;
    CHAR_INFO m_buffer[MAXLINES][MAXCOLS];
};