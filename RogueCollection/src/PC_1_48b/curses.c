//Cursor motion stuff to simulate a "no refresh" version of curses

#include "rogue.h"

//Globals for curses

int LINES = 25, COLS = 80;
int is_saved = FALSE;
int iscuron = TRUE;
int ch_attr = 0x7;
int old_page_no;
int no_check = FALSE;
int scr_ds = 0xB800;
int svwin_ds;
char *savewin;
int scr_type = -1;
int tab_size = 8;
int page_no = 0;

static int old_ds;

#define MAXATTR  17

byte color_attr[] =
{
    7, // 0 normal
    2, // 1 green
    3, // 2 cyan
    4, // 3 red
    5, // 4 magenta
    6, // 5 brown
    8, // 6 dark grey
    9, // 7 light blue
   10, // 8 light green
   12, // 9 light red
   13, //10 light magenta
   14, //11 yellow
   15, //12 uline
    1, //13 blue
  112, //14 reverse
   15, //15 high intensity
  112, //16 bold
    0  //no more
};

byte monoc_attr[] =
{
    7, // 0 normal
    7, // 1 green
    7, // 2 cyan
    7, // 3 red
    7, // 4 magenta
    7, // 5 brown
    7, // 6 dark grey
    7, // 7 light blue
    7, // 8 light green
    7, // 9 light red
    7, //10 light magenta
    7, //11 yellow
   17, //12 uline
    7, //13 blue
  120, //14 reverse
    7, //15 white/hight
  120, //16 bold
    0  //no more
} ;

byte *at_table;

int c_row, c_col; //Save cursor positions so we don't ask dos
int scr_row[25];

byte dbl_box[BX_SIZE] = {0xc9, 0xbb, 0xc8, 0xbc, 0xba, 0xcd, 0xcd};
byte sng_box[BX_SIZE] = {0xda, 0xbf, 0xc0, 0xd9, 0xb3, 0xc4, 0xc4};
byte fat_box[BX_SIZE] = {0xdb, 0xdb, 0xdb, 0xdb, 0xdb, 0xdf, 0xdc};
byte spc_box[BX_SIZE] = {0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20};

//clear screen
clear()
{
  if (scr_ds==svwin_ds) wsetmem(savewin, LINES*COLS, 0x0720);
  else blot_out(0, 0, LINES-1, COLS-1);
}

//Turn cursor on and off
cursor(bool ison)
{
  int oldstate;
  int w_state;

  if (iscuron==ison) return ison;
  oldstate = iscuron;
  iscuron = ison;
  regs->ax = 0x100;
  if (ison)
  {
    regs->cx = (is_color?0x607:0xb0c);
    swint(SW_SCR, regs);
    move(c_row, c_col);
  }
  else
  {
    regs->cx = 0xf00;
    swint(SW_SCR, regs);
  }
  return (oldstate);
}

//get curent cursor position
getrc(int *rp, int *cp)
{
  *rp = c_row;
  *cp = c_col;
}

real_rc(int pn, int *rp, int *cp)
{
  //pc bios: read current cursor position
  regs->ax = 0x300;
  regs->bx = pn<<8;
  swint(SW_SCR, regs);
  *rp = regs->dx>>8;
  *cp = regs->dx&0xff;
}

//clrtoeol
clrtoeol()
{
  int r, c;

  if (scr_ds==svwin_ds) return;
  getrc(&r, &c);
  blot_out(r, c, r, COLS-1);
}

mvaddstr(int r, int c, char *s)
{
  move(r, c);
  addstr(s);
}

mvaddch(int r, int c, char chr)
{
  move(r, c);
  addch(chr);
}

mvinch(int r, int c)
{
  move(r, c);
  return (curch()&0xff);
}

//put the character on the screen and update the character position
addch(byte chr)
{
  int r, c;
  int newc, newr;
  byte old_attr;

  old_attr = ch_attr;
  if (at_table==color_attr)
  {
    //if it is inside a room
    if (ch_attr==7) switch(chr)
    {
      case DOOR: case VWALL: case HWALL: case ULWALL: case URWALL: case LLWALL: case LRWALL:
        ch_attr = 6; //brown
      break;
      case FLOOR:
        ch_attr = 10; //light green
      break;
      case STAIRS:
        ch_attr = 160; //black on green
      break;
      case TRAP:
        ch_attr = 5; //magenta
      break;
      case GOLD: case PLAYER:
        ch_attr = 14; //yellow
      break;
      case POTION: case SCROLL: case STICK: case ARMOR: case AMULET: case RING: case WEAPON:
        ch_attr = 9;
      break;
      case FOOD:
        ch_attr = 4;
      break;
    }
    //if inside a passage or a maze
    else if (ch_attr==112) switch(chr)
    {
      case FOOD:
        ch_attr = 116; //red
      break;
      case GOLD: case PLAYER:
        ch_attr = 126; //yellow on white
      break;
      case POTION: case SCROLL: case STICK: case ARMOR: case AMULET: case RING: case WEAPON:
        ch_attr = 113; //blue on white
      break;
    }
    else if (ch_attr==15 && chr==STAIRS) ch_attr = 160;
  }
  getrc(&r, &c);
  if (chr=='\n')
  {
    if (r==LINES-1) {scroll_up(0, LINES-1, 1); move(LINES-1, 0);}
    else move(r+1, 0);
    ch_attr = old_attr;
    return c_row;
  }
  putchr(chr);
  move(r, c+1);
  ch_attr = old_attr;
  //if you have gone off the screen scroll the whole window
  return (c_row);
}

