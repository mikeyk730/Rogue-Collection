extern "C" {
#include "curses.h"
}
#include <memory>
#include <cstdarg>
#include <cctype>

struct Coord { int x, y; };

typedef unsigned char datatype;

struct __window
{
    __window(int lines, int cols, int begin_y, int begin_x);
    __window(__window* p, int lines, int cols, int begin_y, int begin_x);
    ~__window();

    int addch(chtype ch);
    int addstr(const char* s);
    int clear();
    int clrtoeol();
    int getcury();
    int getcurx();
    int getmaxy();
    int getmaxx();
    chtype inch();
    int move(int r, int c);
    int set_attr(int i);
    int mvwin(int r, int c);
    int refresh();

private:
    datatype get_data(int r, int c);
    void set_data(int r, int c, datatype ch);
    datatype** data();
    Coord data_coords(int r, int c);

    Coord origin = { 0, 0 };
    Coord dimensions = { 0, 0 };

    datatype** m_data = 0;
    int row = 0;
    int col = 0;
    int attr = 0;

    __window* parent = 0;
};

namespace
{
    const int s_baudrate = 3000;
}

WINDOW* stdscr;
WINDOW* curscr;

int COLS;
int LINES;

__window::__window(int lines, int cols, int begin_y, int begin_x)
{
    dimensions = { cols, lines };
    origin = { begin_x, begin_y };

    m_data = new datatype*[lines];
    for (int i = 0; i < lines; ++i)
        m_data[i] = new datatype[cols];

    clear();
}

__window::__window(__window * p, int lines, int cols, int begin_y, int begin_x)
{
    dimensions = { cols, lines };
    origin = { begin_x, begin_y };

    parent = p;
}

__window::~__window()
{
    for (int i = 0; i < dimensions.y; ++i)
        delete m_data[i];
    delete m_data;
}

int __window::addch(chtype ch)
{
    //todo:apply addr
    set_data(row, col, ch);
    col++;
    return OK;
}

int __window::addstr(const char * s)
{
    while (*s)
        addch(*s++);
    return OK;
}

int __window::clear()
{
    for (int r = 0; r < dimensions.y; ++r)
        for (int c = col; c < dimensions.x; ++c)
            set_data(r, c, ' ');
    return OK;
}

int __window::clrtoeol()
{
    for (int c = col; c < dimensions.x; ++c)
        set_data(row, c, 0);
    return OK;
}

int __window::getcury()
{
    return row;
}

int __window::getcurx()
{
    return col;
}

int __window::getmaxy()
{
    return dimensions.y;
}

int __window::getmaxx()
{
    return dimensions.x;
}

chtype __window::inch()
{
    return get_data(row, col);
}

int __window::move(int r, int c)
{
    row = r;
    col = c;
    return OK;
}

int __window::set_attr(int i)
{
    //todo: handle attr
    return OK;
}

int __window::mvwin(int r, int c)
{
    origin = { c, r };
    return OK;
}

int __window::refresh()
{
    for (int r = origin.y; r < origin.y + dimensions.y; ++r)
        for (int c = origin.x; c < origin.x + dimensions.x; ++c)
            curscr->m_data[r][c] = get_data(r, c);
    return 0;
}

datatype __window::get_data(int r, int c)
{
    Coord o = data_coords(r,c);
    return data()[o.y][o.x];
}

void __window::set_data(int r, int c, datatype ch)
{
    Coord o = data_coords(r, c);
    data()[o.y][o.x] = ch;
}

datatype** __window::data()
{
    return parent ? parent->m_data : m_data;
}

Coord __window::data_coords(int r, int c)
{
    Coord p = { c, r };
    if (parent) {
        p.x += origin.x - parent->origin.x;
        p.x += origin.y - parent->origin.y;
    }
    return p;
}

WINDOW* initscr(void)
{
    LINES = 25;
    COLS = 80;

    stdscr = new __window(LINES, COLS, 0, 0);
    curscr = new __window(LINES, COLS, 0, 0);

    return stdscr;
}

WINDOW* newwin(int nlines, int ncols, int begin_y, int begin_x)
{
    return new WINDOW(nlines, ncols, begin_y, begin_x);
}

WINDOW* subwin(WINDOW* w, int nlines, int ncols, int begin_y, int begin_x)
{
    return new WINDOW(w, nlines, ncols, begin_y, begin_x);
}

int	delwin(WINDOW* w)
{
    delete w;
    if (w == curscr || w == stdscr) {
        curscr = 0;
        stdscr = 0;
    }
    return OK;
}


int mvwin(WINDOW* w, int r, int c)
{
    return w->mvwin(r,c);
}

int waddch(WINDOW* w, chtype ch)
{
    return w->addch(ch);
}

int waddrawch(WINDOW* w, chtype ch)
{
    return w->addch(ch);
}

int waddstr(WINDOW* w, const char * s)
{
    return w->addstr(s);
}

int wclear(WINDOW* w)
{
    return w->clear();
}

int werase(WINDOW* w)
{
    return w->clear();
}

int wclrtoeol(WINDOW* w)
{
    return w->clrtoeol();
}

int getcury(WINDOW* w)
{
    return w->getcury();
}

int getcurx(WINDOW* w)
{
    return w->getcurx();
}

int getmaxy(WINDOW* w)
{
    return w->getmaxy();
}

int getmaxx(WINDOW* w)
{
    return w->getmaxx();
}

chtype winch(WINDOW* w)
{
    return w->inch();
}

int wmove(WINDOW* w, int r, int c)
{
    return w->move(r, c);
}

