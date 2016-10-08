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
        0x07, // 0 normal
        0x02, // 1 green
        0x03, // 2 cyan
        0x04, // 3 red
        0x05, // 4 magenta
        0x06, // 5 brown
        0x08, // 6 dark grey
        0x09, // 7 light blue
        0x0a, // 8 light green
        0x0c, // 9 light red
        0x0d, //10 light magenta
        0x0e, //11 yellow
        0x0f, //12 uline
        0x01, //13 blue
        0x70, //14 reverse
        0x0f, //15 high intensity
        0x70, //16 bold
        0     //no more
    };

    const byte monoc_attr[] =
    {
        0x07, // 0 normal
        0x07, // 1 green
        0x07, // 2 cyan
        0x07, // 3 red
        0x07, // 4 magenta
        0x07, // 5 brown
        0x07, // 6 dark grey
        0x07, // 7 light blue
        0x07, // 8 light green
        0x07, // 9 light red
        0x07, //10 light magenta
        0x07, //11 yellow
        0x07, //12 uline
        0x07, //13 blue
        0x78, //14 reverse
        0x07, //15 white/hight
        0x78, //16 bold
        0     //no more
    };

    const byte dbl_box[BX_SIZE] = { 0xc9, 0xbb, 0xc8, 0xbc, 0xba, 0xcd, 0xcd };
    const byte sng_box[BX_SIZE] = { 0xda, 0xbf, 0xc0, 0xd9, 0xb3, 0xc4, 0xc4 };
    const byte fat_box[BX_SIZE] = { 0xdb, 0xdb, 0xdb, 0xdb, 0xdb, 0xdf, 0xdc };
    const byte spc_box[BX_SIZE] = { 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20 };
}

struct ICurses
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
    virtual void error(int mline, char *msg, int a1, int a2, int a3, int a4, int a5) = 0;

    //winit(win_name): initialize window -- open disk window -- determine type of monitor -- determine screen memory location for dma
    virtual void winit(bool narrow) = 0;

    virtual void forcebw() = 0;

    //wdump(windex): dump the screen off to disk, the window is saved so that it can be retrieved using windex
    virtual void wdump() = 0;

    //wrestor(windex): restore the window saved on disk
    virtual void wrestor() = 0;

    //Some general drawing routines
    virtual void box(int ul_r, int ul_c, int lr_r, int lr_c) = 0;
    virtual void vbox(const byte box[BX_SIZE], int ul_r, int ul_c, int lr_r, int lr_c) = 0;

    //center a string according to how many columns there really are
    virtual void center(int row, const char *string) = 0;

    virtual void printw(const char *msg, ...) = 0;
    virtual void scroll_up(int start_row, int end_row, int nlines) = 0;
    virtual void scroll_dn(int start_row, int end_row, int nlines) = 0;
    virtual void scroll() = 0;

    //blot_out region (upper left row, upper left column) (lower right row, lower right column)
    virtual void blot_out(int ul_row, int ul_col, int lr_row, int lr_col) = 0;

    virtual void repchr(int chr, int cnt) = 0;

    //try to fixup screen after we get a control break
    virtual void fixup() = 0;

    //Clear the screen in an interesting fashion
    virtual void implode() = 0;

    //drop_curtain: Close a door on the screen and redirect output to the temporary buffer
    virtual void drop_curtain() = 0;
    virtual void raise_curtain() = 0;

    virtual void move(short y, short x) = 0;
    virtual char curch() = 0;

    virtual int lines() const = 0;
    virtual int columns() const = 0;
};

struct IExCurses : public ICurses
{
    virtual void add_text(short y, short x, byte c) = 0;
    virtual int add_text(byte c) = 0;

    virtual void add_tile(short y, short x, byte c) = 0;
    virtual int add_tile(byte c) = 0;

    virtual void stop_rendering() = 0;
    virtual void resume_rendering() = 0;
};

struct Curses : public IExCurses
{
    Curses(std::shared_ptr<DisplayInterface> output);
    ~Curses();

public:
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
    virtual void winit(bool narrow);

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

    virtual int lines() const;
    virtual int columns() const;

public:
    virtual void add_text(short y, short x, byte c);
    virtual int add_text(byte c);

    virtual void add_tile(short y, short x, byte c);
    virtual int add_tile(byte c);

    virtual void stop_rendering();
    virtual void resume_rendering();

private:
    void MoveAddCharacter(int r, int c, char chr, bool is_text);
    int AddCharacter(byte c, bool is_text);
    void PutCharacter(int c, int attr, bool is_text);

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

    struct Data
    {
        CharInfo* buffer = 0;
        bool* text_mask = 0;
    };
    Data m_data;
    Data m_backup_data;

    std::shared_ptr<DisplayInterface> m_screen;
    bool disable_render = false;
};

void Curses::putchr(int c, int attr)
{
    PutCharacter(c, attr, true);
}

void Curses::PutCharacter(int c, int attr, bool is_text)
{
    CharInfo ci;
    ci.Attributes = attr;
    ci.Char.AsciiChar = c;
    m_data.buffer[m_row*COLS+m_col] = ci;
    m_data.text_mask[m_row*COLS + m_col] = is_text;
    if (!disable_render)
        Render({ m_col, m_row, m_col, m_row });
}

Curses::Curses(std::shared_ptr<DisplayInterface> output) :
    m_screen(output)
{
}

