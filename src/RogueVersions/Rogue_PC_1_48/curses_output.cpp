#include <output_interface.h>
#include <rogue.h>
#include <mach_dep.h>
#undef standout
#undef standend
#undef MOUSE_MOVED

#include <curses.h>

//Globals for curses
#define BX_UL               0
#define BX_UR               1
#define BX_LL               2
#define BX_LR               3
#define BX_VW               4
#define BX_HT               5
#define BX_HB               6

#define MAXATTR  17
#define CURTAIN_SLEEP 20
#define IMPLODE_SLEEP 25

namespace
{
    const unsigned char color_attr[] =
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

    const unsigned char monoc_attr[] =
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

    const unsigned char dbl_box[BX_SIZE] = { 0xc9, 0xbb, 0xc8, 0xbc, 0xba, 0xcd, 0xcd };
    const unsigned char sng_box[BX_SIZE] = { 0xda, 0xbf, 0xc0, 0xd9, 0xb3, 0xc4, 0xc4 };
    const unsigned char fat_box[BX_SIZE] = { 0xdb, 0xdb, 0xdb, 0xdb, 0xdb, 0xdf, 0xdc };
    const unsigned char spc_box[BX_SIZE] = { 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20 };
}

struct PdCursesOutput : public OutputInterface
{
    PdCursesOutput();
    ~PdCursesOutput();

public:
    virtual void clear();

    //Turn cursor on and off
    virtual bool cursor(bool ison);

    //get current cursor position
    virtual void getrc(int *r, int *c);

    virtual void clrtoeol();
    virtual void mvaddstr(int r, int c, const char *s);
    virtual void mvaddch(int r, int c, char chr);
    virtual int mvinch(int r, int c);
    virtual void addstr(const char *s);
    virtual void set_attr(int bute);

    //winit(win_name): initialize window -- open disk window -- determine type of monitor -- determine screen memory location for dma
    virtual void winit(bool narrow);

    virtual void forcebw();

    //wdump(windex): dump the screen off to disk, the window is saved so that it can be retrieved using windex
    virtual void wdump();

    //wrestor(windex): restore the window saved on disk
    virtual void wrestor();

    //Some general drawing routines
    virtual void box(int ul_r, int ul_c, int lr_r, int lr_c);

    //center a string according to how many columns there really are
    virtual void center(int row, const char *string);

    virtual void printw(const char *msg, ...);
    virtual void scroll_up(int start_row, int end_row, int nlines);
    virtual void scroll_dn(int start_row, int end_row, int nlines);
    virtual void scroll();

    //blot_out region (upper left row, upper left column) (lower right row, lower right column)
    virtual void blot_out(int ul_row, int ul_col, int lr_row, int lr_col);

    virtual void repchr(int chr, int cnt);

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
    virtual void add_text(short y, short x, unsigned char c);
    virtual int add_text(unsigned char c);

    virtual void add_tile(short y, short x, unsigned char c);
    virtual int add_tile(unsigned char c);

    virtual void stop_rendering();
    virtual void resume_rendering();

private:
    //drawing routines that do not auto-render
    void private_mvaddch(int r, int c, char chr, bool text);
    void private_addch(unsigned char c, bool text);
    void private_putchr(int c, bool text);
    void private_repchr(WINDOW* w, int chr, int cnt);
    void private_vbox(WINDOW* w, const unsigned char box[BX_SIZE], int ul_r, int ul_c, int lr_r, int lr_c);

    void Render();
    void ApplyCursor();

    //screen size
    short LINES = 25;
    short COLS = 80;

    //points to either color or monochrom attribute table
    const unsigned char *at_table;

    int m_attr = 0x7;
    bool m_cursor = false;
    bool m_curtain_down = false;
    bool m_should_render = true;

    WINDOW* m_backup_window;
};

std::shared_ptr<OutputInterface> CreateCursesOutput()
{
    return std::shared_ptr<OutputInterface>(new PdCursesOutput);
}

PdCursesOutput::PdCursesOutput()
{
}

PdCursesOutput::~PdCursesOutput()
{
    //todo: kill curses?
}

