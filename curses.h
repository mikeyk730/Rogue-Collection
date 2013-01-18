clear();

//Turn cursor on and off
cursor(bool ison);

//get curent cursor position
getrc(int *rp, int *cp);

real_rc(int pn, int *rp, int *cp);

clrtoeol();

mvaddstr(int r, int c, char *s);

mvaddch(int r, int c, char chr);

mvinch(int r, int c);

addch(byte chr);

addstr(char *s);

set_attr(int bute);

error(int mline, char *msg, int a1, int a2, int a3, int a4, int a5);

//Called when rogue runs to move our cursor to be where DOS thinks the cursor is
set_cursor();

//winit(win_name): initialize window -- open disk window -- determine type of monitor -- determine screen memory location for dma
winit();

forcebw();

//wdump(windex): dump the screen off to disk, the window is saved so that it can be retrieved using windex
wdump();

sav_win();

res_win();

//wrestor(windex): restore the window saved on disk
wrestor();

//wclose(): close the window file
wclose();

//Some general drawing routines
box(ul_r, ul_c, lr_r, lr_c);

//box: draw a box given the upper left coordinate and the lower right
vbox(byte box[BX_SIZE], int ul_r, int ul_c, int lr_r, int lr_c);

//center a string according to how many columns there really are
center(int row, char *string);

//printw(Ieeeee)
printw(char *msg, ...);

scroll_up(int start_row, int end_row, int nlines);

scroll_dn(int start_row, int end_row, int nlines);

scroll();

//blot_out region (upper left row, upper left column) (lower right row, lower right column)
blot_out(int ul_row, int ul_col, int lr_row, int lr_col);

repchr(int chr, int cnt);

//try to fixup screen after we get a control break
fixup();

//Clear the screen in an interesting fashion
implode();

//drop_curtain: Close a door on the screen and redirect output to the temporary buffer
drop_curtain();

raise_curtain();

void switch_page(pn);

get_mode(type);

video_mode(type);