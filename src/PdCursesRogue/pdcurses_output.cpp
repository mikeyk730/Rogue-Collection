#include <RogueCore/output_interface.h>
#include <RogueCore/rogue.h>
#include <RogueCore/mach_dep.h>
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
    void MoveAddCharacter(int r, int c, char chr);
    void AddCharacter(unsigned char c);
    void PutCharacter(int c, int attr);

    void vbox(const unsigned char box[BX_SIZE], int ul_r, int ul_c, int lr_r, int lr_c);

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

    std::shared_ptr<DisplayInterface> m_screen;
    bool disable_render = false;

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

//clear screen
void PdCursesOutput::clear()
{
    ::clear();
}

void PdCursesOutput::PutCharacter(int c, int attr)
{
    ::attron(COLOR_PAIR(attr));
    ::addrawch(c);
    ::attron(COLOR_PAIR(attr));
    if (!disable_render)
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
}

void PdCursesOutput::mvaddstr(int r, int c, const char *s)
{
    move(r, c);
    addstr(s);
}

void PdCursesOutput::MoveAddCharacter(int r, int c, char chr)
{
    move(r, c);
    AddCharacter(chr);
}

void PdCursesOutput::mvaddch(int r, int c, char chr)
{
    MoveAddCharacter(r, c, chr);
}

//todo: get rid of entirely
int PdCursesOutput::mvinch(int r, int c)
{
    move(r, c);
    return curch();
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
        return 0xa0; //black on light green
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

void PdCursesOutput::AddCharacter(unsigned char chr)
{
    int r, c;
    getrc(&r, &c);
    if (chr == '\n')
    {
        if (r == LINES - 1) {
            scroll_up(0, LINES - 1, 1);
            move(LINES - 1, 0);
        }
        else
            move(r + 1, 0);
        return;
    }

    unsigned char attr = m_attr;
    if (at_table == color_attr)
    {
        attr = GetColor(chr, attr);
    }
    PutCharacter(chr, attr);
    move(r, c + 1);
    return;
}

void PdCursesOutput::addstr(const char *s)
{
    bool was_disabled = disable_render;
    disable_render = true;

    int i;
    for (i = 0; s[i]; ++i)
    {
        AddCharacter(s[i]);
    }
    disable_render = was_disabled;
    if (!disable_render) {
        Render();
    }
}

void PdCursesOutput::set_attr(int bute)
{
    if (bute < MAXATTR) m_attr = at_table[bute];
    else m_attr = bute;
}

void PdCursesOutput::error(int mline, char *msg, int a1, int a2, int a3, int a4, int a5)
{
    int row, col;

    getrc(&row, &col);
    move(mline, 0);
    clrtoeol();
    printw(msg, a1, a2, a3, a4, a5);
    move(row, col);
}

namespace
{
    //upper half of byte is bg, lower is fg
    //each group of 4 bits represent: is_light r g b
    void init_color_pair(int attr)
    {
        ::init_pair(attr, attr & 0x0f, (attr & 0xf0)>>4);
    }
}

//winit(win_name): initialize window
void PdCursesOutput::winit(bool narrow_screen)
{
    ::initscr();
    ::keypad(::stdscr, TRUE);
    ::noecho();
    ::start_color();

    ::init_color(0x00,    0,    0,    0); //black
    ::init_color(0x01,    0,    0,  667); //blue
    ::init_color(0x02,    0,  667,    0); //green
    ::init_color(0x03,    0,  667,  667); //cyan
    ::init_color(0x04,  667,    0,    0); //red
    ::init_color(0x05,  667,    0,  667); //magenta
    ::init_color(0x06,  667,  333,    0); //brown
    ::init_color(0x07,  667,  667,  667); //grey
    ::init_color(0x08,  250,  250,  250); //d_grey
    ::init_color(0x09,  333,  333, 1000); //l_blue
    ::init_color(0x0a,  333, 1000,  333); //l_green
    ::init_color(0x0b,  100, 1000, 1000); //l_cyan
    ::init_color(0x0c, 1000,  333,  333); //l_red
    ::init_color(0x0d, 1000,  100, 1000); //l_magenta
    ::init_color(0x0e, 1000, 1000,  100); //yellow
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
    vbox(dbl_box, ul_r, ul_c, lr_r, lr_c);
}

//box: draw a box given the upper left coordinate and the lower right
void PdCursesOutput::vbox(const unsigned char box[BX_SIZE], int ul_r, int ul_c, int lr_r, int lr_c)
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
        MoveAddCharacter(i, ul_c, box[BX_VW]);
        MoveAddCharacter(i, lr_c, box[BX_VW]);
    }
    //draw corners
    MoveAddCharacter(ul_r, ul_c, box[BX_UL]);
    MoveAddCharacter(ul_r, lr_c, box[BX_UR]);
    MoveAddCharacter(lr_r, ul_c, box[BX_LL]);
    MoveAddCharacter(lr_r, lr_c, box[BX_LR]);
    move(r, c);
    cursor(wason);

    disable_render = was_disabled;
    if (!disable_render)
        Render();
}

//center a string according to how many columns there really are
void PdCursesOutput::center(int row, const char *string)
{
    mvaddstr(row, (COLS - strlen(string)) / 2, string);
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
    move(end_row, ::getcurx(::stdscr));
}

void PdCursesOutput::scroll_dn(int start_row, int end_row, int nlines)
{
    move(start_row, ::getcurx(::stdscr));
}

void PdCursesOutput::scroll()
{
    scroll_up(0, 24, 1);
}

//blot_out region (upper left row, upper left column) (lower right row, lower right column)
void PdCursesOutput::blot_out(int ul_row, int ul_col, int lr_row, int lr_col)
{
    bool was_disabled = disable_render;
    disable_render = true;

    int r, c;
    for (r = ul_row; r <= lr_row; ++r)
    {
        for (c = ul_col; c <= lr_col; ++c)
        {
            move(r, c);
            PutCharacter(' ', m_attr);
        }
    }
    move(ul_row, ul_col);
    disable_render = was_disabled;
    if (!disable_render)
        Render();
}

void PdCursesOutput::repchr(int chr, int cnt)
{
    bool was_disabled = disable_render;
    disable_render = true;

    while (cnt-- > 0) {
        PutCharacter(chr, m_attr);
    }
    disable_render = was_disabled;
    if (!disable_render)
        Render();
}

//Clear the screen in an interesting fashion
void PdCursesOutput::implode()
{

}

//drop_curtain: Close a door on the screen and redirect output to the temporary buffer
void PdCursesOutput::drop_curtain()
{

}

void PdCursesOutput::raise_curtain()
{

}

void PdCursesOutput::move(short y, short x)
{
    ::move(y, x);
}

char PdCursesOutput::curch()
{
    return (char)::inch();
    //return m_data.buffer[m_row*COLS + m_col].Char.AsciiChar;
}

void PdCursesOutput::add_text(short y, short x, unsigned char c)
{
    MoveAddCharacter(y, x, c);
}

int PdCursesOutput::add_text(unsigned char c)
{
    AddCharacter(c);
    return OK;
}

void PdCursesOutput::add_tile(short y, short x, unsigned char c)
{
    MoveAddCharacter(y, x, c);
}

int PdCursesOutput::add_tile(unsigned char c)
{
    AddCharacter(c);
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

