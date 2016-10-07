//Cursor motion stuff to simulate a "no refresh" version of curses

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>

#include "rogue.h"
#include "curses.h"
#include "main.h"
#include "misc.h"
#include "display_interface.h"
#include "mach_dep.h"

//Globals for curses
#define BX_UL               0
#define BX_UR               1
#define BX_LL               2
#define BX_LR               3
#define BX_VW               4
#define BX_HT               5
#define BX_HB               6

#define MAXATTR  17

namespace 
{
    const byte color_attr[] =
    {
      7, // 0 normal
      2, // 1 green
      3, // 2 cyan
      4, // 3 red
      5, // 4 magenta
      6, // 5 brown
      8, // 6 dark grey
      9, // 7 light blue
      10, // 8 light green
      12, // 9 light red
      13, //10 light magenta
      14, //11 yellow
      15, //12 uline
      1, //13 blue
      112, //14 reverse
      15, //15 high intensity
      112, //16 bold
      0  //no more
    };

    const byte monoc_attr[] =
    {
      7, // 0 normal
      7, // 1 green
      7, // 2 cyan
      7, // 3 red
      7, // 4 magenta
      7, // 5 brown
      7, // 6 dark grey
      7, // 7 light blue
      7, // 8 light green
      7, // 9 light red
      7, //10 light magenta
      7, //11 yellow
      17, //12 uline
      7, //13 blue
      120, //14 reverse
      7, //15 white/hight
      120, //16 bold
      0  //no more
    };

    const byte dbl_box[BX_SIZE] = { 0xc9, 0xbb, 0xc8, 0xbc, 0xba, 0xcd, 0xcd };
    const byte sng_box[BX_SIZE] = { 0xda, 0xbf, 0xc0, 0xd9, 0xb3, 0xc4, 0xc4 };
    const byte fat_box[BX_SIZE] = { 0xdb, 0xdb, 0xdb, 0xdb, 0xdb, 0xdf, 0xdc };
    const byte spc_box[BX_SIZE] = { 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20 };
}

struct Curses
{
    void clear();

    void putchr(int c, int attr);

    //Turn cursor on and off
    bool cursor(bool ison);

    //get current cursor position
    void getrc(int *r, int *c);

    void clrtoeol();

    void mvaddstr(int r, int c, const char *s);

    void mvaddch(int r, int c, char chr);

    int mvinch(int r, int c);

    int addch(byte chr);

    void addstr(const char *s);

    void set_attr(int bute);

    void error(int mline, char *msg, int a1, int a2, int a3, int a4, int a5);

    //winit(win_name): initialize window -- open disk window -- determine type of monitor -- determine screen memory location for dma
    void winit(bool narrow);

    void forcebw();

    //wdump(windex): dump the screen off to disk, the window is saved so that it can be retrieved using windex
    void wdump();

    //wrestor(windex): restore the window saved on disk
    void wrestor();

    //Some general drawing routines
    void box(int ul_r, int ul_c, int lr_r, int lr_c);

    void vbox(const byte box[BX_SIZE], int ul_r, int ul_c, int lr_r, int lr_c);

    //center a string according to how many columns there really are
    void center(int row, const char *string);

    void printw(const char *msg, ...);

    void scroll_up(int start_row, int end_row, int nlines);

    void scroll_dn(int start_row, int end_row, int nlines);

    void scroll();

    //blot_out region (upper left row, upper left column) (lower right row, lower right column)
    void blot_out(int ul_row, int ul_col, int lr_row, int lr_col);

    void repchr(int chr, int cnt);

    //try to fixup screen after we get a control break
    void fixup();

    //Clear the screen in an interesting fashion
    void implode();

    //drop_curtain: Close a door on the screen and redirect output to the temporary buffer
    void drop_curtain();

    void raise_curtain();

    void move(short y, short x);

    char curch();

public:
    Curses(std::shared_ptr<DisplayInterface> output);
    ~Curses();

    int lines() const;
    int columns() const;

    void stop_rendering();
    void resume_rendering();

private:
    void Render();
    void Render(Region rect);
    void ApplyMove();
    void ApplyCursor();

    //screen size
    short LINES = 25;
    short COLS = 80;

    //points to either color or monochrom attribute table
    const byte *at_table;

    //cursor position
    short m_row;
    short m_col;

    int m_attr = 0x7;
    bool m_cursor = false;
    bool m_curtain_down = false;
    bool m_should_render = true;

    CharInfo* m_buffer = 0;
    CharInfo* m_backup = 0;

    std::shared_ptr<DisplayInterface> m_screen;
    bool disable_render = false;
};

