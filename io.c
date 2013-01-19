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

#define AC(a)    (-((a)-11))
#define PT(i,j)  ((COLS==40)?i:j)

extern unsigned tick;
extern char *msgbuf;

static int newpos = 0;
static char *formats = "scud%", *bp, left_justify;
static int min_width, max_width;
static char ibuf[6];

//msg: Display a message at the top of the screen.
void ifterse(char *tfmt, char *fmt, int a1, int a2, int a3, int a4, int a5)
{
  if (expert) msg(tfmt, a1, a2, a3, a4, a5);
  else msg(fmt, a1, a2, a3, a4, a5);
}

void msg(char *fmt, int a1, int a2, int a3, int a4, int a5)
{
  //if the string is "", just clear the line
  if (*fmt=='\0') {move(0, 0); clrtoeol(); mpos = 0; return;}
  //otherwise add to the message and flush it out
  doadd(fmt, a1,a2,a3,a4,a5);
  endmsg();
}

//addmsg: Add things to the current message
void addmsg(char *fmt, int a1, int a2, int a3, int a4, int a5)
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
void more(char *msg)
{
  int x, y;
  int i, msz;
  char mbuf[80];
  int morethere = TRUE;
  int covered = FALSE;

  msz = strlen(msg);
  getxy(&x,&y);
  //it is reasonable to assume that if the you are no longer on line 0, you must have wrapped.
  if (x!=0) {x = 0; y = COLS;}
  if ((y+msz)>COLS) {move(x, y = COLS-msz); covered = TRUE;}
  for (i=0; i<msz; i++)
  {
    mbuf[i] = inch();
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
void doadd(char *fmt, int a1, int a2, int a3, int a4, int a5)
{
  sprintf(&msgbuf[newpos], fmt, a1, a2, a3, a4, a5);
  newpos = strlen(msgbuf);
}

//putmsg: put a msg on the line, make sure that it will fit, if it won't scroll msg sideways until he has read it all
void putmsg(int msgline, char *msg)
{
  char *curmsg, *lastmsg=0, *tmpmsg;
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
void scrl(int msgline, char *str1, char *str2)
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
  static int s_lvl, s_pur = -1, s_hp, s_ac = 0;
  static str_t s_str;
  static int s_elvl = 0;
  static char *state_name[] = {"      ", "Hungry", "Weak", "Faint","?"};

  SIG2();
  getyx(stdscr, oy, ox);
  yellow();
  //Level:
  if (s_lvl!=level)
  {
    s_lvl = level;
    move(PT(22, 23), 0);
    printw("Level:%-4d", level);
  }
  //Hits:
  if (s_hp!=pstats.s_hpt)
  {
    s_hp = pstats.s_hpt;
    move(PT(22, 23), 12);
    if (pstats.s_hpt<100) {
       printw("Hits:%2d(%2d) ", pstats.s_hpt, max_hp);
       //just in case they get wraithed with 3 digit max hits
       addstr("  ");
    }
    else printw("Hits:%3d(%3d) ", pstats.s_hpt, max_hp);
  }
  //Str:
  if (pstats.s_str!=s_str)
  {
    s_str = pstats.s_str;
    move(PT(22, 23), 26);
    printw("Str:%2d(%2d) ", pstats.s_str, max_stats.s_str);
  }
  //Gold
  if(s_pur!=purse)
  {
    s_pur = purse;
    move(23, PT(0, 40));
    printw("Gold:%-5u", purse);
  }
  //Armor:
  if(s_ac!=(cur_armor!=NULL?cur_armor->o_ac:pstats.s_arm))
  {
    s_ac = (cur_armor!=NULL?cur_armor->o_ac:pstats.s_arm);
    if (ISRING(LEFT, R_PROTECT)) s_ac -= cur_ring[LEFT]->o_ac;
    if (ISRING(RIGHT, R_PROTECT)) s_ac -= cur_ring[RIGHT]->o_ac;
    move(23, PT(12, 52));
    printw("Armor:%-2d", AC(cur_armor!=NULL?cur_armor->o_ac:pstats.s_arm));
  }
  //Exp:
  if (s_elvl!=pstats.s_lvl)
  {
    s_elvl = pstats.s_lvl;
    move(23, PT(22, 62));
    printw("%-12s", he_man[s_elvl-1]);
  }
  //Hungry state
  if (s_hungry!=hungry_state)
  {
    s_hungry = hungry_state;
    move(24, PT(28, 58));
    addstr(state_name[0]);
    move(24, PT(28, 58));
    if (hungry_state) {bold(); addstr(state_name[hungry_state]); standend();}
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
  move(hero.y, hero.x);
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
  getxy(&x, &y);
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

#ifdef LUXURY

  int is_attr_on = FALSE, was_touched = FALSE;

  while(*str)
  {
    if (was_touched==TRUE) {standend(); is_attr_on = FALSE; was_touched = FALSE;}
    if (*str=='%')
    {
      str++;
      switch(*str)
      {
        case 'u':
          was_touched = TRUE;
        case 'U':
          uline();
          is_attr_on = TRUE;
          str++;
        break;
        case 'i':
          was_touched = TRUE;
        case 'I':
          standout();
          is_attr_on = TRUE;
          str++;
        break;
        case '$':
          if (is_attr_on) was_touched = TRUE;
          str++;
          continue;
      }
    }
    if ((*str=='\n') || (*str=='\r')) {str++; printw("\n");}
    else if (*str!=0) addch(*str++);
  }
  if (is_attr_on) standend();

#else

  while (*str)
  {
    if (*str=='%') {str++; standout();}
    addch(*str++);
    standend();
  }

#endif LUXURY

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
  
  getxy(&x, &y);
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

//Replacement printf
//Michael Toy, AI Design, January 1984

char *my_stccpy(char* a, char* b, int c)
{
  stccpy(a, b, c);
  return a+strlen(a);
}

char *noterse(char *str)
{
  return (terse || expert?nullstr:str);
}
