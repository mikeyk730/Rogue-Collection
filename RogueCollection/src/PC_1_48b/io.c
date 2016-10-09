//Various input/output functions
//io.c         1.4             (A.I. Design) 12/10/84

#include "rogue.h"

#define AC(a)    (-((a)-11))
#define PT(i,j)  ((COLS==40)?i:j)

extern int scr_type;
extern unsigned tick;
extern char *msgbuf;

static int newpos = 0;
static char *formats = "scud%", *bp, left_justify;
static int min_width, max_width;
static char ibuf[6];

static int pf_str(), pf_chr(), pf_uint(), pf_int(), pf_per();
static int (*(pfuncs[]))() = {pf_str, pf_chr, pf_uint, pf_int, pf_per};

//msg: Display a message at the top of the screen.
ifterse(char *tfmt, char *fmt, int a1, int a2, int a3, int a4, int a5)
{
  if (expert) msg(tfmt, a1, a2, a3, a4, a5);
  else msg(fmt, a1, a2, a3, a4, a5);
}

msg(char *fmt, int a1, int a2, int a3, int a4, int a5)
{
  //if the string is "", just clear the line
  if (*fmt=='\0') {move(0, 0); clrtoeol(); mpos = 0; return;}
  //otherwise add to the message and flush it out
  doadd(fmt, a1,a2,a3,a4,a5);
  endmsg();
}

//addmsg: Add things to the current message
addmsg(char *fmt, int a1, int a2, int a3, int a4, int a5)
{
  doadd(fmt, a1, a2, a3, a4, a5);
}

//endmsg: Display a new msg (giving him a chance to see the previous one if it is up there with the -More-)
endmsg()
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
more(char *msg)
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
doadd(char *fmt, int a1, int a2, int a3, int a4, int a5)
{
  sprintf(&msgbuf[newpos], fmt, a1, a2, a3, a4, a5);
  newpos = strlen(msgbuf);
}

//putmsg: put a msg on the line, make sure that it will fit, if it won't scroll msg sideways until he has read it all
putmsg(int msgline, char *msg)
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
        if ((tmpmsg>=(lastmsg+COLS)) || (strlen(curmsg)<COLS)) break;
        curmsg = tmpmsg+1;
      } while (1);
    }
  } while (curlen>COLS);
}

//scrl: scroll a message across the line
scrl(int msgline, char *str1, char *str2)
{
  char *fmt;
  int x,y;

  if (COLS>40) fmt = "%.80s"; else fmt = "%.40s";
  if (str1==0)
  {
    move(msgline, 0);
    if (strlen(str2)<COLS) clrtoeol();
    printw(fmt, str2);
  }
  else while (str1<=str2)
  {
    move(msgline, 0);
    printw(fmt, str1++);
    if (strlen(str1)<(COLS-1)) clrtoeol();
  }
}

//unctrl: Print a readable version of a certain character
char *unctrl(unsigned char ch)
{
  static chstr[9]; //Defined in curses library

  if (isspace(ch)) strcpy(chstr, " ");
  else if (!isprint(ch)) if (ch<' ') sprintf(chstr, "^%c", ch+'@'); else sprintf(chstr, "\\x%x", ch);
  else {chstr[0] = ch; chstr[1] = 0;}
  return chstr;
}