Curses::~Curses()
{
    delete[] m_data.buffer;
    delete[] m_data.text_mask;
    delete[] m_backup_data.buffer;
    delete[] m_backup_data.text_mask;
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

void Curses::MoveAddCharacter(int r, int c, char chr, bool is_text)
{
    move(r, c);
    AddCharacter(chr, is_text);
}

void Curses::mvaddch(int r, int c, char chr)
{
    MoveAddCharacter(r, c, chr, true);
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
    return AddCharacter(chr, true);
}

int Curses::AddCharacter(byte chr, bool is_text)
{
    int r, c;
    byte old_attr;

    old_attr = m_attr;
    if (at_table == color_attr)
    {
        //if it is inside a room
        if (m_attr == 0x07) switch (chr)
        {
        case DOOR: case VWALL: case HWALL: case ULWALL: case URWALL: case LLWALL: case LRWALL:
            m_attr = 0x06; //brown
            break;
        case FLOOR:
            m_attr = 0x0a; //light green
            break;
        case STAIRS:
            m_attr = 0xa0; //black on light green
            break;
        case TRAP:
            m_attr = 0x05; //magenta
            break;
        case GOLD: case PLAYER:
            m_attr = 0x0e; //yellow
            break;
        case POTION: case SCROLL: case STICK: case ARMOR: case AMULET: case RING: case WEAPON:
            m_attr = 0x09; //light blue
            break;
        case FOOD:
            m_attr = 0x04; //red
            break;
        }
        //if inside a passage or a maze
        else if (m_attr == 0x70) switch (chr)
        {
        case FOOD:
            m_attr = 0x74; //red on grey
            break;
        case GOLD: case PLAYER:
            m_attr = 0x7e; //yellow on grey
            break;
        case POTION: case SCROLL: case STICK: case ARMOR: case AMULET: case RING: case WEAPON:
            m_attr = 0x71; //blue on grey
            break;
        }
        else if (m_attr == 0x0f && chr == STAIRS) 
            m_attr = 0xa0;
    }
    getrc(&r, &c);
    if (chr == '\n')
    {
        if (r == LINES - 1) { scroll_up(0, LINES - 1, 1); move(LINES - 1, 0); }
        else move(r + 1, 0);
        m_attr = old_attr;
        return m_row;
    }
    PutCharacter(chr, m_attr, is_text);
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
        AddCharacter(s[i], true);
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

    m_data.buffer = new CharInfo[LINES*COLS];
    m_data.text_mask = new bool[LINES*COLS];
    memset(m_data.buffer, ' ', sizeof(CharInfo)*LINES*COLS);
    memset(m_data.text_mask, true, sizeof(bool)*LINES*COLS);

    m_backup_data.buffer = new CharInfo[LINES*COLS];
    m_backup_data.text_mask = new bool[LINES*COLS];

    move(m_row, m_col);
}

void Curses::forcebw()
{
    at_table = monoc_attr;
}

//wdump(windex): dump the screen off to disk, the window is saved so that it can be retrieved using windex
void Curses::wdump()
{
    memcpy(m_backup_data.buffer, m_data.buffer, sizeof(CharInfo)*LINES*COLS);
    memcpy(m_backup_data.text_mask, m_data.text_mask, sizeof(bool)*LINES*COLS);
}

//wrestor(windex): restore the window saved on disk
void Curses::wrestor()
{
    memcpy(m_data.buffer, m_backup_data.buffer, sizeof(CharInfo)*LINES*COLS);
    memcpy(m_data.text_mask, m_backup_data.text_mask, sizeof(bool)*LINES*COLS);
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
    for (i = ul_r + 1; i < lr_r; i++) {
        MoveAddCharacter(i, ul_c, box[BX_VW], true);
        MoveAddCharacter(i, lr_c, box[BX_VW], true);
    }
    //draw corners
    MoveAddCharacter(ul_r, ul_c, box[BX_UL], true);
    MoveAddCharacter(ul_r, lr_c, box[BX_UR], true);
    MoveAddCharacter(lr_r, ul_c, box[BX_LL], true);
    MoveAddCharacter(lr_r, lr_c, box[BX_LR], true);
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
            PutCharacter(' ', m_attr, true);
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
        PutCharacter(chr, m_attr, true);
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
    return m_data.buffer[m_row*COLS+m_col].Char.AsciiChar;
}

void Curses::add_text(short y, short x, byte c)
{
    MoveAddCharacter(y, x, c, true);
}

int Curses::add_text(byte c)
{
    return AddCharacter(c, true);
}

void Curses::add_tile(short y, short x, byte c)
{
    MoveAddCharacter(y, x, c, false);
}

int Curses::add_tile(byte c)
{
    return AddCharacter(c, false);
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

    m_screen->Draw(m_data.buffer, m_data.text_mask);
}

void Curses::Render(Region rect)
{
    if (!m_should_render || m_curtain_down)
        return;

    m_screen->Draw(m_data.buffer, m_data.text_mask, rect);
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
    m_curses->add_text(p.y, p.x, c);
}

int OutputShim::add_text(byte c)
{
    return m_curses->add_text(c);
}

void OutputShim::add_tile(Coord p, byte c)
{
    m_curses->add_tile(p.y, p.x, c);
}

int OutputShim::add_tile(byte c)
{
    return m_curses->add_tile(c);
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
