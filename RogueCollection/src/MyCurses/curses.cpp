extern "C" {
#include "curses.h"
}
#include <memory>
#include <cstdarg>
#include <cctype>

#include "display_interface.h"
#include "windows_console.h"

DisplayInterface::~DisplayInterface() {}

struct __window
{
    __window(int lines, int cols, int begin_y, int begin_x);
    __window(__window* p, int lines, int cols, int begin_y, int begin_x);
    ~__window();

    int addch(chtype ch);
    int addrawch(chtype ch);
    int addstr(const char* s);
    int clear();
    int erase();
    int clrtoeol();
    int getcury();
    int getcurx();
    int getmaxy();
    int getmaxx();
    chtype inch();
    int move(int r, int c);
    int attron(chtype);
    int attroff(chtype);
    int set_attr(int i);
    int mvwin(int r, int c);
    int refresh();

private:
    chtype get_data(int r, int c);
    void set_data(int r, int c, chtype ch);
    chtype** data();
    Coord data_coords(int r, int c);

    Coord origin = { 0, 0 };
    Coord dimensions = { 0, 0 };

    chtype** m_data = 0;
    chtype attr = 0;

    int row = 0;
    int col = 0;

    bool clear_screen = false;

    __window* parent = 0;
};

namespace
{
    const int s_baudrate = 3000;
    DisplayInterface* s_screen = 0;
}

WINDOW* stdscr;
WINDOW* curscr;

int COLS;
int LINES;

__window::__window(int lines, int cols, int begin_y, int begin_x)
{
    dimensions = { cols, lines };
    origin = { begin_x, begin_y };

    m_data = new chtype*[lines];
    for (int i = 0; i < lines; ++i)
        m_data[i] = new chtype[cols];

    erase();
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
    if (ch == '\n')
    {
        clrtoeol();
        ++row;
        col = 0;
    }
    else if (ch == '\b')
    {
        set_data(row, col, ' ');
        if (col > 0) 
            --col;
    }
    else if (ch == '\t')
    {
        do {
            set_data(row, col, ' ');
            ++col;
        } while (col % 8);
    }
    else {
        return addrawch(ch);
    }
    return OK;
}

int __window::addrawch(chtype ch)
{
    //todo:apply addr
    set_data(row, col, ch);
    ++col;
    return OK;
}

int __window::addstr(const char * s)
{
    while (*s)
        addch(*s++);
    return OK;
}

int __window::attroff(chtype ch)
{
    if ((ch | A_COLOR) == (attr | A_COLOR))
    { 
        attr &= ~A_COLOR; //clear the color 
    } 
    return OK;
}

int __window::attron(chtype ch)
{
    if ((ch | A_COLOR) && (attr | A_COLOR))
    {
        attr &= ~A_COLOR; //clear the old color 
    }
    attr |= (ch & A_COLOR);
    return OK;
}

int __window::clear()
{
    erase();
    clear_screen = true;
    return OK;
}

int __window::erase()
{
    move(0, 0);
    for (int r = 0; r < dimensions.y; ++r)
        for (int c = 0; c < dimensions.x; ++c)
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
    //todo: should i return attrs?
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
    if (clear_screen) {
        curscr->erase();
        if (s_screen)
            s_screen->UpdateRegion(curscr->m_data, { 0,0,COLS - 1,LINES - 1 });
        clear_screen = false;
    }

    for (int r = origin.y; r < origin.y + dimensions.y; ++r)
        for (int c = origin.x; c < origin.x + dimensions.x; ++c)
            curscr->m_data[r][c] = get_data(r, c);

    if (s_screen)
    {
        Region r;
        r.Left = origin.x;
        r.Top = origin.y;
        r.Right = origin.x + dimensions.x - 1;
        r.Bottom = origin.y + dimensions.y - 1;

        s_screen->UpdateRegion(curscr->m_data, r);
        s_screen->MoveCursor({ col, row });
    }
    return 0;
}

chtype __window::get_data(int r, int c)
{
    Coord o = data_coords(r,c);
    return data()[o.y][o.x];
}

void __window::set_data(int r, int c, chtype ch)
{
    ch |= attr;
    Coord o = data_coords(r, c);
    data()[o.y][o.x] = ch;
}

chtype** __window::data()
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

    s_screen = new WindowsConsole({ 0, 0 });
    s_screen->SetDimensions({ COLS, LINES });

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
    return w->addrawch(ch);
}

int waddstr(WINDOW* w, const char * s)
{
    return w->addstr(s);
}

int wattroff(WINDOW* w, chtype ch)
{
    return w->attroff(ch);
}

int wattron(WINDOW* w, chtype ch)
{
    return w->attron(ch);
}

int wclear(WINDOW* w)
{
    return w->clear();
}

int werase(WINDOW* w)
{
    return w->erase();
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
    return w->attroff(COLOR_PAIR(0x70));
}

int wstandout(WINDOW* w)
{
    return w->attron(COLOR_PAIR(0x70));
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

int attroff(chtype ch)
{
    return wattroff(stdscr, ch);
}

int attron(chtype ch)
{
    return wattron(stdscr, ch);
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

int curs_set(int mode)
{
    if (s_screen)
        s_screen->SetCursor(mode != 0);
    return OK;
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

int init_color(short, short, short, short)
{
    return OK;
}

int init_pair(short, short, short)
{
    return OK;
}

int start_color(void)
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