addstr(char *s)
{
  while (*s) addch(*s++);
}

set_attr(int bute)
{
  if (bute<MAXATTR) ch_attr = at_table[bute];
  else ch_attr = bute;
}

error(int mline, char *msg, int a1, int a2, int a3, int a4, int a5)
{
  int row, col;

  getrc(&row, &col);
  move(mline, 0);
  clrtoeol();
  printw(msg, a1, a2, a3, a4, a5);
  move(row, col);
}

//Called when rogue runs to move our cursor to be where DOS thinks the cursor is
set_cursor()
{
  //regs->ax = 15 << 8;
  //swint(SW_SCR, regs);
  //real_rc(regs->bx >> 8, &c_row, &c_col);
}

//winit(win_name): initialize window -- open disk window -- determine type of monitor -- determine screen memory location for dma
winit(char drive)
{
  int i, cnt;
  extern int _dsval;

  //Get monitor type
  regs->ax = 15<<8;
  swint(SW_SCR, regs);
  old_page_no = regs->bx>>8;
  scr_type = regs->ax = 0xff&regs->ax;
  //initialization is any good because restarting game has old values!!! So reassign defaults
  LINES = 25;
  COLS = 80;
  scr_ds = 0xB800;
  at_table = monoc_attr;
  switch (scr_type)
  {
    //It is a TV
    case 1: at_table = color_attr;
    case 0: COLS = 40; break;
    //It's a high resolution monitor
    case 3: at_table = color_attr;
    case 2: break;
    case 7: scr_ds = 0xB000; no_check = TRUE; break;
    default: move(24, 0); fatal("Unknown screen type (%d)", regs->ax);
  }
  //Read current cursor position
  real_rc(old_page_no, &c_row, &c_col);
  if ((savewin = sbrk(4096))==-1)
  {
    svwin_ds = -1;
    savewin = (char *)_flags;
    if (scr_type==7) fatal(no_mem);
  }
  else
  {
    savewin = (char *)(((int)savewin+0xf)&0xfff0);
    svwin_ds = (((int)savewin>>4)&0xfff)+_dsval;
  }
  for (i = 0, cnt = 0; i<25; cnt += 2*COLS, i++) scr_row[i] = cnt;
  newmem(2);
  switch_page(3);
  if (old_page_no!=page_no) clear();
  move(c_row, c_col);
  if (isjr()) no_check = TRUE;
}

forcebw()
{
  at_table = monoc_attr;
}

//wdump(windex): dump the screen off to disk, the window is saved so that it can be retrieved using windex
wdump()
{
  sav_win();
  dmain(savewin, LINES*COLS, scr_ds, 0);
  is_saved = TRUE;
}

sav_win()
{
  if (savewin==_flags) dmaout(savewin, LINES*COLS, 0xb800, 8192);
  return (savewin);
}

res_win()
{
  if (savewin==_flags) dmain(savewin, LINES*COLS, 0xb800, 8192);
}

//wrestor(windex): restore the window saved on disk
wrestor()
{
  dmaout(savewin, LINES*COLS, scr_ds, 0);
  res_win();
  is_saved = FALSE;
}

//wclose(): close the window file
wclose()
{
  //Restore cursor (really you want to restore video state, but be careful)
  if (scr_type>=0) cursor(TRUE);
  if (page_no!=old_page_no) switch_page(old_page_no);
}

//Some general drawing routines
box(ul_r, ul_c, lr_r, lr_c)
{
  vbox(dbl_box, ul_r, ul_c, lr_r, lr_c);
}

//box: draw a box given the upper left coordinate and the lower right
vbox(byte box[BX_SIZE], int ul_r, int ul_c, int lr_r, int lr_c)
{
  int i, wason;
  int r, c;

  wason = cursor(FALSE);
  getrc(&r, &c);
  //draw horizontal boundary
  move(ul_r, ul_c+1);
  repchr(box[BX_HT], i = (lr_c-ul_c-1));
  move(lr_r, ul_c+1);
  repchr(box[BX_HB], i);
  //draw vertical boundary
  for (i = ul_r+1; i<lr_r; i++) {mvaddch(i, ul_c, box[BX_VW]); mvaddch(i, lr_c, box[BX_VW]);}
  //draw corners
  mvaddch(ul_r, ul_c, box[BX_UL]);
  mvaddch(ul_r, lr_c, box[BX_UR]);
  mvaddch(lr_r, ul_c, box[BX_LL]);
  mvaddch(lr_r, lr_c, box[BX_LR]);
  move(r, c);
  cursor(wason);
}

