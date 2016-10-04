//Cursor motion stuff to simulate a "no refresh" version of curses

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <Windows.h>

#include "rogue.h"
#include "console_output.h"
#include "mach_dep.h"
#include "main.h"
#include "misc.h"

#pragma warning(disable:4838)

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

void ConsoleOutput::putchr(int c, int attr)
{
    putchr_(c, attr);
    Render({ c_col, c_row, c_col, c_row });
}

void ConsoleOutput::putchr_(int c, int attr)
{
    CHAR_INFO ci;
    ci.Attributes = attr;
    ci.Char.AsciiChar = c;
    m_buffer[c_row][c_col] = ci;
}

ConsoleOutput::ConsoleOutput(Coord origin) :
    m_origin(origin)
{
    hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
}

ConsoleOutput::~ConsoleOutput()
{
    CloseHandle(hConsole);
}

//clear screen
void ConsoleOutput::clear()
{
    blot_out(0, 0, LINES - 1, COLS - 1);
}

//Turn cursor on and off
bool ConsoleOutput::cursor(bool enable)
{
    bool was_enabled = m_cursor;
    m_cursor = enable;
    if(enable != was_enabled)
        ApplyCursor();
    return was_enabled;
}

//get curent cursor position
void ConsoleOutput::getrc(int *r, int *c)
{
    *r = c_row;
    *c = c_col;
}

//clrtoeol
void ConsoleOutput::clrtoeol()
{
    int r, c;
    getrc(&r, &c);
    blot_out(r, c, r, COLS - 1);
}

void ConsoleOutput::mvaddstr(int r, int c, const char *s)
{
    move_(r, c);
    addstr(s);
}

void ConsoleOutput::mvaddch(int r, int c, char chr)
{
    move_(r, c);
    addch(chr);
}

//todo: get rid of entirely
int ConsoleOutput::mvinch(int r, int c)
{
    move_(r, c);
    return curch();
}

//put the character on the screen and update the character position
int ConsoleOutput::addch(byte chr)
{
    int r, c;
    byte old_attr;

    old_attr = ch_attr;
    if (at_table == color_attr)
    {
        //if it is inside a room
        if (ch_attr == 7) switch (chr)
        {
        case DOOR: case VWALL: case HWALL: case ULWALL: case URWALL: case LLWALL: case LRWALL:
            ch_attr = 6; //brown
            break;
        case FLOOR:
            ch_attr = 10; //light green
            break;
        case STAIRS:
            ch_attr = 160; //black on green
            break;
        case TRAP:
            ch_attr = 5; //magenta
            break;
        case GOLD: case PLAYER:
            ch_attr = 14; //yellow
            break;
        case POTION: case SCROLL: case STICK: case ARMOR: case AMULET: case RING: case WEAPON:
            ch_attr = 9;
            break;
        case FOOD:
            ch_attr = 4;
            break;
        }
        //if inside a passage or a maze
        else if (ch_attr == 112) switch (chr)
        {
        case FOOD:
            ch_attr = 116; //red
            break;
        case GOLD: case PLAYER:
            ch_attr = 126; //yellow on white
            break;
        case POTION: case SCROLL: case STICK: case ARMOR: case AMULET: case RING: case WEAPON:
            ch_attr = 113; //blue on white
            break;
        }
        else if (ch_attr == 15 && chr == STAIRS) ch_attr = 160;
    }
    getrc(&r, &c);
    if (chr == '\n')
    {
        if (r == LINES - 1) { scroll_up(0, LINES - 1, 1); move_(LINES - 1, 0); }
        else move_(r + 1, 0);
        ch_attr = old_attr;
        return c_row;
    }
    putchr(chr, ch_attr);
    move(r, c + 1);
    ch_attr = old_attr;
    //if you have gone off the screen scroll the whole window
    return (c_row);
}

void ConsoleOutput::addstr(const char *s)
{
    while (*s) addch(*s++);
}

void ConsoleOutput::set_attr(int bute)
{
    if (bute < MAXATTR) ch_attr = at_table[bute];
    else ch_attr = bute;
}

