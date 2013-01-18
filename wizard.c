//Special wizard commands (some of which are also non-wizard commands under strange circumstances)
//wizard.c    1.4 (AI Design) 12/14/84

#include "rogue.h"
#include "daemons.h"
#include "pack.h"
#include "list.h"
#include "curses.h"
#include "io.h"
#include "wizard.h"
#include "daemon.h"
#include "main.h"
#include "things.h"
#include "rooms.h"
#include "misc.h"
#include "new_leve.h"
#include "monsters.h"
#include "mach_dep.h"

//whatis: What a certain object is
void whatis()
{
  THING *obj;

  if (ppack==NULL) {msg("You don't have anything in your pack to identify"); return;}
  for (;;) if ((obj = get_item("identify", 0))==NULL)
  {
    msg("You must identify something");
    msg(" ");
    mpos = 0;
  }
  else break;
  switch (obj->o_type)
  {
    case SCROLL:
      s_know[obj->o_which] = TRUE;
      *s_guess[obj->o_which] = 0;
    break;

    case POTION:
      p_know[obj->o_which] = TRUE;
      *p_guess[obj->o_which] = 0;
    break;

    case STICK:
      ws_know[obj->o_which] = TRUE;
      obj->o_flags |= ISKNOW;
      *ws_guess[obj->o_which] = 0;
    break;

    case WEAPON: case ARMOR:
      obj->o_flags |= ISKNOW;
    break;

    case RING:
      r_know[obj->o_which] = TRUE;
      obj->o_flags |= ISKNOW;
      *r_guess[obj->o_which] = 0;
    break;
  }
  //If it is vorpally enchanted, then reveal what type of monster it is vorpally enchanted against
  if (obj->o_enemy) obj->o_flags |= ISREVEAL;
  msg(inv_name(obj, FALSE));
}

#ifdef WIZARD

//create_obj: Wizard command for getting anything he wants
create_obj()
{
  THING *obj;
  byte ch, bless;

  if ((obj = new_item())==NULL) {msg("can't create anything now"); return;}
  msg("type of item: ");
  switch (readchar())
  {
    case '!': obj->o_type = POTION; break;
    case '?': obj->o_type = SCROLL; break;
    case '/': obj->o_type = STICK; break;
    case '=': obj->o_type = RING; break;
    case ')': obj->o_type = WEAPON; break;
    case ']': obj->o_type = ARMOR; break;
    case ',': obj->o_type = AMULET; break;
    default: obj->o_type = FOOD; break;
  }
  mpos = 0;
  msg("which %c do you want? (0-f)", obj->o_type);
  obj->o_which = (isdigit((ch = readchar()))?ch-'0':ch-'a'+10);
  obj->o_group = 0;
  obj->o_count = 1;
  obj->o_damage = obj->o_hurldmg = "0d0";
  mpos = 0;
  if (obj->o_type==WEAPON || obj->o_type==ARMOR)
  {
    msg("blessing? (+,-,n)");
    bless = readchar();
    mpos = 0;
    if (bless=='-') obj->o_flags |= ISCURSED;
    if (obj->o_type==WEAPON)
    {
      init_weapon(obj, obj->o_which);
      if (bless=='-') obj->o_hplus -= rnd(3)+1;
      if (bless=='+') obj->o_hplus += rnd(3)+1;
    }
    else
    {
      obj->o_ac = a_class[obj->o_which];
      if (bless=='-') obj->o_ac += rnd(3)+1;
      if (bless=='+') obj->o_ac -= rnd(3)+1;
    }
  }
  else if (obj->o_type==RING) switch (obj->o_which)
  {
    case R_PROTECT: case R_ADDSTR: case R_ADDHIT: case R_ADDDAM:
      msg("blessing? (+,-,n)");
      bless = readchar();
      mpos = 0;
      if (bless=='-') obj->o_flags |= ISCURSED;
      obj->o_ac = (bless=='-'?-1:rnd(2)+1);
    break;

    case R_AGGR: case R_TELEPORT:
      obj->o_flags |= ISCURSED;
    break;
  }
  else if (obj->o_type==STICK) fix_stick(obj);
  else if (obj->o_type==GOLD) {msg("how much?"); get_num(&obj->o_goldval, stdscr);}
  add_pack(obj, FALSE);
}

#endif

//teleport: Bamf the hero someplace else
int teleport()
{
  int rm;
  coord c;

  mvaddch(hero.y, hero.x, chat(hero.y, hero.x));
  do {rm = rnd_room(); rnd_pos(&rooms[rm], &c);} while (!(step_ok(winat(c.y, c.x))));
  if (&rooms[rm]!=proom) {leave_room(&hero); bcopy(hero, c); enter_room(&hero);}
  else {bcopy(hero, c); look(TRUE);}
  mvaddch(hero.y, hero.x, PLAYER);
  //turn off ISHELD in case teleportation was done while fighting a Fungi
  if (on(player, ISHELD)) {player.t_flags &= ~ISHELD; f_restor();}
  no_move = 0;
  count = 0;
  running = FALSE;
  flush_type();
  //Teleportation can be a confusing experience (unless you really are a wizard)
#ifdef WIZARD
  if (!wizard)
  {
#endif WIZARD
    if (on(player, ISHUH)) lengthen(unconfuse, rnd(4)+2);
    else fuse(unconfuse, 0, rnd(4)+2);
    player.t_flags |= ISHUH;
#ifdef WIZARD
  }
#endif WIZARD
  return rm;
}

#ifdef UNIX
#ifdef WIZARD

//passwd: See if user knows password
passwd()
{
  char *sp, c;
  char buf[MAXSTR];

  msg("wizard's Password:");
  mpos = 0;
  sp = buf;
  while ((c = getchar())!='\n' && c!='\r' && c!=ESCAPE)
  if (c==_tty.sg_kill) sp = buf;
  else if (c==_tty.sg_erase && sp>buf) sp--;
  else *sp++ = c;
  if (sp==buf) return FALSE;
  *sp = '\0';
  return (strcmp(PASSWD, crypt(buf, "mT"))==0);
}

#endif
#endif

#ifdef WIZARD

//show_map: Print out the map for the wizard
show_map()
{
  int y, x, real;

  wdump(0);
  wclear(hw);
  for (y = 1; y<maxrow; y++) for (x = 0; x<COLS; x++)
  {
    if (!(real = flat(y, x)&F_REAL)) standout();
    mvaddch(y, x, chat(y, x));
    if (!real) standend();
  }
  show_win(hw, "---More (level map)---");
  wrestor(0);
}

get_num(int *place)
{
  char numbuf[12];

  getinfo(numbuf, 10);
  *place = atoi(numbuf);
  return (*place);
}

#endif