int wprintw(WINDOW * w, const char * f, ...)
{
    char buf[1024 * 16];
    va_list argptr;
    va_start(argptr, f);
    vsprintf_s(buf, f, argptr);
    va_end(argptr);

    return w->addstr(buf);
}

int wrefresh(WINDOW *w)
{
    return w->refresh();
}

int wstandend(WINDOW* w)
{
    return w->set_attr(0);
}

int wstandout(WINDOW* w)
{
    return w->set_attr(0x70);
}

int mvwaddch(WINDOW * w, int r, int c, chtype ch)
{
    wmove(w, r, c);
    return waddch(w, ch);
}

int mvwaddrawch(WINDOW * w, int r, int c, chtype ch)
{
    wmove(w, r, c);
    return waddrawch(w, ch);
}

int mvwaddstr(WINDOW* w, int r, int c, const char *s)
{
    wmove(w, r, c);
    return waddstr(w, s);
}

chtype mvwinch(WINDOW *w, int r, int c)
{
    wmove(w, r, c);
    return winch(w);
}

int mvwprintw(WINDOW * w, int r, int c, const char * f, ...)
{
    char buf[1024 * 16];
    va_list argptr;
    va_start(argptr, f);
    vsprintf_s(buf, f, argptr);
    va_end(argptr);

    wmove(w, r, c);
    return waddstr(w, buf);
}

int	addch(chtype ch)
{
    return waddch(stdscr, ch);
}

int	addrawch(chtype ch)
{
    return waddrawch(stdscr, ch);
}

int addstr(const char* s)
{
    return waddstr(stdscr, s);
}

int clear(void)
{
    return wclear(stdscr);
}

int	clrtoeol(void)
{
    return wclrtoeol(stdscr);
}

chtype inch(void)
{
    return winch(stdscr);
}

int	move(int r, int c)
{
    return wmove(stdscr, r, c);
}

int	mvaddch(int r, int c, chtype ch)
{
    return mvwaddch(stdscr, r, c, ch);
}

int	mvaddrawch(int r, int c, chtype ch)
{
    return mvwaddrawch(stdscr, r, c, ch);
}

int mvaddstr(int r, int c, const char* s)
{
    return mvwaddstr(stdscr, r, c, s);
}

chtype mvinch(int r, int c)
{
    return mvwinch(stdscr, r, c);
}

int	mvprintw(int r, int c, const char* f, ...)
{
    char buf[1024 * 16];
    va_list argptr;
    va_start(argptr, f);
    vsprintf_s(buf, f, argptr);
    va_end(argptr);

    return mvwaddstr(stdscr, r, c, buf);
}

int printw(const char* f, ...)
{
    char buf[1024 * 16];
    va_list argptr;
    va_start(argptr, f);
    vsprintf_s(buf, f, argptr);
    va_end(argptr);

    return waddstr(stdscr, buf);
}

int refresh(void)
{
    return wrefresh(stdscr);
}

int	standend(void)
{
    return wstandend(stdscr);
}

int	standout(void)
{
    return wstandout(stdscr);
}

int baudrate(void)
{
    return s_baudrate;
}










char *unctrl(chtype ch)
{
    static char chstr[9];

    if (isspace(ch))
        strcpy_s(chstr, " ");
    else if (!isprint(ch))
        if (ch < ' ')
            sprintf_s(chstr, "^%c", ch + '@');
        else
            sprintf_s(chstr, "\\x%x", ch);
    else {
        chstr[0] = (unsigned char)ch;
        chstr[1] = 0;
    }
    return chstr;
}

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

#define CTRL(ch)  (ch&037)

static struct xlate
{
    unsigned char keycode, keyis;
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

#undef getch
#undef ungetch
#include <conio.h>

int my_getch()
{
    return _getch();
}

int getkey()
{
    struct xlate *x;
    int key = my_getch();
    if (key != 0 && key != 0xE0) return key;

    key = my_getch();
    for (x = xtab; x < xtab + (sizeof xtab) / sizeof *xtab; x++)
    {
        if (key == x->keycode)
        {
            return x->keyis;
        }
    }
    return 0;
}




//todo input interface
int wgetch(WINDOW*)
{
    return getkey();
}

int wgetnstr(WINDOW *, char* s, int n)
{
    s[0] = my_getch();
    s[1] = 0;
    return OK;
}




//todo: 

int     init_color(short, short, short, short)
{
    return OK;
}

int     init_pair(short, short, short)
{
    return OK;
}


int start_color(void)
{
    return OK;
}

int     curs_set(int)
{
    return OK;
}

int     attroff(chtype) 
{
    return OK;
}

int     attron(chtype)
{
    return OK;
}


_bool isendwin(void)
{
    return true;
}

int raw(void)
{
    return 0;
}

char killchar(void)
{
    return 0;
}

int mvcur(int, int, int, int)
{
    return 0;
}

int endwin(void)
 {
    return 0;
}

char erasechar(void)
{
    return 0;
}

int flushinp(void)
{
    return OK;
}

int nocbreak(void)
{
    return 0;
}

int noecho(void)
{
    return 0;
}

int halfdelay(int)
{
    return 0;
}

int	clearok(WINDOW *, _bool)
{
    return 0;
}

int keypad(WINDOW *, _bool)
{
    return OK;
}

int leaveok(WINDOW *, _bool)
{
    return 0;
}

int touchwin(WINDOW *)
{
    return 0;
}

int idlok(WINDOW *, _bool)
{
    return 0;
}

int     nodelay(WINDOW *, _bool)
{
    return OK;
}