void ConsoleOutput::error(int mline, char *msg, int a1, int a2, int a3, int a4, int a5)
{
    int row, col;

    getrc(&row, &col);
    move_(mline, 0);
    clrtoeol();
    printw(msg, a1, a2, a3, a4, a5);
    move_(row, col);
}

//winit(win_name): initialize window
void ConsoleOutput::winit(bool narrow_screen)
{
    LINES = 25;
    COLS = narrow_screen ? 40 : 80;
    at_table = color_attr;
    memset(m_buffer, ' ', sizeof(m_buffer));
    move_(c_row, c_col);
}

void ConsoleOutput::forcebw()
{
    at_table = monoc_attr;
}

//wdump(windex): dump the screen off to disk, the window is saved so that it can be retrieved using windex
void ConsoleOutput::wdump()
{
    memcpy(m_backup, m_buffer, sizeof(m_buffer));
}

//wrestor(windex): restore the window saved on disk
void ConsoleOutput::wrestor()
{
    memcpy(m_buffer, m_backup, sizeof(m_buffer));
    Render();
}

//Some general drawing routines
void ConsoleOutput::box(int ul_r, int ul_c, int lr_r, int lr_c)
{
    vbox(dbl_box, ul_r, ul_c, lr_r, lr_c);
}

//box: draw a box given the upper left coordinate and the lower right
void ConsoleOutput::vbox(const byte box[BX_SIZE], int ul_r, int ul_c, int lr_r, int lr_c)
{
    int i;
    bool wason;
    int r, c;

    wason = cursor(false);
    getrc(&r, &c);
    //draw horizontal boundary
    move_(ul_r, ul_c + 1);
    repchr(box[BX_HT], i = (lr_c - ul_c - 1));
    move_(lr_r, ul_c + 1);
    repchr(box[BX_HB], i);
    //draw vertical boundary
    for (i = ul_r + 1; i < lr_r; i++) { mvaddch(i, ul_c, box[BX_VW]); mvaddch(i, lr_c, box[BX_VW]); }
    //draw corners
    mvaddch(ul_r, ul_c, box[BX_UL]);
    mvaddch(ul_r, lr_c, box[BX_UR]);
    mvaddch(lr_r, ul_c, box[BX_LL]);
    mvaddch(lr_r, lr_c, box[BX_LR]);
    move_(r, c);
    cursor(wason);
}

//center a string according to how many columns there really are
void ConsoleOutput::center(int row, const char *string)
{
    mvaddstr(row, (COLS - strlen(string)) / 2, string);
}

void ConsoleOutput::printw(const char *format, ...)
{
    char dest[1024 * 16];
    va_list argptr;
    va_start(argptr, format);
    vsprintf(dest, format, argptr);
    va_end(argptr);

    addstr(dest);
}

void ConsoleOutput::scroll_up(int start_row, int end_row, int nlines)
{
    move_(end_row, c_col);
}

void ConsoleOutput::scroll_dn(int start_row, int end_row, int nlines)
{
    move_(start_row, c_col);
}

void ConsoleOutput::scroll()
{
    scroll_up(0, 24, 1);
}

//blot_out region (upper left row, upper left column) (lower right row, lower right column)
void ConsoleOutput::blot_out(int ul_row, int ul_col, int lr_row, int lr_col)
{
    int r, c;
    for (r = ul_row; r <= lr_row; ++r)
    {
        for (c = ul_col; c <= lr_col; ++c)
        {
            move_(r, c);
            putchr_(' ', ch_attr);
        }
    }
    move_(ul_row, ul_col);
    Render({ (SHORT)ul_col, (SHORT)ul_row, (SHORT)lr_col, (SHORT)lr_row });
}

void ConsoleOutput::repchr(int chr, int cnt)
{
    SMALL_RECT r = { c_col, c_row, c_col + cnt - 1, c_row };
    while (cnt-- > 0) { 
        putchr_(chr, ch_attr);
        c_col++;
    }
    Render(r);
}

//try to fixup screen after we get a control break
void ConsoleOutput::fixup()
{
    blot_out(c_row, c_col, c_row, c_col + 1);
}

