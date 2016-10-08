extern "C" {
#include "curses.h"
}
#include <cstdarg>
#include "output_shim.h"

extern "C"
{
    void init_curses(int r, int c, std::shared_ptr<DisplayInterface> output);
    void shutdown_curses();
}

namespace
{
    IExCurses* shim = 0;
}

int COLS;
int LINES;

void init_curses(int r, int c, std::shared_ptr<DisplayInterface> output)
{
    LINES = r;
    COLS = c;
    shim = CreateCurses(output);
    shim->winit(false);
}

void shutdown_curses()
{
    delete shim;
}

int	addch(chtype c)
{
    shim->add_text(c);
    return OK;
}

int addstr(const char* s)
{
    shim->addstr(s);
    return OK;
}

int clear(void)
{
    shim->clear();
    return OK;
}

int	clrtoeol(void)
{
    shim->clrtoeol();
    return OK;
}

int	move(int r, int c)
{
    shim->move(r, c);
    return OK;
}

int	mvaddch(int r, int c, chtype ch)
{
    shim->add_text(r, c, ch);
    return OK;
}

int mvaddstr(int r, int c, const char* s)
{
    shim->mvaddstr(r, c, s);
    return OK;
}

int	mvprintw(int r, int c, const char* f, ...)
{
    char buf[1024 * 16];
    va_list argptr;
    va_start(argptr, f);
    vsprintf(buf, f, argptr);
    va_end(argptr);

    shim->move(r, c);
    shim->printw(buf);
    return OK;
}

int	standend(void)
{
    shim->set_attr(0);
    return OK;
}

int	standout(void)
{
    shim->set_attr(14);
    return OK;
}

int printw(const char* f, ...)
{
    char buf[1024 * 16];
    va_list argptr;
    va_start(argptr, f);
    vsprintf(buf, f, argptr);
    va_end(argptr);

    shim->printw(buf);
    return OK;
}

chtype inch(void)
{
    //mdk: doesn't return attributes
    return shim->curch();
}










//todo: 

int refresh(void)
{
    //mdk: should be required to do any drawing
    //other functions should manipuate data only
    return 0;
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

int baudrate(void)
{
    return 0;
}

char* unctrl(chtype c)
{
    return nullptr;
}

chtype mvinch(int, int)
{
    return chtype();
}

int halfdelay(int)
{
    return 0;
}



WINDOW	*curscr;
WINDOW	*stdscr;

WINDOW * initscr(void)
{
    return nullptr;
}

WINDOW * newwin(int, int, int, int)
{
    return nullptr;
}

int waddstr(WINDOW *, const char *)
{
    return 0;
}

int	clearok(WINDOW *, bool)
{
    return 0;
}

int	delwin(WINDOW *)
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

int mvwprintw(WINDOW *, int, int, const char *, ...)
{
    return 0;
}

WINDOW * subwin(WINDOW *, int, int, int, int)
{
    return nullptr;
}

int touchwin(WINDOW *)
{
    return 0;
}

int wclear(WINDOW *)
{
    return 0;
}

int wclrtoeol(WINDOW *)
{
    return 0;
}

int wmove(WINDOW *, int, int)
{
    return 0;
}

int wprintw(WINDOW *, const char *, ...)
{
    return 0;
}

int wrefresh(WINDOW *)
{
    return 0;
}

int idlok(WINDOW *, bool)
{
    return 0;
}

int waddch(WINDOW *, chtype)
{
    return 0;
}

chtype mvwinch(WINDOW *, int, int)
{
    return chtype();
}

int mvwaddch(WINDOW *, int, int, chtype)
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
