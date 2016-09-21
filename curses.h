#define BX_SIZE 7

#define standend()          set_attr(0)
#define green()             set_attr(1)
#define cyan()              set_attr(2)
#define red()               set_attr(3)
#define magenta()           set_attr(4)
#define brown()             set_attr(5)
#define dgrey()             set_attr(6)
#define lblue()             set_attr(7)
#define lgrey()             set_attr(8)
#define lred()              set_attr(9)
#define lmagenta()          set_attr(10)
#define yellow()            set_attr(11)
#define uline()             set_attr(12)
#define blue()              set_attr(13)
#define standout()          set_attr(14)
#define high()              set_attr(15)
#define bold()              set_attr(16)

namespace Screen
{
    void DrawChar(Coord p, char c);
}

void clear();

void putchr(int c, int attr);

//Turn cursor on and off
bool cursor(bool ison);

//get current cursor position
void getrc(int *r, int *c);

void clrtoeol();

void mvaddstr(int r, int c, const char *s);

void mvaddch(int r, int c, char chr);

int mvinch(int r, int c);

int addch(byte chr);

void addstr(const char *s);

void set_attr(int bute);

void error(int mline, char *msg, int a1, int a2, int a3, int a4, int a5);

//winit(win_name): initialize window -- open disk window -- determine type of monitor -- determine screen memory location for dma
void winit();

void forcebw();

//wdump(windex): dump the screen off to disk, the window is saved so that it can be retrieved using windex
void wdump();

//wrestor(windex): restore the window saved on disk
void wrestor();

//Some general drawing routines
void box(int ul_r, int ul_c, int lr_r, int lr_c);

//box: draw a box given the upper left coordinate and the lower right
void vbox(byte box[BX_SIZE], int ul_r, int ul_c, int lr_r, int lr_c);

//center a string according to how many columns there really are
void center(int row, const char *string);

//printw(Ieeeee)
void printw(const char *msg, ...);

void scroll_up(int start_row, int end_row, int nlines);

void scroll_dn(int start_row, int end_row, int nlines);

void scroll();

//blot_out region (upper left row, upper left column) (lower right row, lower right column)
void blot_out(int ul_row, int ul_col, int lr_row, int lr_col);

void repchr(int chr, int cnt);

//try to fixup screen after we get a control break
void fixup();

//Clear the screen in an interesting fashion
void implode();

//drop_curtain: Close a door on the screen and redirect output to the temporary buffer
void drop_curtain();

void raise_curtain();

void move(short y, short x);

char curch();
