//Various input/output functions
//io.c         1.4             (A.I. Design) 12/10/84

#include <stdio.h>
#include <ctype.h>

#include "rogue.h"
#include "io.h"
#include "curses.h"
#include "misc.h"
#include "mach_dep.h"
#include "strings.h"
#include "command.h"
#include "rings.h"
#include "hero.h"
#include "level.h"
#include "pack.h"

#define AC(a)    (-((a)-11))
#define PT(i,j)  ((COLS==40)?i:j)

extern unsigned tick;
char msgbuf[BUFSIZE];

static int newpos = 0;
static char *formats = "scud%", *bp, left_justify;
static int min_width, max_width;
static char ibuf[6];

bool save_msg = TRUE;       //Remember last msg
bool terse = FALSE;
bool expert = FALSE;

int short_msgs()
{
  return in_small_screen_mode() || in_brief_mode();
}

void set_small_screen_mode(int enable)
{
  terse = enable;
}

int in_small_screen_mode()
{
  return terse;
}

void set_brief_mode(int enable)
{
  expert = enable;
}

int in_brief_mode()
{
  return expert;
}

//msg: Display a message at the top of the screen.
void ifterse(const char *tfmt, const char *fmt, int a1, int a2, int a3, int a4, int a5)
{
  if (expert) msg(tfmt, a1, a2, a3, a4, a5);
  else msg(fmt, a1, a2, a3, a4, a5);
}

void unsaved_msg(const char *fmt, int a1, int a2, int a3, int a4, int a5)
{
    save_msg = FALSE;
    msg(fmt, a1, a2, a3, a4, a5);
    save_msg = TRUE;
}

void msg(const char *fmt, int a1, int a2, int a3, int a4, int a5)
{
  //if the string is "", just clear the line
  if (*fmt=='\0') {move(0, 0); clrtoeol(); mpos = 0; return;}
  //otherwise add to the message and flush it out
  doadd(fmt, a1,a2,a3,a4,a5);
  endmsg();
}

//addmsg: Add things to the current message
void addmsg(const char *fmt, int a1, int a2, int a3, int a4, int a5)
{
  doadd(fmt, a1, a2, a3, a4, a5);
}

//endmsg: Display a new msg (giving him a chance to see the previous one if it is up there with the -More-)
void endmsg()
{
  if (save_msg) strcpy(huh, msgbuf);
  if (mpos) {look(FALSE); move(0, mpos); more(" More ");}
  //All messages should start with uppercase, except ones that start with a pack addressing character
  if (islower(msgbuf[0]) && msgbuf[1]!=')') msgbuf[0] = toupper(msgbuf[0]);
  putmsg(0, msgbuf);
  mpos = newpos;
  newpos = 0;
}

//More: tag the end of a line and wait for a space
void more(const char *msg)
{
  int x, y;
  int i, msz;
  char mbuf[80];
  int morethere = TRUE;
  int covered = FALSE;

  msz = strlen(msg);
  getrc(&x,&y);
  //it is reasonable to assume that if the you are no longer on line 0, you must have wrapped.
  if (x!=0) {x = 0; y = COLS;}
  if ((y+msz)>COLS) {move(x, y = COLS-msz); covered = TRUE;}
  for (i=0; i<msz; i++)
  {
    mbuf[i] = curch();
    if ((i+y)<(COLS-2)) move(x, y+i+1);
    mbuf[i+1] = 0;
  }
  move(x, y);
  standout();
  addstr(msg);
  standend();
  while (readchar()!=' ')
  {
    if (covered && morethere) {move(x, y); addstr(mbuf); morethere = FALSE;}
    else if (covered) {move(x, y); standout(); addstr(msg); standend(); morethere = TRUE;}
  }
  move(x, y);
  addstr(mbuf);
}

//doadd: Perform an add onto the message buffer
void doadd(const char *fmt, int a1, int a2, int a3, int a4, int a5)
{
  sprintf(&msgbuf[newpos], fmt, a1, a2, a3, a4, a5);
  newpos = strlen(msgbuf);
}