namespace
{
    //upper half of byte is bg, lower is fg
    //each group of 4 bits represent: is_light r g b
    void init_color_pair(int attr)
    {
        ::init_pair(attr, attr & 0x0f, (attr & 0xf0) >> 4);
    }
}

//winit(win_name): initialize window
void PdCursesOutput::winit(bool narrow_screen)
{
    ::initscr();
    ::keypad(::stdscr, TRUE);
    ::noecho();
    ::start_color();

    ::init_color(0x00, 0, 0, 0); //black
    ::init_color(0x01, 0, 0, 667); //blue
    ::init_color(0x02, 0, 667, 0); //green
    ::init_color(0x03, 0, 667, 667); //cyan
    ::init_color(0x04, 667, 0, 0); //red
    ::init_color(0x05, 667, 0, 667); //magenta
    ::init_color(0x06, 667, 333, 0); //brown
    ::init_color(0x07, 667, 667, 667); //grey
    ::init_color(0x08, 250, 250, 250); //d_grey
    ::init_color(0x09, 333, 333, 1000); //l_blue
    ::init_color(0x0a, 333, 1000, 333); //l_green
    ::init_color(0x0b, 100, 1000, 1000); //l_cyan
    ::init_color(0x0c, 1000, 333, 333); //l_red
    ::init_color(0x0d, 1000, 100, 1000); //l_magenta
    ::init_color(0x0e, 1000, 1000, 100); //yellow
    ::init_color(0x0f, 1000, 1000, 1000); //white

    init_color_pair(0x01);
    init_color_pair(0x02);
    init_color_pair(0x04);
    init_color_pair(0x05);
    init_color_pair(0x06);
    init_color_pair(0x07);
    init_color_pair(0x09);
    init_color_pair(0x0A);
    init_color_pair(0x0D);
    init_color_pair(0x0E);
    init_color_pair(0x70);
    init_color_pair(0x71);
    init_color_pair(0x72);
    init_color_pair(0x74);
    init_color_pair(0x78);
    init_color_pair(0x7E);
    init_color_pair(0xA0);

    LINES = 25;
    COLS = narrow_screen ? 40 : 80;
    at_table = color_attr;

    //todo: set cursor false?
}

//clear screen
void PdCursesOutput::clear()
{
    ::clear();
    Render();
}

//Turn cursor on and off
bool PdCursesOutput::cursor(bool enable)
{
    bool was_enabled = m_cursor;
    m_cursor = enable;
    if (enable != was_enabled)
        ApplyCursor();
    return was_enabled;
}

//get curent cursor position
void PdCursesOutput::getrc(int *r, int *c)
{
    *r = ::getcury(::stdscr);
    *c = ::getcurx(::stdscr);
}

//clrtoeol
void PdCursesOutput::clrtoeol()
{
    ::clrtoeol();
    Render();
}

void PdCursesOutput::mvaddch(int r, int c, char chr)
{
    private_mvaddch(r, c, chr, true);
    Render();
}

void PdCursesOutput::private_mvaddch(int r, int c, char chr, bool text)
{
    ::move(r, c);
    private_addch(chr, text);
}

//todo: get rid of entirely
int PdCursesOutput::mvinch(int r, int c)
{
    return (char)::mvinch(r, c);
}

int GetColor(int chr, int attr)
{
    //if it is inside a room
    if (attr == 0x07) switch (chr)
    {
    case DOOR: case VWALL: case HWALL: case ULWALL: case URWALL: case LLWALL: case LRWALL:
        return 0x06; //brown
    case FLOOR:
        return 0x0a; //light green
    case STAIRS:
        return 0x20; //black on green
    case TRAP:
        return 0x05; //magenta
    case GOLD: case PLAYER:
        return 0x0e; //yellow
    case POTION: case SCROLL: case STICK: case ARMOR: case AMULET: case RING: case WEAPON:
        return 0x09; //light blue
    case FOOD:
        return 0x04; //red
    }
    //if inside a passage or a maze
    else if (attr == 0x70) switch (chr)
    {
    case FOOD:
        return 0x74; //red on grey
    case GOLD: case PLAYER:
        return 0x7e; //yellow on grey
    case POTION: case SCROLL: case STICK: case ARMOR: case AMULET: case RING: case WEAPON:
        return 0x71; //blue on grey
    }
    // mdk: don't think used
    //else if (m_attr == 0x0f && chr == STAIRS)
    //    return 0xa0;

    return attr;
}