void Curses::putchr(int c, int attr)
{
    CharInfo ci;
    ci.Attributes = attr;
    ci.Char.AsciiChar = c;
    m_buffer[m_row*COLS+m_col] = ci;
    if (!disable_render)
        Render({ m_col, m_row, m_col, m_row });
}

Curses::Curses(std::shared_ptr<DisplayInterface> output) :
    m_screen(output)
{
}

Curses::~Curses()
{
    delete[] m_buffer;
    delete[] m_backup;
}

//clear screen
void Curses::clear()
{
    blot_out(0, 0, LINES - 1, COLS - 1);
}

//Turn cursor on and off
bool Curses::cursor(bool enable)
{
    bool was_enabled = m_cursor;
    m_cursor = enable;
    if (m_cursor)
        ApplyMove();
    if(enable != was_enabled)
        ApplyCursor();
    return was_enabled;
}

//get curent cursor position
void Curses::getrc(int *r, int *c)
{
    *r = m_row;
    *c = m_col;
}

//clrtoeol
void Curses::clrtoeol()
{
    int r, c;
    getrc(&r, &c);
    blot_out(r, c, r, COLS - 1);
}

void Curses::mvaddstr(int r, int c, const char *s)
{
    move(r, c);
    addstr(s);
}

void Curses::mvaddch(int r, int c, char chr)
{
    move(r, c);
    addch(chr);
}

//todo: get rid of entirely
int Curses::mvinch(int r, int c)
{
    move(r, c);
    return curch();
}

//put the character on the screen and update the character position
int Curses::addch(byte chr)
{
    int r, c;
    byte old_attr;

    old_attr = m_attr;
    if (at_table == color_attr)
    {
        //if it is inside a room
        if (m_attr == 7) switch (chr)
        {
        case DOOR: case VWALL: case HWALL: case ULWALL: case URWALL: case LLWALL: case LRWALL:
            m_attr = 6; //brown
            break;
        case FLOOR:
            m_attr = 10; //light green
            break;
        case STAIRS:
            m_attr = 160; //black on green
            break;
        case TRAP:
            m_attr = 5; //magenta
            break;
        case GOLD: case PLAYER:
            m_attr = 14; //yellow
            break;
        case POTION: case SCROLL: case STICK: case ARMOR: case AMULET: case RING: case WEAPON:
            m_attr = 9;
            break;
        case FOOD:
            m_attr = 4;
            break;
        }
        //if inside a passage or a maze
        else if (m_attr == 112) switch (chr)
        {
        case FOOD:
            m_attr = 116; //red
            break;
        case GOLD: case PLAYER:
            m_attr = 126; //yellow on white
            break;
        case POTION: case SCROLL: case STICK: case ARMOR: case AMULET: case RING: case WEAPON:
            m_attr = 113; //blue on white
            break;
        }
        else if (m_attr == 15 && chr == STAIRS) m_attr = 160;
    }
    getrc(&r, &c);
    if (chr == '\n')
    {
        if (r == LINES - 1) { scroll_up(0, LINES - 1, 1); move(LINES - 1, 0); }
        else move(r + 1, 0);
        m_attr = old_attr;
        return m_row;
    }
    putchr(chr, m_attr);
    move(r, c + 1);
    m_attr = old_attr;
    //if you have gone off the screen scroll the whole window
    return (m_row);
}

void Curses::addstr(const char *s)
{
    bool was_disabled = disable_render;
    disable_render = true;

    int i;
    for (i = 0; s[i]; ++i)
    {
        addch(s[i]);
    }
    disable_render = was_disabled;
    if (!disable_render) {
        if (i > 0) {
            Region r;
            r.Top = m_row;
            r.Bottom = m_row;
            r.Left = m_col - i;
            r.Right = m_col - 1;
            Render(r);
        }
    }
}

void Curses::set_attr(int bute)
{
    if (bute < MAXATTR) m_attr = at_table[bute];
    else m_attr = bute;
}

void Curses::error(int mline, char *msg, int a1, int a2, int a3, int a4, int a5)
{
    int row, col;

    getrc(&row, &col);
    move(mline, 0);
    clrtoeol();
    printw(msg, a1, a2, a3, a4, a5);
    move(row, col);
}

//winit(win_name): initialize window
void Curses::winit(bool narrow_screen)
{
    LINES = 25;
    COLS = narrow_screen ? 40 : 80;
    at_table = color_attr;

    m_screen->SetDimensions({ COLS, LINES });
    m_screen->SetCursor(false);

    m_buffer = new CharInfo[LINES*COLS];
    m_backup = new CharInfo[LINES*COLS];
    memset(m_buffer, ' ', sizeof(CharInfo)*LINES*COLS);

    move(m_row, m_col);
}

void Curses::forcebw()
{
    at_table = monoc_attr;
}