//putmsg: put a msg on the line, make sure that it will fit, if it won't scroll msg sideways until he has read it all
void putmsg(int msgline, const char *msg)
{
  const char *curmsg, *lastmsg=0, *tmpmsg;
  int curlen;

  curmsg = msg;
  do
  {
    scrl(msgline, lastmsg, curmsg);
    newpos = curlen = strlen(curmsg);
    if (curlen>COLS)
    {
      more(" Cont ");
      lastmsg = curmsg;
      do
      {
        tmpmsg = stpbrk(curmsg, " ");
        //If there are no blanks in line
        if ((tmpmsg==0 || tmpmsg>=&lastmsg[COLS]) && lastmsg==curmsg) {curmsg = &lastmsg[COLS]; break;}
        if ((tmpmsg>=(lastmsg+COLS)) || (strlen(curmsg)< (size_t)COLS)) break;
        curmsg = tmpmsg+1;
      } while (1);
    }
  } while (curlen>COLS);
}

//scrl: scroll a message across the line
void scrl(int msgline, const char *str1, const char *str2)
{
  char *fmt;

  if (COLS>40) fmt = "%.80s"; else fmt = "%.40s";
  if (str1==0)
  {
    move(msgline, 0);
    if (strlen(str2)<(size_t)COLS) clrtoeol();
    printw(fmt, str2);
  }
  else while (str1<=str2)
  {
    move(msgline, 0);
    printw(fmt, str1++);
    if (strlen(str1)<(size_t)(COLS-1)) clrtoeol();
  }
}

//unctrl: Print a readable version of a certain character
char *unctrl(unsigned char ch)
{
  static char chstr[9]; //Defined in curses library

  if (isspace(ch)) strcpy(chstr, " ");
  else if (!isprint(ch)) if (ch<' ') sprintf(chstr, "^%c", ch+'@'); else sprintf(chstr, "\\x%x", ch);
  else {chstr[0] = ch; chstr[1] = 0;}
  return chstr;
}

//status: Display the important stats line.  Keep the cursor where it was.
void status()
{
  int oy, ox;
  static int s_hungry;
  static int s_level, s_pur = -1, s_hp, s_ac = 0;
  static unsigned int strength;
  static int s_elvl = 0;
  static char *state_name[] = {"      ", "Hungry", "Weak", "Faint","?"};

  SIG2();
  getrc(&oy, &ox);
  yellow();
  //Level:
  if (s_level!=get_level())
  {
    s_level = get_level();
    move(PT(22, 23), 0);
    printw("Level:%-4d", get_level());
  }
  //Hits:
  if (s_hp!=player.stats.hp)
  {
    s_hp = player.stats.hp;
    move(PT(22, 23), 12);
    if (player.stats.hp<100) {
      printw("Hits:%2d(%2d) ", player.stats.hp, player.stats.max_hp);
      //just in case they get wraithed with 3 digit max hits
      addstr("  ");
    }
    else printw("Hits:%3d(%3d) ", player.stats.hp, player.stats.max_hp);
  }
  //Str:
  if (player.stats.str!=strength)
  {
    strength = player.stats.str;
    move(PT(22, 23), 26);
    printw("Str:%2d(%2d) ", player.stats.str, max_stats.str);
  }
  //Gold
  if(s_pur != get_purse())
  {
    s_pur = get_purse();
    move(23, PT(0, 40));
    printw("Gold:%-5u", get_purse());
  }
  //Armor:
  if(s_ac!=(get_current_armor()!=NULL?get_current_armor()->armor_class:player.stats.ac))
  {
    s_ac = (get_current_armor()!=NULL?get_current_armor()->armor_class:player.stats.ac);
    if (is_ring_on_hand(LEFT, R_PROTECT)) s_ac -= get_ring(LEFT)->ring_level;
    if (is_ring_on_hand(RIGHT, R_PROTECT)) s_ac -= get_ring(RIGHT)->ring_level;
    move(23, PT(12, 52));
    printw("Armor:%-2d", AC(get_current_armor()!=NULL?get_current_armor()->armor_class:player.stats.ac));
  }
  //Exp:
  if (s_elvl!=player.stats.level)
  {
    s_elvl = player.stats.level;
    move(23, PT(22, 62));
    printw("%-12s", he_man[s_elvl-1]);
  }
  //Hungry state
  if (s_hungry!=get_hungry_state())
  {
    s_hungry = get_hungry_state();
    move(24, PT(28, 58));
    addstr(state_name[0]);
    move(24, PT(28, 58));
    if (get_hungry_state()) {bold(); addstr(state_name[get_hungry_state()]); standend();}
  }
  standend();
  move(oy, ox);
}

