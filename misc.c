//All sorts of miscellaneous routines
//misc.c       1.4             (A.I. Design)   12/14/84

#include <stdio.h>

#include "rogue.h"
#include "io.h"
#include "daemons.h"
#include "misc.h"
#include "monsters.h"
#include "pack.h"
#include "curses.h"
#include "chase.h"
#include "rip.h"
#include "new_leve.h"
#include "list.h"
#include "mach_dep.h"
#include "main.h"
#include "fight.h"
#include "daemon.h"
#include "level.h"

//tr_name: Print the name of a trap
char *tr_name(byte type)
{
  switch (type)
  {
  case T_DOOR: return "a trapdoor";
  case T_BEAR: return "a beartrap";
  case T_SLEEP: return "a sleeping gas trap";
  case T_ARROW: return "an arrow trap";
  case T_TELEP: return "a teleport trap";
  case T_DART: return "a poison dart trap";
  }
  msg("weird trap: %d", type);
  return NULL;
}

//look: A quick glance all around the player
void look(bool wakeup)
{
  int x, y;
  byte ch, pch;
  THING *tp;
  struct room *rp;
  int ey, ex;
  int passcount = 0;
  byte pfl, fp;
  int sy, sx, sumhero, diffhero;

  rp = proom;
  pfl = flat(hero.y, hero.x);
  pch = chat(hero.y, hero.x);
  //if the hero has moved
  if (!ce(oldpos, hero))
  {
    if (!on(player, ISBLIND))
    {
      for (x = oldpos.x-1; x<=(oldpos.x+1); x++)
        for (y = oldpos.y-1; y<=(oldpos.y+1); y++)
        {
          if ((y==hero.y && x==hero.x) || offmap(y,x)) continue;
          move(y, x);
          ch = curch();
          if (ch==FLOOR)
          {
            if ((oldrp->r_flags & (ISGONE|ISDARK))==ISDARK) addch(' ');
          }
          else
          {
            fp = flat(y, x);
            //if the maze or passage (that the hero is in!!) needs to be redrawn (passages once drawn always stay on) do it now.
            if (((fp&F_MAZE) || (fp&F_PASS)) && (ch!=PASSAGE) && (ch!=STAIRS) && ((fp&F_PNUM)==(pfl & F_PNUM))) 
              addch(PASSAGE);
          }
        }
    }
    oldpos = hero;
    oldrp = rp;
  }
  ey = hero.y+1;
  ex = hero.x+1;
  sx = hero.x-1;
  sy = hero.y-1;
  if (door_stop && !firstmove && running) {sumhero = hero.y+hero.x; diffhero = hero.y-hero.x;}
  for (y = sy; y<=ey; y++) if (y>0 && y<maxrow) for (x = sx; x<=ex; x++)
  {
    if (x<=0 || x>=COLS) continue;
    if (!on(player, ISBLIND))
    {
      if (y==hero.y && x==hero.x) continue;
    }
    else if (y!=hero.y || x!=hero.x) continue;
    //THIS REPLICATES THE moat() MACRO.  IF MOAT IS CHANGED, THIS MUST BE CHANGED ALSO ?? What does this really mean ??
    fp = flat(y, x);
    ch = chat(y, x);
    //No Doors
    if (pch!=DOOR && ch!=DOOR)
      //Either hero or other in a passage
      if ((pfl&F_PASS)!=(fp & F_PASS))
      {
        //Neither is in a maze
        if (!(pfl&F_MAZE) && !(fp&F_MAZE)) continue;
      }
      //Not in same passage
      else if ((fp&F_PASS) && (fp&F_PNUM)!=(pfl & F_PNUM)) continue;
      if ((tp = moat(y, x))!=NULL) if (on(player, SEEMONST) && on(*tp, ISINVIS))
      {
        if (door_stop && !firstmove) running = FALSE;
        continue;
      }
      else
      {
        if (wakeup) wake_monster(y, x);
        if (tp->t_oldch != ' ' || (!(rp->r_flags&ISDARK) && !on(player, ISBLIND))) tp->t_oldch = chat(y, x);
        if (see_monst(tp)) ch = tp->t_disguise;
      }
      //The current character used for IBM ARMOR doesn't look right in Inverse
      if ((ch!=PASSAGE) && (fp&(F_PASS|F_MAZE))) if (ch!=ARMOR) standout();
      move(y, x);
      addch(ch);
      standend();
      if (door_stop && !firstmove && running)
      {
        switch (runch)
        {
        case 'h': if (x==ex) continue; break;
        case 'j': if (y==sy) continue; break;
        case 'k': if (y==ey) continue; break;
        case 'l': if (x==sx) continue; break;
        case 'y': if ((y+x)-sumhero>=1) continue; break;
        case 'u': if ((y-x)-diffhero>=1) continue; break;
        case 'n': if ((y+x)-sumhero<=-1) continue; break;
        case 'b': if ((y-x)-diffhero<=-1) continue; break;
        }
        switch (ch)
        {
        case DOOR: if (x==hero.x || y==hero.y) running = FALSE; break;
        case PASSAGE: if (x==hero.x || y==hero.y) passcount++; break;
        case FLOOR: case VWALL: case HWALL: case ULWALL: case URWALL: case LLWALL: case LRWALL: case ' ': break;
        default: running = FALSE; break;
        }
      }
  }
  if (door_stop && !firstmove && passcount>1) running = FALSE;
  move(hero.y, hero.x);
  if ((flat(hero.y, hero.x)&F_PASS) || (was_trapped>TRUE) || (flat(hero.y, hero.x)&F_MAZE)) standout();
  addch(PLAYER);
  standend();
  if (was_trapped) {beep(); was_trapped = FALSE;}
}

