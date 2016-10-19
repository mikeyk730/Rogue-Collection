extern "C" {
#include "curses.h"
#undef getch
}
#include <algorithm>
#include <cstdarg>
#include "../Shared/display_interface.h"
#include "input_interface.h"

extern "C"
{
    void init_curses(DisplayInterface* screen, InputInterface* input, int lines, int cols);
    void shutdow_curses();
}

namespace
{
    const int s_baudrate = 3000;
    DisplayInterface* s_screen = 0;
    InputInterface* s_input = 0;
}

struct __window
{
    __window(int lines, int cols, int begin_y, int begin_x);
    __window(__window* p, int lines, int cols, int begin_y, int begin_x);
    ~__window();

    int addch(chtype ch);
    int addrawch(chtype ch);
    int addstr(const char* s);
    int attron(chtype);
    int attroff(chtype);
    int clear();
    int clrtoeol();
    int erase();
    int getch();
    int getcury();
    int getcurx();
    int getmaxy();
    int getmaxx();
    chtype inch();
    int move(int r, int c);
    int mvwin(int r, int c);
    int nodelay(bool enable);
    int overlay(WINDOW* dest, bool copy_spaces) const;
    int refresh();
    int getnstr(char* dest, int n);
    int chgat(int n, attr_t attr, short color, const void *opts);

private:
    std::string getsnstr_impl(unsigned int n);

    chtype get_data(int r, int c) const;
    chtype get_data_absolute(int abs_r, int abs_c) const;
    void set_data(int r, int c, chtype ch);
    void set_data_absolute(int abs_r, int abs_c, chtype ch);
    chtype* data(int row, int col) const;
    Coord data_coords(int r, int c) const;
    Region window_region() const;
    int index(int r, int c) const;

private:
    Coord origin = { 0, 0 };
    Coord dimensions = { 0, 0 };

    chtype* m_data = 0;
    chtype attr = 0;

    int row = 0;
    int col = 0;

    bool clear_screen = false;
    bool no_delay = false;

    __window* parent = 0;
};

WINDOW* stdscr = 0;
WINDOW* curscr = 0;

int COLS = 0;
int LINES = 0;

__window::__window(int lines, int cols, int begin_y, int begin_x)
{
    if (lines == 0)
        lines = LINES - begin_y;
    if (cols == 0)
        cols = COLS - begin_x;

    dimensions = { cols, lines };
    origin = { begin_x, begin_y };

    m_data = new chtype[lines*cols];

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
    delete[] m_data;
}

int __window::addch(chtype ch)
{
    ch |= attr;
    if ((ch&A_CHARTEXT) == '\n')
    {
        clrtoeol();
        ++row;
        col = 0;
    }
    else if ((ch&A_CHARTEXT) == '\b')
    {
        if (col > 0)
            --col;
        set_data(row, col, ' ');
    }
    else if ((ch&A_CHARTEXT) == '\t')
    {
        do {
            set_data(row, col, ' ');
            ++col;
        } while (col % 8);
    }
    else {
        set_data(row, col, ch);
        ++col;
    }
    return OK;
}