//wdump(windex): dump the screen off to disk, the window is saved so that it can be retrieved using windex
void Curses::wdump()
{
    memcpy(m_backup, m_buffer, sizeof(CharInfo)*LINES*COLS);
}

//wrestor(windex): restore the window saved on disk
void Curses::wrestor()
{
    memcpy(m_buffer, m_backup, sizeof(CharInfo)*LINES*COLS);
    Render();
}

//Some general drawing routines
void Curses::box(int ul_r, int ul_c, int lr_r, int lr_c)
{
    vbox(dbl_box, ul_r, ul_c, lr_r, lr_c);
}

//box: draw a box given the upper left coordinate and the lower right
void Curses::vbox(const byte box[BX_SIZE], int ul_r, int ul_c, int lr_r, int lr_c)
{
    bool was_disabled = disable_render;
    disable_render = true;

    int i;
    bool wason;
    int r, c;

    wason = cursor(false);
    getrc(&r, &c);
    //draw horizontal boundary
    move(ul_r, ul_c + 1);
    repchr(box[BX_HT], i = (lr_c - ul_c - 1));
    move(lr_r, ul_c + 1);
    repchr(box[BX_HB], i);
    //draw vertical boundary
    for (i = ul_r + 1; i < lr_r; i++) { mvaddch(i, ul_c, box[BX_VW]); mvaddch(i, lr_c, box[BX_VW]); }
    //draw corners
    mvaddch(ul_r, ul_c, box[BX_UL]);
    mvaddch(ul_r, lr_c, box[BX_UR]);
    mvaddch(lr_r, ul_c, box[BX_LL]);
    mvaddch(lr_r, lr_c, box[BX_LR]);
    move(r, c);
    cursor(wason);

    disable_render = was_disabled;
    if (!disable_render)
        Render({ ul_c, ul_r, lr_c, lr_r });
}

//center a string according to how many columns there really are
void Curses::center(int row, const char *string)
{
    mvaddstr(row, (COLS - strlen(string)) / 2, string);
}

void Curses::printw(const char *format, ...)
{
    char dest[1024 * 16];
    va_list argptr;
    va_start(argptr, format);
    vsprintf(dest, format, argptr);
    va_end(argptr);

    addstr(dest);
}

void Curses::scroll_up(int start_row, int end_row, int nlines)
{
    move(end_row, m_col);
}

void Curses::scroll_dn(int start_row, int end_row, int nlines)
{
    move(start_row, m_col);
}

void Curses::scroll()
{
    scroll_up(0, 24, 1);
}

//blot_out region (upper left row, upper left column) (lower right row, lower right column)
void Curses::blot_out(int ul_row, int ul_col, int lr_row, int lr_col)
{
    bool was_disabled = disable_render;
    disable_render = true;

    int r, c;
    for (r = ul_row; r <= lr_row; ++r)
    {
        for (c = ul_col; c <= lr_col; ++c)
        {
            move(r, c);
            putchr(' ', m_attr);
        }
    }
    move(ul_row, ul_col);
    disable_render = was_disabled;
    if (!disable_render)
        Render({ ul_col, ul_row, lr_col, lr_row });
}

void Curses::repchr(int chr, int cnt)
{
    bool was_disabled = disable_render;
    disable_render = true;

    Region r = { m_col, m_row, m_col + cnt - 1, m_row };
    while (cnt-- > 0) { 
        putchr(chr, m_attr);
        m_col++;
    }
    disable_render = was_disabled;
    if (!disable_render)
        Render(r);
}

//try to fixup screen after we get a control break
void Curses::fixup()
{
    blot_out(m_row, m_col, m_row, m_col + 1);
}

//Clear the screen in an interesting fashion
void Curses::implode()
{
    int j, r, c, cinc = COLS / 10 / 2, er, ec;

    er = (COLS == 80 ? LINES - 3 : LINES - 4);
    //If the curtain is down, just clear the memory
    for (r = 0, c = 0, ec = COLS - 1; r < 10; r++, c += cinc, er--, ec -= cinc)
    {
        vbox(sng_box, r, c, er, ec);
        sleep(25);
        for (j = r + 1; j <= er - 1; j++)
        {
            move(j, c + 1); repchr(' ', cinc - 1);
            move(j, ec - cinc + 1); repchr(' ', cinc - 1);
        }
        vbox(spc_box, r, c, er, ec);
    }
}

//drop_curtain: Close a door on the screen and redirect output to the temporary buffer
void Curses::drop_curtain()
{
    int r;
    cursor(false);
    green();
    vbox(sng_box, 0, 0, LINES - 1, COLS - 1);
    yellow();
    for (r = 1; r < LINES - 1; r++)
    {
        move(r, 1);
        repchr(0xb1, COLS - 2);
        sleep(20);
    }
    move(0, 0);
    standend();
    m_curtain_down = true;
}

