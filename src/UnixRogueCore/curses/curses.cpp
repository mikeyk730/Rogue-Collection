extern "C" {
#include "curses.h"
}
#include <cstdarg>
#include "output_shim.h"
#include "coord.h"

namespace
{
    OutputShim* shim = 0;
}

int COLS;
int LINES;

void init(int r, int c, std::shared_ptr<DisplayInterface> output)
{
    LINES = r;
    COLS = c;
    shim = new OutputShim(output);
}

void shutdown()
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

int refresh(void)
{
    return 0;
}

int waddstr(WINDOW *, const char *)
{
    return 0;
}

int	mvaddch(int r, int c, chtype ch)
{
    shim->add_text({ c, r }, ch);
    return OK;
}

int mvaddstr(int r, int c, const char* s)
{
    shim->mvaddstr({ c, r }, s);
    return OK;
}

int	mvprintw(int r, int c, const char* f, ...)
{
    char buf[1024 * 16];
    va_list argptr;
    va_start(argptr, buf);
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

int	 printw(const char* f, ...)
{
    char buf[1024 * 16];
    va_list argptr;
    va_start(argptr, buf);
    vsprintf(buf, f, argptr);
    va_end(argptr);

    shim->printw(buf);
    return OK;
}

int raw(void)
{
    return 0;
}

//todo: 

WINDOW	*curscr;
WINDOW	*stdscr;

int	clearok(WINDOW *, bool)
{
    return OK;
}

int	delwin(WINDOW *)
{
    return OK;
}

void keypad(WINDOW *, bool)
{

}

char killchar(void)
{
    return 0;
}

int leaveok(WINDOW *, bool)
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

chtype inch(void)
{
    return chtype();
}

int nocbreak(void)
{
    return 0;
}

int noecho(void)
{
    return 0;
}

int idlok(WINDOW *, bool)
{
    return 0;
}

WINDOW * initscr(void)
{
    return nullptr;
}


int baudrate(void)
{
    return 0;
}

char * unctrl(chtype c)
{
    return nullptr;
}

chtype mvinch(int, int)
{
    return chtype();
}

WINDOW * newwin(int, int, int, int)
{
    return nullptr;
}

int halfdelay(int)
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
