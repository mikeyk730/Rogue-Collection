#pragma once
#include <memory>
#include <string>

struct DisplayInterface;
struct IExCurses;
struct Coord;

struct OutputShim
{
public:
    OutputShim(std::shared_ptr<DisplayInterface> output);
    ~OutputShim();

    void clear();

    //Turn cursor on and off
    bool cursor(bool ison);

    //get current cursor position
    void getrc(int *r, int *c);

    void clrtoeol();

    void addstr(const char *s);

    void set_attr(int bute);

    //winit(win_name): initialize window -- open disk window -- determine type of monitor -- determine screen memory location for dma
    void winit(bool narrow);

    void forcebw();

    //wdump(windex): dump the screen off to disk, the window is saved so that it can be retrieved using windex
    void wdump();

    //wrestor(windex): restore the window saved on disk
    void wrestor();

    //Some general drawing routines
    void box(int ul_r, int ul_c, int lr_r, int lr_c);

    //center a string according to how many columns there really are
    void center(int row, const char *string);

    void printw(const char* format, ...);

    //blot_out region (upper left row, upper left column) (lower right row, lower right column)
    void blot_out(int ul_row, int ul_col, int lr_row, int lr_col);

    void repchr(int chr, int cnt);

    //Clear the screen in an interesting fashion
    void implode();

    //drop_curtain: Close a door on the screen and redirect output to the temporary buffer
    void drop_curtain();

    void raise_curtain();

    void move(short y, short x);

    char curch();

    int lines() const;
    int columns() const;

    void stop_rendering();
    void resume_rendering();

    void add_text(Coord p, unsigned char c);
    int add_text(unsigned char c);

    void add_tile(Coord p, unsigned char c);
    int add_tile(unsigned char c);

    int mvinch(Coord p);
    void mvaddstr(Coord p, const std::string& s);

    std::unique_ptr<IExCurses> m_curses;
};