//Clear the screen in an interesting fashion
void ConsoleOutput::implode()
{
    int j, r, c, cinc = COLS / 10 / 2, er, ec;

    er = (COLS == 80 ? LINES - 3 : LINES - 4);
    //If the curtain is down, just clear the memory
    for (r = 0, c = 0, ec = COLS - 1; r < 10; r++, c += cinc, er--, ec -= cinc)
    {
        vbox(sng_box, r, c, er, ec);
        Sleep(25);
        for (j = r + 1; j <= er - 1; j++)
        {
            move_(j, c + 1); repchr(' ', cinc - 1);
            move_(j, ec - cinc + 1); repchr(' ', cinc - 1);
        }
        vbox(spc_box, r, c, er, ec);
    }
}

//drop_curtain: Close a door on the screen and redirect output to the temporary buffer
void ConsoleOutput::drop_curtain()
{
    int r;
    cursor(false);
    green();
    vbox(sng_box, 0, 0, LINES - 1, COLS - 1);
    yellow();
    for (r = 1; r < LINES - 1; r++)
    {
        move_(r, 1);
        repchr(0xb1, COLS - 2);
        Sleep(20);
    }
    move_(0, 0);
    standend();
    m_curtain_down = true;
}

void ConsoleOutput::raise_curtain()
{
    m_curtain_down = false;

    for (short r = LINES - 2; r > 0; r--)
    {
        Render({1, r, (short)COLS-2, r});
        Sleep(20);
    }
    Render();
}

void ConsoleOutput::move(short y, short x)
{
    move_(y, x);
    ApplyMove();
}

void ConsoleOutput::move_(short y, short x)
{
    c_row = y;
    c_col = x;
}

char ConsoleOutput::curch()
{
    return m_buffer[c_row][c_col].Char.AsciiChar;
}

void ConsoleOutput::mvaddch(Coord p, byte c)
{
    mvaddch(p.y, p.x, c);
}

int ConsoleOutput::mvinch(Coord p)
{
    return mvinch(p.y, p.x);
}

void ConsoleOutput::mvaddstr(Coord p, const std::string & s)
{
    mvaddstr(p.y, p.x, s.c_str());
}

int ConsoleOutput::lines() const
{
    return LINES;
}

int ConsoleOutput::columns() const
{
    return COLS;
}

bool ConsoleOutput::small_screen_mode() const
{
    return COLS == 40;
}

Coord ConsoleOutput::translated_position()
{
    return{ c_col + m_origin.x, c_row + m_origin.y };
}

void ConsoleOutput::Render()
{
    if(!should_render || m_curtain_down)
        return;

    COORD dwBufferSize = { MAXCOLS, MAXLINES };
    COORD dwBufferCoord = { 0, 0 };
    SMALL_RECT rcRegion = { m_origin.x, m_origin.y, m_origin.x + COLS - 1, m_origin.y + LINES - 1 };
    WriteConsoleOutput(hConsole, (CHAR_INFO *)m_buffer, dwBufferSize, dwBufferCoord, &rcRegion);
}

void ConsoleOutput::Render(SMALL_RECT rect)
{
    if (!should_render || m_curtain_down)
        return;

    COORD dwBufferSize = { MAXCOLS, MAXLINES };
    COORD dwBufferCoord = { rect.Left, rect.Top };
    SMALL_RECT rcRegion = { m_origin.x+rect.Left, m_origin.y+rect.Top, m_origin.x+rect.Right, m_origin.y+rect.Bottom };
    WriteConsoleOutput(hConsole, (CHAR_INFO *)m_buffer, dwBufferSize, dwBufferCoord, &rcRegion);
}

void ConsoleOutput::ApplyMove()
{
    if (!should_render || m_curtain_down)
        return;

    Coord pos = translated_position();
    COORD p = { pos.x, pos.y };
    SetConsoleCursorPosition(hConsole, p);
}

void ConsoleOutput::ApplyCursor()
{
    if (!should_render || m_curtain_down)
        return;

    CONSOLE_CURSOR_INFO info;
    info.bVisible = m_cursor;
    info.dwSize = 25;
    SetConsoleCursorInfo(hConsole, &info);
}

void ConsoleOutput::StopRendering()
{
    should_render = false;
}

void ConsoleOutput::ResumeRendering()
{
    should_render = true;
    ApplyMove();
    Render();
    ApplyCursor();
}