//find_obj: Find the unclaimed object at y, x
THING *find_obj(int y, int x)
{
  THING *op;

  for (op = lvl_obj; op!=NULL; op = next(op)) if (op->o_pos.y==y && op->o_pos.x==x) return op;

  debug("Non-object %c %d,%d", chat(y, x), y, x);
  return NULL; //NOTREACHED
}

//eat: She wants to eat something, so let her try
void eat()
{
  THING *obj;

  if ((obj = get_item("eat", FOOD))==NULL) 
    return;
  if (obj->o_type!=FOOD) {
    msg("ugh, you would get ill if you ate that"); 
    return;
  }
  inpack--;
  if (--obj->o_count<1) {
    detach(ppack, obj); 
    discard(obj);
  }
  if (food_left<0)
    food_left = 0;
  if (food_left>(STOMACHSIZE-20))
    no_command += 2+rnd(5);
  if ((food_left += HUNGERTIME-200+rnd(400))>STOMACHSIZE)
    food_left = STOMACHSIZE;
  hungry_state = 0;
  if (obj==cur_weapon) 
    cur_weapon = NULL;
  if (obj->o_which==1)
    msg("my, that was a yummy %s", fruit);
  else if (rnd(100)>70) {
    pstats.s_exp++; 
    msg("yuk, this food tastes awful");
    check_level();
  }
  else 
    msg("yum, that tasted good");
  if (no_command) 
    msg("You feel bloated and fall asleep");
}

//chg_str: Used to modify the player's strength.  It keeps track of the highest it has been, just in case
void chg_str(int amt)
{
  str_t comp;

  if (amt==0) return;
  add_str(&pstats.s_str, amt);
  comp = pstats.s_str;
  if (ISRING(LEFT, R_ADDSTR)) add_str(&comp, -cur_ring[LEFT]->o_ac);
  if (ISRING(RIGHT, R_ADDSTR)) add_str(&comp, -cur_ring[RIGHT]->o_ac);
  if (comp>max_stats.s_str) max_stats.s_str = comp;
}

//add_str: Perform the actual add, checking upper and lower bound
void add_str(str_t *sp, int amt)
{
  if ((*sp += amt)<3) *sp = 3;
  else if (*sp>31) *sp = 31;
}

//add_haste: Add a haste to the player
int add_haste(bool potion)
{
  if (on(player, ISHASTE))
  {
    no_command += rnd(8);
    player.t_flags &= ~ISRUN;
    extinguish(nohaste);
    player.t_flags &= ~ISHASTE;
    msg("you faint from exhaustion");
    return FALSE;
  }
  else
  {
    player.t_flags |= ISHASTE;
    if (potion) fuse(nohaste, 0, rnd(4)+10);
    return TRUE;
  }
}

//aggravate: Aggravate all the monsters on this level
void aggravate()
{
  THING *mi;

  for (mi = mlist; mi!=NULL; mi = next(mi)) start_run(&mi->t_pos);
}

//vowelstr: For printfs: if string starts with a vowel, return "n" for an "an".

char *vowelstr(char *str)
{
  switch (*str)
  {
  case 'a': case 'A': case 'e': case 'E': case 'i': case 'I': case 'o': case 'O': case 'u': case 'U': return "n";
  default: return "";
  }
}

//is_current: See if the object is one of the currently used items
int is_current(THING *obj)
{
  if (obj==NULL) return FALSE;
  if (obj==cur_armor || obj==cur_weapon || obj==cur_ring[LEFT] || obj==cur_ring[RIGHT])
  {
    msg("That's already in use");
    return TRUE;
  }
  return FALSE;
}