//status: Display the important stats line.  Keep the cursor where it was.
status()
{
  int oy, ox;
  static int s_hungry;
  static int s_lvl, s_pur = -1, s_hp, s_ac = 0;
  static str_t s_str;
  static int s_elvl = 0;
  static char *state_name[] = {"      ", "Hungry", "Weak", "Faint","?"};

  SIG2();
  getyx(stdscr, oy, ox);
  if (is_color) yellow();
  //Level:
  if (s_lvl!=level)
  {
    s_lvl = level;
    move(PT(22, 23), 0);
    printw("Level:%-4.4d", level);
  }
  //Hits:
  if (s_hp!=pstats.s_hpt)
  {
    s_hp = pstats.s_hpt;
    move(PT(22, 23), 12);
    printw("Hits:%.3d(%.3d) ", pstats.s_hpt, max_hp);
    //just in case they get wraithed with 3 digit max hits
    if (pstats.s_hpt<100) addch(' ');
  }
  //Str:
  if (pstats.s_str!=s_str)
  {
    s_str = pstats.s_str;
    move(PT(22, 23), 26);
    printw("Str:%.3d(%.3d) ", pstats.s_str, max_stats.s_str);
  }
  //Gold
  if(s_pur!=purse)
  {
    s_pur = purse;
    move(23, PT(0, 40));
    printw("Gold:%-5.5u", purse);
  }
  //Armor:
  if(s_ac!=(cur_armor!=NULL?cur_armor->o_ac:pstats.s_arm))
  {
    s_ac = (cur_armor!=NULL?cur_armor->o_ac:pstats.s_arm);
    if (ISRING(LEFT, R_PROTECT)) s_ac -= cur_ring[LEFT]->o_ac;
    if (ISRING(RIGHT, R_PROTECT)) s_ac -= cur_ring[RIGHT]->o_ac;
    move(23, PT(12, 52));
    printw("Armor:%-2.2d", AC(cur_armor!=NULL?cur_armor->o_ac:pstats.s_arm));
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
  if (is_color) standend();
  move(oy, ox);
}

//wait_for: Sit around until the guy types the right key
wait_for(char ch)
{
  char c;

  if (ch=='\n') while ((c = readchar())!='\n' && c!='\r') continue;
  else while (readchar()!=ch) continue;
}

//show_win: Function used to display a window and wait before returning
show_win(int *scr, char *message)
{
  mvaddstr(0, 0, message);
  move(hero.y, hero.x);
  wait_for(' ');
}

//This routine reads information from the keyboard. It should do all the strange processing that is needed to retrieve sensible data from the user
getinfo(char *str, int size)
{
  char *retstr, ch;
  int readcnt = 0;
  int wason, ret = 1;
  char buf[160];

  dmain(buf, 80, scr_ds, 0);
  retstr = str;
  *str = 0;
  wason = cursor(TRUE);
  while (ret==1) switch (ch = getch())
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
  dmaout(buf, 80, scr_ds, 0);
  return ret;
}

backspace()
{
  int x, y;

  getxy(&x, &y);
  if (--y<0) y = 0;
  move(x, y);
  putchr(' ');
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

str_attr(char *str)
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
SIG2()
{
  static unsigned icnt = 0, ntick = 0;
  static int key_init = TRUE;
  static int numl, capsl;
  static int nspot, cspot, tspot;
  int new_numl, new_capsl, new_fmode;
  static int bighand, littlehand;
  int showtime = FALSE, spare;
  int x, y;
  char wbuf[10];

#ifdef DEMO

  static tot_time = 0;

#endif DEMO

  if (tick<ntick) return;
  ntick = tick+6;
  if (is_saved || scr_type<0) return;
  regs->ax = 0x200;
  swint(SW_KEY, regs);
  new_numl = regs->ax;
  new_capsl = new_numl&0x40;
  new_fmode = new_numl&0x10;
  new_numl &= 0x20;
  //set up the clock the first time here
  if (key_init)
  {
    regs->ax = 0x2c<<8;
    swint(SW_DOS, regs);
    bighand = (regs->cx>>8)%12;
    littlehand = regs->cx&0xFF;
    showtime++;
  }
  if (tick>1092)
  {
    //time os call kills jr and others we keep track of it ourselves
    littlehand = (littlehand+1)%60;
    if (littlehand==0) bighand = (bighand+1)%12;
    tick = tick-1092;
    ntick = tick+6;
    showtime++;
  }
  //this is built for speed so set up once first time this is executed
  if (key_init || reinit)
  {
    reinit = key_init = FALSE;
    if (COLS==40) {nspot = 10; cspot = 19; tspot = 35;}
    else {nspot = 20; cspot = 39; tspot = 75;}
    //this will force all fields to be updated first time through
    numl = !new_numl;
    capsl = !new_capsl;
    showtime++;
    faststate = !new_fmode;
  }
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
    move(24, nspot);
    if (numl) {bold(); addstr("NUM LOCK"); standend();}
    else addstr("        ");
  }
  if (capsl!=new_capsl)
  {
    capsl = new_capsl;
    move(24, cspot);
    if (capsl) {bold(); addstr("CAP LOCK"); standend();}
    else addstr("        ");
  }
  if (showtime)
  {
    showtime = FALSE;

#ifdef DEMO

    //Don't let them get by level 10 because they might do something nasty like disable the clock
    if (((tot_time++ -max_level)>DEMOTIME) || max_level>10) demo(DEMOTIME);

#endif DEMO

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

char *my_stccpy(a, b, c)
{
  stccpy(a, b, c);
  return a+strlen(a);
}

char *sprintf(char *buf, char *fmt, int arg)
{
  char *cp, *init;
  int *ap = &arg, pad;
  char tbuf[128];

  init = buf;
  while (*fmt)
  {
    if (*fmt!='%') *buf++ = *fmt++;
    else
    {
      left_justify = max_width = 0;
      if (*++fmt=='-') left_justify++, fmt++;
      min_width = scan_num(fmt);
      if (*bp=='.') max_width = scan_num(++bp);
      fmt = bp;
      bp = tbuf;
      if (cp = stpchr(formats, *fmt)) ap += (*(pfuncs[cp-formats]))(ap);
      *bp = 0;
      if (max_width && strlen(tbuf)>max_width) tbuf[max_width] = 0;
      pad = min_width-strlen(tbuf);
      bp = buf;
      if (!left_justify) blanks(pad);
      bp = my_stccpy(bp, tbuf, 200);
      if (left_justify) blanks(pad);
      buf = bp;
      if (*fmt) fmt++;
    }
  }
  *buf = 0;
  return init;
}

scan_num(char *cp)
{
  int i = 0;

  bp = cp;
  while (isdigit(*bp)) i = i*10+*bp++ -'0';
  return i;
}

pf_str(char **cp)
{
  bp = my_stccpy(bp, *cp, 200);
  return 1;
}

blanks(cnt)
{
  while (cnt-->0) *bp++ = ' ';
  *bp = 0;
}

pf_chr(char *c)
{
  *bp++ = *c;
  return 1;
}

pf_int(int *ip)
{
  if (*ip<0) {*bp++ = '-'; *ip = (-*ip);}
  return pf_uint(ip);
}

pf_uint(unsigned int *ip)
{
  char *cp = ibuf, once;
  unsigned int i = *ip, d = 10000, r;

  if (*ip==0) {*ibuf = '0'; ibuf[1] = 0;}
  else
  {
    once = 0;
    while (d)
    {
      if ((r = i/d) || once) {*cp++ = r+'0'; once = 1; i -= r*d;}
      d /= 10;
    }
    *cp = 0;
  }
  bp = my_stccpy(bp, ibuf, 6);
  return 1;
}

pf_per(ip)
{
  *bp++ = '%';
  return 0;
}

noterse(char *str)
{
  return (terse || expert?nullstr:str);
}