//center a string according to how many columns there really are
center(int row, char *string)
{
  mvaddstr(row, (COLS-strlen(string))/2, string);
}

//printw(Ieeeee)
printw(char *msg, int a1, int a2, int a3, int a4, int a5, int a6, int a7, int a8)
{
  char pwbuf[132];

  sprintf(pwbuf, msg, a1, a2, a3, a4, a5, a6, a7, a8);
  addstr(pwbuf);
}

scroll_up(int start_row, int end_row, int nlines)
{
  regs->ax = 0x600+nlines;
  regs->bx = 0x700;
  regs->cx = start_row<<8;
  regs->dx = (end_row<<8)+COLS-1;
  swint(SW_SCR, regs);
  move(end_row, c_col);
}

scroll_dn(int start_row, int end_row, int nlines)
{
  regs->ax = 0x700+nlines;
  regs->bx = 0x700;
  regs->cx = start_row<<8;
  regs->dx = (end_row<<8)+COLS-1;
  swint(SW_SCR, regs);
  move(start_row, c_col);
}

scroll()
{
  scroll_up(0, 24, 1);
}

//blot_out region (upper left row, upper left column) (lower right row, lower right column)
blot_out(int ul_row, int ul_col, int lr_row, int lr_col)
{
  regs->ax = 0x600;
  regs->bx = 0x700;
  regs->cx = (ul_row<<8)+ul_col;
  regs->dx = (lr_row<<8)+lr_col;
  swint(SW_SCR, regs);
  move(ul_row, ul_col);
}

repchr(int chr, int cnt)
{
  while(cnt-->0) {putchr(chr); c_col++;}
}

//try to fixup screen after we get a control break
fixup()
{
  blot_out(c_row, c_col, c_row, c_col+1);
}

//Clear the screen in an interesting fashion
implode()
{
  int j, delay, r, c, cinc = COLS/10/2, er, ec;

  er = (COLS==80?LINES-3:LINES-4);
  //If the curtain is down, just clear the memory
  if (scr_ds==svwin_ds) {wsetmem(savewin, (er+1)*COLS, 0x0720); return;}
  delay = scr_type==7?500:10;
  for (r = 0, c = 0, ec = COLS-1; r<10; r++, c += cinc, er--, ec -= cinc)
  {
    vbox(sng_box, r, c, er, ec);
    for (j = delay; j--;) ;
    for (j = r+1; j<=er-1; j++)
    {
      move(j, c+1); repchr(' ', cinc-1);
      move(j, ec-cinc+1); repchr(' ', cinc-1);
    }
    vbox(spc_box, r, c, er, ec);
  }
}

//drop_curtain: Close a door on the screen and redirect output to the temporary buffer
drop_curtain()
{
  int r, c, j, delay;

  if (svwin_ds==-1) return;
  old_ds = scr_ds;
  dmain(savewin, LINES*COLS, scr_ds, 0);
  cursor(FALSE);
  delay = (scr_type==7?3000:2000);
  green();
  vbox(sng_box, 0, 0, LINES-1, COLS-1);
  yellow();
  for (r = 1; r<LINES-1; r++)
  {
    move(r, 1);
    repchr(0xb1, COLS-2);
    for (j = delay; j--;) ;
  }
  scr_ds = svwin_ds;
  move(0, 0);
  standend();
}

raise_curtain()
{
  int i, j, o, delay;

  if (svwin_ds==-1) return;
  scr_ds = old_ds;
  delay = (scr_type==7?3000:2000);
  for (i = 0, o = (LINES-1)*COLS*2; i<LINES; i++, o -= COLS*2)
  {
    dmaout(savewin+o, COLS, scr_ds, o);
    for (j = delay; j--;) ;
  }
}

switch_page(pn)
{
  int pgsize;

  if (scr_type==7) {page_no = 0; return;}
  if (COLS==40) pgsize = 2048; else pgsize = 4096;
  regs->ax = 0x0500|pn;
  swint(SW_SCR, regs);
  scr_ds = 0xb800+((pgsize*pn)>>4);
  page_no = pn;
}

get_mode(type)
{
  struct sw_regs regs;

  regs.ax = 0xF00;
  swint(SW_SCR, &regs);
  return 0xff&regs.ax;
}

video_mode(type)
{
  struct sw_regs regs;

  regs.ax = type;
  swint(SW_SCR, &regs);
  return regs.ax;
}
