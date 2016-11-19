#pragma once
#include <memory>

#define BX_SIZE 7

struct OutputInterface
{
    virtual void clear() = 0;

    //Turn cursor on and off
    virtual bool cursor(bool ison) = 0;

    //get current cursor position
    virtual void getrc(int *r, int *c) = 0;

    virtual void clrtoeol() = 0;
    virtual void mvaddstr(int r, int c, const char *s) = 0;
    virtual void mvaddch(int r, int c, char chr) = 0;
    virtual int mvinch(int r, int c) = 0;
    virtual void addstr(const char *s) = 0;
    virtual void set_attr(int bute) = 0;

    //winit(win_name): initialize window -- open disk window -- determine type of monitor -- determine screen memory location for dma
    virtual void winit(bool narrow) = 0;

    virtual void forcebw() = 0;

    //wdump(windex): dump the screen off to disk, the window is saved so that it can be retrieved using windex
    virtual void wdump() = 0;

    //wrestor(windex): restore the window saved on disk
    virtual void wrestor() = 0;

    //Some general drawing routines
    virtual void box(int ul_r, int ul_c, int lr_r, int lr_c) = 0;

    //center a string according to how many columns there really are
    virtual void center(int row, const char *string) = 0;

    virtual void printw(const char *msg, ...) = 0;
    virtual void scroll_up(int start_row, int end_row, int nlines) = 0;
    virtual void scroll_dn(int start_row, int end_row, int nlines) = 0;
    virtual void scroll() = 0;

    //blot_out region (upper left row, upper left column) (lower right row, lower right column)
    virtual void blot_out(int ul_row, int ul_col, int lr_row, int lr_col) = 0;

    virtual void repchr(int chr, int cnt) = 0;

    //Clear the screen in an interesting fashion
    virtual void implode() = 0;

    //drop_curtain: Close a door on the screen and redirect output to the temporary buffer
    virtual void drop_curtain() = 0;
    virtual void raise_curtain() = 0;

    virtual void move(short y, short x) = 0;
    virtual unsigned char curch() = 0;

    virtual int lines() const = 0;
    virtual int columns() const = 0;

    virtual void add_text(short y, short x, unsigned char c) = 0;
    virtual int add_text(unsigned char c) = 0;

    virtual void add_tile(short y, short x, unsigned char c) = 0;
    virtual int add_tile(unsigned char c) = 0;

    virtual void stop_rendering() = 0;
    virtual void resume_rendering() = 0;

    virtual void play_sound(const char* id) = 0;
};

struct DisplayInterface;

std::shared_ptr<OutputInterface> CreateScreenOutput(std::shared_ptr<DisplayInterface> display);