extern "C" {
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
    __window() {}
    __window(int nlines, int ncols, int begin_y, int begin_x);

    std::unique_ptr<IExCurses> ptr = 0;
};

namespace
{
    WINDOW s_stdscr;
    std::function<std::shared_ptr<DisplayInterface>(Coord)> s_factory;

    //int s_baudrate = 1000; //terse
    int s_baudrate = 3000;  //no terse
}


WINDOW* stdscr;

int COLS;
int LINES;


__window::__window(int nlines, int ncols, int begin_y, int begin_x)
{
    ptr.reset(CreateCurses(s_factory({ begin_x, begin_y }), nlines, ncols));
}

void init_curses(int r, int c, std::function<std::shared_ptr<DisplayInterface>(Coord)> factory)
{
    LINES = r;
    COLS = c;

    s_factory = factory;
    s_stdscr.ptr.reset(CreateCurses(s_factory({ 0,0 }), LINES, COLS));
    stdscr = &s_stdscr;
}

void shutdown_curses()
{
    stdscr = 0;
    s_stdscr.ptr.reset();
}

int baudrate(void)
{
    return s_baudrate;
}

WINDOW* newwin(int nlines, int ncols, int begin_y, int begin_x)
{
    return new WINDOW(nlines, ncols, begin_y, begin_x);
}

int	delwin(WINDOW* w)
{
    delete w;
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

chtype winch(WINDOW* w)
{
    //mdk: doesn't return attributes
    return w->ptr->curch();
}

int wmove(WINDOW* w, int r, int c)
{
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












//todo: 



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

char* unctrl(chtype c)
{
    return nullptr;
}

int halfdelay(int)
{
    return 0;
}

WINDOW* curscr;


WINDOW* initscr(void)
{
    return nullptr;
}

WINDOW* subwin(WINDOW *, int, int, int, int)
{
    return nullptr;
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

int getcury(WINDOW *)
{
    return 0;
}

int getcurx(WINDOW *)
{
    return 0;
}

int getmaxy(WINDOW *)
{
    return 0;
}

int getmaxx(WINDOW *)
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