//get_dir: Set up the direction co_ordinate for use in various "prefix" commands
int get_dir()
{
  int ch;

  if (again) return TRUE;
  msg("which direction? ");
  do {
    if ((ch = readchar())==ESCAPE) {
      msg(""); 
      return FALSE;
    }
  } while (find_dir(ch, &delta)==0);
  msg("");
  if (on(player, ISHUH) && rnd(5)==0) do
  {
    delta.y = rnd(3)-1;
    delta.x = rnd(3)-1;
  } while (delta.y==0 && delta.x==0);
  return TRUE;
}

bool find_dir(byte ch, coord *cp)
{
  bool gotit;

  gotit = TRUE;
  switch (ch)
  {
  case 'h': case'H': cp->y = 0; cp->x = -1; break;
  case 'j': case'J': cp->y = 1; cp->x = 0; break;
  case 'k': case'K': cp->y = -1; cp->x = 0; break;
  case 'l': case'L': cp->y = 0; cp->x = 1; break;
  case 'y': case'Y': cp->y = -1; cp->x = -1; break;
  case 'u': case'U': cp->y = -1; cp->x = 1; break;
  case 'b': case'B': cp->y = 1; cp->x = -1; break;
  case 'n': case'N': cp->y = 1; cp->x = 1; break;
  default: gotit = FALSE; break;
  }
  return gotit;
}

//sign: Return the sign of the number
int sign(int nm)
{
  if (nm<0) return -1;
  else return (nm>0);
}

//spread: Give a spread around a given number (+/- 10%)
int spread(int nm)
{
  int r = nm-nm/10+rnd(nm/5);
  return r;
}

//call_it: Call an object something after use.
void call_it(bool know, char **guess)
{
  if (know && **guess) **guess = 0;
  else if (!know && **guess==0)
  {
    msg("%scall it? ", noterse("what do you want to "));
    getinfo(prbuf, MAXNAME);
    if (*prbuf!=ESCAPE) strcpy(*guess, prbuf);
    msg("");
  }
}

//step_ok: Returns true if it is ok to step on ch
int step_ok(int ch)
{
  switch (ch)
  {
  case ' ': case VWALL: case HWALL: case ULWALL: case URWALL: case LLWALL: case LRWALL: return FALSE;
  default: return ((ch<'A') || (ch>'Z'));
  }
}

//goodch: Decide how good an object is and return the correct character for printing.
int goodch(THING *obj)
{
  int ch = MAGIC;

  if (obj->o_flags&ISCURSED) ch = BMAGIC;
  switch (obj->o_type)
  {
  case ARMOR:
    if (obj->o_ac>a_class[obj->o_which]) ch = BMAGIC;
    break;
  case WEAPON:
    if (obj->o_hplus<0 || obj->o_dplus<0) ch = BMAGIC;
    break;
  case SCROLL:
    switch (obj->o_which) {case S_SLEEP: case S_CREATE: case S_AGGR: ch = BMAGIC; break;}
    break;
  case POTION:
    switch (obj->o_which) {case P_CONFUSE: case P_PARALYZE: case P_POISON: case P_BLIND: ch = BMAGIC; break;}
    break;
  case STICK:
    switch (obj->o_which) {case WS_HASTE_M: case WS_TELTO: ch = BMAGIC; break;}
    break;
  case RING:
    switch (obj->o_which)
    {
    case R_PROTECT: case R_ADDSTR: case R_ADDDAM: case R_ADDHIT: if (obj->o_ac<0) ch = BMAGIC; break;
    case R_AGGR: case R_TELEPORT: ch = BMAGIC; break;
    }
    break;
  }
  return ch;
}

//help: prints out help screens
void help(char **helpscr)
{
  int hcount = 0;
  int hrow, hcol;
  int isfull;
  byte answer = 0;

  wdump();
  while (*helpscr && answer!=ESCAPE)
  {
    isfull = FALSE;
    if ((hcount%(terse?23:46))==0) clear();
    //determine row and column
    hcol = 0;
    if (terse)
    {
      hrow = hcount%23;
      if (hrow==22) isfull = TRUE;
    }
    else
    {
      hrow = (hcount%46)/2;
      if (hcount%2) hcol = 40;
      if (hrow==22 && hcol==40) isfull = TRUE;
    }
    move (hrow, hcol);
    addstr(*helpscr++);
    //decide if we need print a continue type message
    if ((*helpscr==0) || isfull)
    {
      if (*helpscr==0) 
        mvaddstr(24, 0, "--press space to continue--");
      else if (terse) 
        mvaddstr(24, 0, "--Space for more, Esc to continue--");
      else 
        mvaddstr(24, 0, "--Press space for more, Esc to continue--");
      do {
        answer = readchar();
      } while (answer!=' ' && answer!=ESCAPE);
    }
    hcount++;
  }
  wrestor();
}