void PdCursesOutput::private_addch(unsigned char chr, bool text)
{
    int r, c;
    getrc(&r, &c);
    if (chr == '\n')
    {
        if (r == LINES - 1) {
            scroll_up(0, LINES - 1, 1);
            ::move(LINES - 1, 0);
        }
        else
            ::move(r + 1, 0);
        return;
    }

    unsigned char attr = m_attr;
    if (at_table == color_attr && !text)
    {
        attr = GetColor(chr, attr);
    }
    ::attron(COLOR_PAIR(attr));
    private_putchr(chr, text);
    ::attroff(COLOR_PAIR(attr));
    ::move(r, c + 1);
    return;
}

void PdCursesOutput::private_putchr(int c, bool text)
{
    if (text)
        ::addch(c);
    else
        ::addrawch(c);
}

void PdCursesOutput::mvaddstr(int r, int c, const char *s)
{
    ::move(r, c);
    addstr(s);
}

void PdCursesOutput::addstr(const char *s)
{
    for (int i = 0; s[i]; ++i)
    {
        private_addch(s[i], true);
    }
    Render();
}

void PdCursesOutput::set_attr(int bute)
{
    if (bute < MAXATTR) 
        m_attr = at_table[bute];
    else 
        m_attr = bute;
    ::attrset(COLOR_PAIR(m_attr));
}

void PdCursesOutput::forcebw()
{
    at_table = monoc_attr;
}

//wdump(windex): dump the screen off to disk, the window is saved so that it can be retrieved using windex
void PdCursesOutput::wdump()
{
    m_backup_window = ::newwin(0, 0, 0, 0);
    ::overwrite(stdscr, m_backup_window);
}

//wrestor(windex): restore the window saved on disk
void PdCursesOutput::wrestor()
{
    ::overwrite(m_backup_window, stdscr);
    ::delwin(m_backup_window);
    m_backup_window = 0;
    Render();
}

//Some general drawing routines
void PdCursesOutput::box(int ul_r, int ul_c, int lr_r, int lr_c)
{
    bool wason;
    int r, c;

    wason = cursor(false);
    getrc(&r, &c);
    private_vbox(stdscr, dbl_box, ul_r, ul_c, lr_r, lr_c);
    ::move(r, c);
    cursor(wason);

    Render();

}

//box: draw a box given the upper left coordinate and the lower right
void PdCursesOutput::private_vbox(WINDOW* w, const unsigned char box[BX_SIZE], int ul_r, int ul_c, int lr_r, int lr_c)
{
    int i;
    //draw horizontal boundary
    ::wmove(w, ul_r, ul_c + 1);
    private_repchr(w, box[BX_HT], i = (lr_c - ul_c - 1));
    ::wmove(w, lr_r, ul_c + 1);
    private_repchr(w, box[BX_HB], i);
    //draw vertical boundary
    for (i = ul_r + 1; i < lr_r; i++) {
        ::mvwaddch(w, i, ul_c, box[BX_VW]);
        ::mvwaddch(w, i, lr_c, box[BX_VW]);
    }
    //draw corners
    ::mvwaddch(w, ul_r, ul_c, box[BX_UL]);
    ::mvwaddch(w, ul_r, lr_c, box[BX_UR]);
    ::mvwaddch(w, lr_r, ul_c, box[BX_LL]);
    ::mvwaddch(w, lr_r, lr_c, box[BX_LR]);
}

//center a string according to how many columns there really are
void PdCursesOutput::center(int row, const char *string)
{
    int n = (int)strlen(string);
    mvaddstr(row, (COLS - n) / 2, string);
}

void PdCursesOutput::printw(const char *format, ...)
{
    char dest[1024 * 16];
    va_list argptr;
    va_start(argptr, format);
    vsprintf_s(dest, format, argptr);
    va_end(argptr);

    addstr(dest);
}

void PdCursesOutput::scroll_up(int start_row, int end_row, int nlines)
{
    ::move(end_row, ::getcurx(::stdscr));
}

void PdCursesOutput::scroll_dn(int start_row, int end_row, int nlines)
{
    ::move(start_row, ::getcurx(::stdscr));
}