int __window::addrawch(chtype ch)
{
    ch |= attr;
    ch |= A_ALTCHARSET;
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

int __window::getch()
{
    if (!s_input)
        return ERR;
    int ch = s_input->GetChar(!no_delay, false, nullptr);
    return ch ? ch : ERR;
}

int flushinp(void)
{
    if (s_input)
        s_input->Flush();
    return OK;
}

char erasechar(void)
{
    return '\b';
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

std::string __window::getsnstr_impl(unsigned int n)
{
    const int ESCAPE = 0x1b;

    std::string s;
    while (true)
    {
        unsigned char c = s_input->GetChar(true, true, nullptr);
        switch (c)
        {
        case ESCAPE:
            s.clear();
            s.push_back(ESCAPE);
            return s;
        case '\b':
            if (!s.empty()) {
                addch('\b');
                s.pop_back();
            }
            break;
        default:
            if (s.size() >= n) {
                beep();
                break;
            }
            //todo: i should care about echo
            addch(c);
            s.push_back(c);
            break;
        case '\n':
        case '\r':
            return s;
        }

        refresh();
    }
}

int __window::getnstr(char* dest, int n)
{
    if (!s_input)
        return ERR;

    std::string s = getsnstr_impl(n - 1);
    strcpy(dest, s.c_str());
    return OK;
}

int __window::chgat(int n, attr_t attr, short color, const void * opts)
{
    if (n == -1)
        n = COLS - col;

    for (int c = col; c < col + n; ++c) {
        chtype ch = (get_data(row, c)&A_CHARTEXT | attr | COLOR_PAIR(color));
        set_data(row, c, ch);
    }

    return OK;
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

int __window::mvwin(int r, int c)
{
    origin = { c, r };
    return OK;
}

int __window::nodelay(bool enable)
{
    no_delay = enable;
    return OK;
}

int __window::refresh()
{
    Region region = window_region();

    if (clear_screen) {
        curscr->erase();
        region = { 0, 0, COLS - 1, LINES - 1 };
        clear_screen = false;
    }

    if (region.Top == 0 && region.Left == 0 && region.Bottom == LINES - 1 && region.Right == COLS - 1)
    {
        memcpy(curscr->m_data, m_data, LINES*COLS * sizeof(chtype));
    }
    else {
        for (int r = origin.y; r < origin.y + dimensions.y; ++r)
            for (int c = origin.x; c < origin.x + dimensions.x; ++c)
                *(curscr->data(r, c)) = get_data_absolute(r, c);
    }

    if (s_screen)
    {
        s_screen->MoveCursor({ col, row });
        s_screen->UpdateRegion(curscr->m_data, region);
    }
    return OK;
}

int __window::overlay(WINDOW * dest, bool copy_spaces) const
{
    Region r1 = window_region();
    Region r2 = dest->window_region();

    // calculate the overlap of the windows in absolute screen coordinates
    Region overlap;
    overlap.Left = std::max(r1.Left, r2.Left);
    overlap.Top = std::max(r1.Top, r2.Top);
    overlap.Right = std::min(r1.Right, r2.Right);
    overlap.Bottom = std::min(r1.Bottom, r2.Bottom);

    for (int r = overlap.Top; r <= overlap.Bottom; ++r) {
        for (int c = overlap.Left; c <= overlap.Right; ++c) {
            auto ch = get_data_absolute(r, c);
            if (copy_spaces || (ch&A_CHARTEXT) != ' ')
                dest->set_data_absolute(r, c, ch);
        }
    }
    return OK;
}

Region __window::window_region() const
{
    Region r;
    r.Left = origin.x;
    r.Top = origin.y;
    r.Right = r.Left + dimensions.x - 1;
    r.Bottom = r.Top + dimensions.y - 1;
    return r;
}

int __window::index(int r, int c) const
{
    return r*dimensions.x + c;
}

chtype __window::get_data(int r, int c) const
{
    Coord o = data_coords(r, c);
    return *data(o.y,o.x);
}

chtype __window::get_data_absolute(int abs_r, int abs_c) const
{
    return get_data(abs_r - origin.y, abs_c - origin.x);
}

void __window::set_data(int r, int c, chtype ch)
{
    Coord o = data_coords(r, c);
    *data(o.y,o.x) = ch;
}

void __window::set_data_absolute(int abs_r, int abs_c, chtype ch)
{
    set_data(abs_r - origin.y, abs_c - origin.x, ch);
}

chtype* __window::data(int r, int c) const
{
    return parent ? parent->data(r,c) : &m_data[index(r,c)];
}

Coord __window::data_coords(int r, int c) const
{
    Coord p = { c, r };
    if (parent) {
        p.x += origin.x - parent->origin.x;
        p.x += origin.y - parent->origin.y;
    }
    return p;
}

void init_curses(DisplayInterface* screen, InputInterface* input, int lines, int cols)
{
    LINES = lines;
    COLS = cols;
    s_screen = screen;
    s_input = input;
}

void shutdow_curses()
{

}

WINDOW* initscr(void)
{
    if (LINES == 0)
        LINES = 25;
    if (COLS == 0)
        COLS = 80;

    if (s_screen)
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

int overlay(const WINDOW* w, WINDOW* dest)
{
    return w->overlay(dest, false);
}

int overwrite(const WINDOW* w, WINDOW* dest)
{
    return w->overlay(dest, true);
}

int mvwin(WINDOW* w, int r, int c)
{
    return w->mvwin(r, c);
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

int wchgat(WINDOW* w, int n, attr_t attr, short color, const void* opt)
{
    return w->chgat(n, attr, color, opt);
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

int wgetnstr(WINDOW* w, char* dest, int n)
{
    return w->getnstr(dest, n);
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
    vsprintf(buf, f, argptr);
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

int mvwchgat(WINDOW* w, int r, int c, int n, attr_t attr, short color, const void* opt)
{
    wmove(w, r, c);
    return wchgat(w, n, attr, color, opt);
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

int getnstr(char* dest, int n)
{
    return wgetnstr(stdscr, dest, n);
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

int mvchgat(int r, int c, int n, attr_t attr, short color, const void* opt)
{
    return mvwchgat(stdscr, r, c, n, attr, color, opt);
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

int wgetch(WINDOW* w)
{
    return w->getch();
}

int nodelay(WINDOW* w, _bool enable)
{
    return w->nodelay(enable != 0);
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
        strcpy(chstr, " ");
    else if (!isprint(ch))
        if (ch < ' ')
            sprintf(chstr, "^%c", ch + '@');
        else
            sprintf(chstr, "\\x%x", ch);
    else {
        chstr[0] = (unsigned char)ch;
        chstr[1] = 0;
    }
    return chstr;
}

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
    return OK;
}

char killchar(void)
{
    return OK;
}

int mvcur(int, int, int, int)
{
    return OK;
}

int endwin(void)
{
    return OK;
}

int nocbreak(void)
{
    return OK;
}

int noecho(void)
{
    return OK;
}

int halfdelay(int)
{
    return OK;
}

int	clearok(WINDOW *, _bool)
{
    return OK;
}

int keypad(WINDOW *, _bool)
{
    return OK;
}

int leaveok(WINDOW *, _bool)
{
    return OK;
}

int touchwin(WINDOW *)
{
    return OK;
}

int idlok(WINDOW *, _bool)
{
    return OK;
}

int crmode(void)
{
    return OK;
}

int nocrmode(void)
{
    return OK;
}

int echo(void)
{
    return OK;
}

int beep(void)
{
    return putchar('\a');
}