void Curses::raise_curtain()
{
    m_curtain_down = false;

    for (int r = LINES - 2; r > 0; r--)
    {
        Render({1, r, COLS-2, r});
        sleep(20);
    }
    Render();
}

void Curses::move(short y, short x)
{
    m_row = y;
    m_col = x;
    if(m_cursor)
        ApplyMove();
}

char Curses::curch()
{
    return m_buffer[m_row*COLS+m_col].Char.AsciiChar;
}

int Curses::lines() const
{
    return LINES;
}

int Curses::columns() const
{
    return COLS;
}

void Curses::stop_rendering()
{
    m_should_render = false;
}

void Curses::resume_rendering()
{
    m_should_render = true;
    ApplyMove();
    Render();
    ApplyCursor();
}

void Curses::Render()
{
    if (!m_should_render || m_curtain_down)
        return;

    m_screen->Draw(m_buffer);
}

void Curses::Render(Region rect)
{
    if (!m_should_render || m_curtain_down)
        return;

    m_screen->Draw(m_buffer, rect);
}

void Curses::ApplyMove()
{
    if (!m_should_render || m_curtain_down)
        return;

    m_screen->MoveCursor({ m_col, m_row });
}

void Curses::ApplyCursor()
{
    if (!m_should_render || m_curtain_down)
        return;

    m_screen->SetCursor(m_cursor);
}

OutputShim::OutputShim(std::shared_ptr<DisplayInterface> output) 
    : m_curses(new Curses(output))
{
}

OutputShim::~OutputShim()
{
}

void OutputShim::clear()
{
    m_curses->clear();
}

bool OutputShim::cursor(bool ison)
{
    return m_curses->cursor(ison);
}

void OutputShim::getrc(int * r, int * c)
{
    m_curses->getrc(r, c);
}

void OutputShim::clrtoeol()
{
    m_curses->clrtoeol();
}

void OutputShim::addstr(const char * s)
{
    m_curses->addstr(s);
}

void OutputShim::set_attr(int bute)
{
    m_curses->set_attr(bute);
}

void OutputShim::winit(bool narrow)
{
    m_curses->winit(narrow);
}

void OutputShim::forcebw()
{
    m_curses->forcebw();
}

void OutputShim::wdump()
{
    m_curses->wdump();
}

void OutputShim::wrestor()
{
    m_curses->wrestor();
}

void OutputShim::box(int ul_r, int ul_c, int lr_r, int lr_c)
{
    m_curses->box(ul_r, ul_c, lr_r, lr_c);
}

void OutputShim::center(int row, const char * string)
{
    m_curses->center(row, string);
}

void OutputShim::printw(const char * format, ...)
{
    char msg[1024 * 16];
    va_list argptr;
    va_start(argptr, format);
    vsprintf(msg, format, argptr);
    va_end(argptr);

    m_curses->printw(msg);
}

void OutputShim::blot_out(int ul_row, int ul_col, int lr_row, int lr_col)
{
    m_curses->blot_out(ul_row, ul_col, lr_row, lr_col);
}

void OutputShim::repchr(int chr, int cnt)
{
    m_curses->repchr(chr, cnt);
}

void OutputShim::implode()
{
    m_curses->implode();
}

void OutputShim::drop_curtain()
{
    m_curses->drop_curtain();
}

void OutputShim::raise_curtain()
{
    m_curses->raise_curtain();
}

void OutputShim::move(short y, short x)
{
    m_curses->move(y, x);
}

char OutputShim::curch()
{
    return m_curses->curch();
}

void OutputShim::add_text(Coord p, byte c)
{
    m_curses->mvaddch(p.y, p.x, c);
}

int OutputShim::add_text(byte c)
{
    return m_curses->addch(c);
}

void OutputShim::add_tile(Coord p, byte c)
{
    m_curses->mvaddch(p.y, p.x, c);
}

int OutputShim::add_tile(byte c)
{
    return m_curses->addch(c);
}

int OutputShim::mvinch(Coord p)
{
    return m_curses->mvinch(p.y, p.x);
}

void OutputShim::mvaddstr(Coord p, const std::string & s)
{
    m_curses->mvaddstr(p.y, p.x, s.c_str());
}

int OutputShim::lines() const
{
    return m_curses->lines();
}

int OutputShim::columns() const
{
    return m_curses->columns();
}

void OutputShim::stop_rendering()
{
    m_curses->stop_rendering();
}

void OutputShim::resume_rendering()
{
    m_curses->resume_rendering();
}