void PdCursesOutput::scroll()
{
    scroll_up(0, 24, 1);
}

//blot_out region (upper left row, upper left column) (lower right row, lower right column)
void PdCursesOutput::blot_out(int ul_row, int ul_col, int lr_row, int lr_col)
{
    int r, c;
    for (r = ul_row; r <= lr_row; ++r)
    {
        for (c = ul_col; c <= lr_col; ++c)
        {
            ::move(r, c);
            private_putchr(' ', true);
        }
    }
    ::move(ul_row, ul_col);
    Render();
}

void PdCursesOutput::repchr(int chr, int cnt)
{
    private_repchr(stdscr, chr, cnt);
    Render();
}

void PdCursesOutput::private_repchr(WINDOW* w, int chr, int cnt)
{
    while (cnt-- > 0) {
        ::waddch(w, chr);
    }
}

//Clear the screen in an interesting fashion
void PdCursesOutput::implode()
{
    int j, r, c, cinc = COLS / 10 / 2, er, ec;

    er = (COLS == 80 ? LINES - 3 : LINES - 4);

    for (r = 0, c = 0, ec = COLS - 1; r < 10; r++, c += cinc, er--, ec -= cinc)
    {
        private_vbox(stdscr, sng_box, r, c, er, ec);
        Render();
        sleep(IMPLODE_SLEEP);
        for (j = r + 1; j <= er - 1; j++)
        {
            move(j, c + 1); 
            private_repchr(stdscr, ' ', cinc - 1);
            move(j, ec - cinc + 1);
            private_repchr(stdscr, ' ', cinc - 1);
        }
        private_vbox(stdscr, spc_box, r, c, er, ec);
    }
    Render();
}

//drop_curtain: Close a door on the screen and redirect output to the temporary buffer
void PdCursesOutput::drop_curtain()
{
    if (m_curtain_down)
        return;

    wdump();
    wattron(m_backup_window, COLOR_PAIR(0x02));
    private_vbox(m_backup_window, sng_box, 0, 0, LINES - 1, COLS - 1);
    wattron(m_backup_window, COLOR_PAIR(0x0e));
    for (int r = 1; r < LINES - 1; r++)
    {
        wmove(m_backup_window, r, 1);
        int cnt = COLS - 2;
        while (cnt-- > 0) {
            ::waddch(m_backup_window, 0xb1);
        }
        wrefresh(m_backup_window);
        sleep(CURTAIN_SLEEP);
    }
    m_curtain_down = true;
}

void PdCursesOutput::raise_curtain()
{
    if (!m_curtain_down)
        return;

    for (int r = LINES - 2; r > 0; r--)
    {
        copywin(stdscr, m_backup_window, r, 1, r, 1, r, COLS - 2, FALSE);
        wrefresh(m_backup_window);
        sleep(CURTAIN_SLEEP);
    }
    m_curtain_down = false;
}

void PdCursesOutput::move(short y, short x)
{
    ::move(y, x);
    if (m_cursor)
        Render();
}

char PdCursesOutput::curch()
{
    return (char)::inch();
}

int PdCursesOutput::add_text(unsigned char c)
{
    private_addch(c, true);
    Render();
    return OK;
}

void PdCursesOutput::add_text(short y, short x, unsigned char c)
{
    private_mvaddch(y, x, c, true);
    Render();
}

void PdCursesOutput::add_tile(short y, short x, unsigned char c)
{
    private_mvaddch(y, x, c, false);
    Render();
}

int PdCursesOutput::add_tile(unsigned char c)
{
    private_addch(c, false);
    Render();
    return OK;
}

int PdCursesOutput::lines() const
{
    return LINES;
}

int PdCursesOutput::columns() const
{
    return COLS;
}

void PdCursesOutput::stop_rendering()
{
    m_should_render = false;
}

void PdCursesOutput::resume_rendering()
{
    m_should_render = true;
    Render();
    ApplyCursor();
}

void PdCursesOutput::Render()
{
    if (!m_should_render || m_curtain_down)
        return;
    ::refresh();
}

void PdCursesOutput::ApplyCursor()
{
    if (!m_should_render || m_curtain_down)
        return;
    ::curs_set(m_cursor);
}

