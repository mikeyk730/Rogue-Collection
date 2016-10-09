extern "C" {
#include <conio.h>
#include "curses.h"
}
#include <cstdarg>

#include <functional>
#include "output_shim.h"
#include "coord.h"

extern "C"
{
    void init_curses(int r, int c, std::function<std::shared_ptr<DisplayInterface>(Coord)> output);
    void shutdown_curses();
}

struct __window
{
    __window(int nlines, int ncols, int begin_y, int begin_x);

    std::shared_ptr<IExCurses> ptr = 0;
    Coord origin = { 0, 0 };
    Coord dimensions = { 0, 0 };
    bool subwindow = false;
};

namespace
{
    std::shared_ptr<WINDOW> s_stdscr;
    std::function<std::shared_ptr<DisplayInterface>(Coord)> s_factory;

    //int s_baudrate = 1000; //terse
    int s_baudrate = 3000;  //no terse
}


WINDOW* stdscr;
WINDOW* curscr;

int COLS;
int LINES;


__window::__window(int nlines, int ncols, int begin_y, int begin_x)
{
    ptr.reset(CreateCurses(s_factory({ begin_x, begin_y }), nlines, ncols));
    origin = { begin_x, begin_y };
    dimensions = { ncols, nlines };
}

void init_curses(int r, int c, std::function<std::shared_ptr<DisplayInterface>(Coord)> factory)
{
    LINES = r;
    COLS = c;
    s_factory = factory; 

    s_stdscr.reset(new WINDOW(LINES, COLS, 0, 0));
    stdscr = s_stdscr.get();

    //todo: this should be the actual physical state of the screen -- we don't have that
    curscr = stdscr;
}

void shutdown_curses()
{
    s_stdscr.reset();
}

WINDOW* initscr(void)
{
    //we've already done out init
    return stdscr;
}

WINDOW* newwin(int nlines, int ncols, int begin_y, int begin_x)
{
    return new WINDOW(nlines, ncols, begin_y, begin_x);
}

WINDOW* subwin(WINDOW* w, int nlines, int ncols, int begin_y, int begin_x)
{
    WINDOW* sub = new WINDOW(*w);
    sub->origin = { begin_x - w->origin.x, begin_y - w->origin.y};
    sub->dimensions = { ncols, nlines };
    sub->subwindow = true;
    return sub;
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

int waddch(WINDOW* w, chtype ch)
{
    w->ptr->addch(ch);
    return OK;
}

int waddstr(WINDOW* w, const char * s)
{
    w->ptr->addstr(s);
    return OK;
}

int wclear(WINDOW* w)
{
    w->ptr->clear();
    return OK;
}

int wclrtoeol(WINDOW* w)
{
    w->ptr->clrtoeol();
    return OK;
}

int getcury(WINDOW* w)
{
    int r, c;
    w->ptr->getrc(&r, &c);
    return r;
}

int getcurx(WINDOW* w)
{
    int r, c;
    w->ptr->getrc(&r, &c);
    return c;
}

int getmaxy(WINDOW* w)
{
    return w->dimensions.y;
}

int getmaxx(WINDOW* w)
{
    return w->dimensions.x;
}

chtype winch(WINDOW* w)
{
    //mdk: doesn't return attributes
    return w->ptr->curch();
}

int wmove(WINDOW* w, int r, int c)
{
    if (subwin) {
        r += w->origin.y;
        c += w->origin.x;
    }
    w->ptr->move(r, c);
    return OK;
}

int wprintw(WINDOW * w, const char * f, ...)
{
    char buf[1024 * 16];
    va_list argptr;
    va_start(argptr, f);
    vsprintf(buf, f, argptr);
    va_end(argptr);

    w->ptr->addstr(buf);
    return OK;
}

int wrefresh(WINDOW *w)
{
    w->ptr->refresh();
    return OK;
}

int wstandend(WINDOW* w)
{
    w->ptr->set_attr(0);
    return OK;
}

int wstandout(WINDOW* w)
{
    w->ptr->set_attr(14);
    return OK;
}

int mvwaddch(WINDOW * w, int r, int c, chtype ch)
{
    wmove(w, r, c);
    return waddch(w, ch);
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
    vsprintf(buf, f, argptr);
    va_end(argptr);

    wmove(w, r, c);
    return waddstr(w, buf);
}



int	addch(chtype ch)
{
    return waddch(stdscr, ch);
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
    vsprintf(buf, f, argptr);
    va_end(argptr);

    return mvwaddstr(stdscr, r, c, buf);
}

int printw(const char* f, ...)
{
    char buf[1024 * 16];
    va_list argptr;
    va_start(argptr, f);
    vsprintf(buf, f, argptr);
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
    static char chstr[9]; //Defined in curses library

    if (isspace(ch))
        strcpy(chstr, " ");
    else if (!isprint(ch))
        if (ch < ' ')
            sprintf(chstr, "^%c", ch + '@');
        else
            sprintf(chstr, "\\x%x", ch);
    else {
        chstr[0] = ch;
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

int getkey()
{
    struct xlate *x;
    int key = _getch();
    if (key != 0 && key != 0xE0) return key;

    key = _getch();
    for (x = xtab; x < xtab + (sizeof xtab) / sizeof *xtab; x++)
    {
        if (key == x->keycode)
        {
            return x->keyis;
        }
    }
    return 0;
}









//todo: 



int raw(void)
{
    return 0;
}


int getch(void)
{
    return getkey();
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

int	clearok(WINDOW *, bool)
{
    return 0;
}

void keypad(WINDOW *, bool)
{
}

int leaveok(WINDOW *, bool)
{
    return 0;
}

int mvwin(WINDOW *, int, int)
{
    return 0;
}

int touchwin(WINDOW *)
{
    return 0;
}

int idlok(WINDOW *, bool)
{
    return 0;
}

int werase(WINDOW *)
{
    return 0;
}

int wgetnstr(WINDOW *, char *, int)
{
    return 0;
}