//wait_for: Sit around until the guy types the right key
void wait_for(char ch)
{
  char c;

  if (ch=='\n') while ((c = readchar())!='\n' && c!='\r') continue;
  else while (readchar()!=ch) continue;
}

//show_win: Function used to display a window and wait before returning
void show_win(char *message)
{
  mvaddstr(0, 0, message);
  move(player.pos.y, player.pos.x);
  wait_for(' ');
}

//This routine reads information from the keyboard. It should do all the strange processing that is needed to retrieve sensible data from the user
int getinfo(char *str, int size)
{
  char *retstr, ch;
  int readcnt = 0;
  int wason, ret = 1;

  retstr = str;
  *str = 0;
  wason = cursor(TRUE);
  while (ret==1) switch (ch = getkey())
  {
  case ESCAPE:
    while (str!=retstr) {backspace(); readcnt--; str--;}
    ret = *str = ESCAPE;
    cursor(wason);
    break;
  case '\b':
    if (str!=retstr) {backspace(); readcnt--; str--;}
    break;
  default:
    if (readcnt>=size) {beep(); break;}
    readcnt++;
    addch(ch);
    *str++ = ch;
    if ((ch&0x80)==0) break;
  case '\n':
  case '\r':
    *str = 0;
    cursor(wason);
    ret = ch;
    break;
  }
  return ret;
}

void backspace()
{
  int x, y;
  getrc(&x, &y);
  if (--y<0) y = 0;
  move(x, y);
  putchar(' ');
  move(x, y);
}

//str_attr: format a string with attributes.
//
//    formats:
//        %i - the following character is turned inverse vidio
//        %I - All characters upto %$ or null are turned inverse vidio
//        %u - the following character is underlined
//        %U - All characters upto %$ or null are underlined
//        %$ - Turn off all attributes
//
//     Attributes do not nest, therefore turning on an attribute while
//     a different one is in effect simply changes the attribute.
//
//     "No attribute" is the default and is set on leaving this routine
//
//     Eventually this routine will contain colors and character intensity
//     attributes.  And I'm not sure how I'm going to interface this with
//     printf certainly '%' isn't a good choice of characters.  jll.

void str_attr(char *str)
{
  while (*str)
  {
    if (*str=='%') {str++; standout();}
    addch(*str++);
    standend();
  }
}

//key_state:
void SIG2()
{
  static int numl, capsl;
  static int nspot, cspot, tspot;
  int new_numl=is_num_lock_on(), new_capsl=is_caps_lock_on(), new_fmode=is_scroll_lock_on();
  static int bighand, littlehand;
  int showtime = FALSE, spare;
  int x, y;

  if (COLS==40) {nspot = 10; cspot = 19; tspot = 35;}
  else {nspot = 20; cspot = 39; tspot = 75;}

  getrc(&x, &y);
  if (faststate!=new_fmode)
  {
    faststate = new_fmode;
    count = 0;
    show_count();
    running = FALSE;
    move(LINES-1, 0);
    if (faststate) {bold(); addstr("Fast Play"); standend();}
    else addstr("         ");
  }
  if (numl!=new_numl)
  {
    numl = new_numl;
    count = 0;
    show_count();
    running = FALSE;
    move(LINES-1, nspot);
    if (numl) {bold(); addstr("NUM LOCK"); standend();}
    else addstr("        ");
  }
  if (capsl!=new_capsl)
  {
    capsl = new_capsl;
    move(LINES-1, cspot);
    if (capsl) {bold(); addstr("CAP LOCK"); standend();}
    else addstr("        ");
  }
  if (showtime)
  {
    showtime = FALSE;
    //work around the compiler buggie boos
    spare = littlehand%10;
    move(24, tspot);
    bold();
    printw("%2d:%1d%1d", bighand?bighand:12, littlehand/10, spare);
    standend();
  }
  move(x, y);
}

char *noterse(char *str)
{
  return (short_msgs() ? "" : str);
}