int DISTANCE(int y1, int x1, int y2, int x2)
{
  int dx, dy;

  dx = (x1-x2);
  dy = (y1-y2);
  return dx*dx+dy*dy;
}

int _ce(coord *a, coord *b)
{
  return (a->x==b->x && a->y==b->y);
}

int INDEX(int y, int x)
{
  return ((x*(maxrow-1))+y-1);
}

int offmap(int y, int x)
{
  return (y<1 || y>=maxrow || x<0 || x>=COLS);
}

int winat(int y, int x)
{
  return (moat(y, x)!=NULL?moat(y, x)->t_disguise:chat(y, x));
}

//search: Player gropes about him to find hidden things.
void search()
{
  int y, x;
  byte fp;
  int ey, ex;

  if (on(player, ISBLIND)) 
    return;
  ey = hero.y+1;
  ex = hero.x+1;
  for (y = hero.y-1; y<=ey; y++) for (x = hero.x-1; x<=ex; x++)
  {
    if ((y==hero.y && x==hero.x) || offmap(y, x)) 
      continue;
    fp = flat(y, x);
    if (!(fp&F_REAL)) {
      switch (chat(y, x))
      {
      case VWALL: case HWALL: case ULWALL: case URWALL: case LLWALL: case LRWALL:
        if (rnd(5)!=0) break;
        set_chat(y, x, DOOR);
        set_flag(y, x, F_REAL);
        count = running = FALSE;
        break;
      case FLOOR:
        if (rnd(2)!=0) break;
        set_chat(y, x, TRAP);
        set_flag(y, x, F_REAL);
        count = running = FALSE;
        msg("you found %s", tr_name(fp&F_TMASK));
        break;
      }
    }
  }
}


//d_level: He wants to go down a level
void d_level()
{
  if (chat(hero.y, hero.x)!=STAIRS && wizard == FALSE)
    msg("I see no way down");
  else {
    level++; 
    new_level();
  }
}

//u_level: He wants to go up a level
void u_level()
{
  if (chat(hero.y, hero.x)==STAIRS || wizard == TRUE) 
    if (amulet) {
      level--; 
      if (level==0) 
        total_winner(); 
      new_level(); 
      msg("you feel a wrenching sensation in your gut");
    } 
    else msg("your way is magically blocked");
  else msg("I see no way up");
}

//call: Allow a user to call a potion, scroll, or ring something
void call()
{
  THING *obj;
  char **guess, *elsewise;
  bool *know;

  obj = get_item("call", CALLABLE);

  //Make certain that it is something that we want to wear
  if (obj==NULL) return;
  switch (obj->o_type)
  {
  case RING: guess = (char **)r_guess; 
    know = r_know;
    elsewise = (*guess[obj->o_which]!=0?guess[obj->o_which]:r_stones[obj->o_which]);
    break;
  case POTION: guess = (char **)p_guess;
    know = p_know;
    elsewise = (*guess[obj->o_which]!=0?guess[obj->o_which]:p_colors[obj->o_which]);
    break;
  case SCROLL: guess = (char **)s_guess;
    know = s_know;
    elsewise = (*guess[obj->o_which]!=0?guess[obj->o_which]:(char *)(&s_names[obj->o_which])); 
    break;
  case STICK: guess = (char **)ws_guess;
    know = ws_know;
    elsewise = (*guess[obj->o_which]!=0?guess[obj->o_which]:ws_made[obj->o_which]);
    break;
  default: msg("you can't call that anything"); return;
  }
  if (know[obj->o_which]) {
    msg("that has already been identified"); 
    return;
  }
  msg("Was called \"%s\"", elsewise);
  msg("what do you want to call it? ");
  getinfo(prbuf,MAXNAME);
  if (*prbuf && *prbuf!=ESCAPE) 
    strcpy(guess[obj->o_which], prbuf);
  msg("");
}

//prompt player for definition of macro
void do_macro(char *buf, int sz)
{
  char *cp = prbuf;

  msg("F9 was %s, enter new macro: ", buf);
  if (getinfo(prbuf, sz-1)!=ESCAPE) 
    do {
      if (*cp!=CTRL('F')) 
        *buf++ = *cp;
    } while (*cp++);
  msg("");
  flush_type();
}
