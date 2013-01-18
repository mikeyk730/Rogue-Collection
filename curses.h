void clear();

//Turn cursor on and off
int cursor(bool ison);

//get curent cursor position
void getrc(int *rp, int *cp);

void real_rc(int pn, int *rp, int *cp);

void clrtoeol();

void mvaddstr(int r, int c, char *s);

void mvaddch(int r, int c, char chr);

int mvinch(int r, int c);

int addch(byte chr);

void addstr(char *s);

void set_attr(int bute);

void error(int mline, char *msg, int a1, int a2, int a3, int a4, int a5);

//Called when rogue runs to move our cursor to be where DOS thinks the cursor is
void set_cursor();

//winit(win_name): initialize window -- open disk window -- determine type of monitor -- determine screen memory location for dma
void winit();

void forcebw();

//wdump(windex): dump the screen off to disk, the window is saved so that it can be retrieved using windex
void wdump();

void sav_win();

void res_win();

//wrestor(windex): restore the window saved on disk
void wrestor();

//wclose(): close the window file
void wclose();

//Some general drawing routines
void box(int ul_r, int ul_c, int lr_r, int lr_c);

//box: draw a box given the upper left coordinate and the lower right
void vbox(byte box[BX_SIZE], int ul_r, int ul_c, int lr_r, int lr_c);

//center a string according to how many columns there really are
void center(int row, char *string);

//printw(Ieeeee)
void printw(char *msg, ...);

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

void switch_page(int pn);

int get_mode(int type);

int video_mode(int type);