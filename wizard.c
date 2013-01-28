//Special wizard commands (some of which are also non-wizard commands under strange circumstances)
//wizard.c    1.4 (AI Design) 12/14/84

#include <ctype.h>
#include <stdlib.h>

#include "rogue.h"
#include "daemons.h"
#include "pack.h"
#include "thing.h"
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
#include "sticks.h"
#include "rings.h"
#include "scrolls.h"
#include "potions.h"
#include "weapons.h"
#include "level.h"
#include "armor.h"

//whatis: What a certain object is
void whatis()
{
  ITEM *obj;

  if (player.pack==NULL) {msg("You don't have anything in your pack to identify"); return;}
  for (;;) if ((obj = get_item("identify", 0))==NULL)
  {
    msg("You must identify something");
    msg(" ");
    mpos = 0;
  }
  else break;
  switch (obj->type)
  {
  case SCROLL:
    discover_scroll(obj->which);
    *s_guess[obj->which] = 0;
    break;

  case POTION:
    discover_potion(obj->which);
    *p_guess[obj->which] = 0;
    break;

  case STICK:
    discover_stick(obj->which);
    obj->flags |= ISKNOW;
    *ws_guess[obj->which] = 0;
    break;

  case WEAPON: case ARMOR:
    obj->flags |= ISKNOW;
    break;

  case RING:
    discover_ring(obj->which);
    obj->flags |= ISKNOW;
    *r_guess[obj->which] = 0;
    break;
  }
  //If it is vorpally enchanted, then reveal what type of monster it is vorpally enchanted against
  if (obj->enemy) obj->flags |= ISREVEAL;
  msg(inv_name(obj, FALSE));
}

//create_obj: Wizard command for getting anything he wants
void create_obj()
{
  ITEM *obj;
  byte ch, bless;
  int limit, option;

  if ((obj = create_item(0,0))==NULL) {msg("can't create anything now"); return;}
  msg("type of item: ");
  switch (readchar())
  {
  case '!': obj->type = POTION; limit=MAXPOTIONS-1; break;
  case '?': obj->type = SCROLL; limit=MAXSCROLLS-1; break;
  case '/': obj->type = STICK; limit=MAXSTICKS-1; break;
  case '=': obj->type = RING; limit=MAXRINGS-1; break;
  case ')': obj->type = WEAPON; limit=MAXWEAPONS-1; break;
  case ']': obj->type = ARMOR; limit=MAXARMORS-1; break;
  case ',': obj->type = AMULET; limit=0; break;
  default: obj->type = FOOD; limit=1; break;
  }
  mpos = 0;
  msg("which %c do you want? (0-%x)", obj->type, limit);
  option = (isdigit((ch = readchar()))?ch-'0':ch-'a'+10);
  obj->which = (option >= 0 && option <= limit) ? option : 0;
  obj->group = 0;
  obj->count = 1;
  obj->damage = obj->throw_damage = "0d0";
  mpos = 0;
  if (obj->type==WEAPON || obj->type==ARMOR)
  {
    msg("blessing? (+,-,n)");
    bless = readchar();
    mpos = 0;
    if (bless=='-') obj->flags |= ISCURSED;
    if (obj->type==WEAPON)
    {
      init_weapon(obj, obj->which);
      if (bless=='-') obj->hit_plus -= rnd(3)+1;
      if (bless=='+') obj->hit_plus += rnd(3)+1;
    }
    else
    {
      obj->armor_class = get_default_class(obj->which);
      if (bless=='-') obj->armor_class += rnd(3)+1;
      if (bless=='+') obj->armor_class -= rnd(3)+1;
    }
  }
  else if (obj->type==RING) switch (obj->which)
  {
  case R_PROTECT: case R_ADDSTR: case R_ADDHIT: case R_ADDDAM:
    msg("blessing? (+,-,n)");
    bless = readchar();
    mpos = 0;
    if (bless=='-') obj->flags |= ISCURSED;
    obj->armor_class = (bless=='-'?-1:rnd(2)+1);
    break;

  case R_AGGR: case R_TELEPORT:
    obj->flags |= ISCURSED;
    break;
  }
  else if (obj->type==STICK) fix_stick(obj);
  else if (obj->type==GOLD) {msg("how much?"); get_num(&obj->gold_value);}
  add_pack(obj, FALSE);
}

//teleport: Bamf the hero someplace else
int teleport()
{
  int rm;
  Coord c;

  mvaddch(player.pos.y, player.pos.x, get_tile(player.pos.y, player.pos.x));
  do {rm = rnd_room(); rnd_pos(&rooms[rm], &c);} while (!(step_ok(display_character(c.y, c.x))));
  if (&rooms[rm]!=player.room) {leave_room(&player.pos); bcopy(player.pos, c); enter_room(&player.pos);}
  else {bcopy(player.pos, c); look(TRUE);}
  mvaddch(player.pos.y, player.pos.x, PLAYER);
  //turn off ISHELD in case teleportation was done while fighting a Flytrap
  if (on(player, ISHELD)) {player.flags &= ~ISHELD; f_restor();}
  no_move = 0;
  count = 0;
  running = FALSE;
  flush_type();
  //Teleportation can be a confusing experience (unless you really are a wizard)
  if (!wizard)
  {
    if (on(player, ISHUH)) lengthen(unconfuse, rnd(4)+2);
    else fuse(unconfuse, 0, rnd(4)+2);
    player.flags |= ISHUH;
  }
  return rm;
}

//show_map: Print out the map for the wizard
void show_map()
{
  int y, x, real;

  wdump(0);
  clear();
  for (y = 1; y<maxrow; y++) for (x = 0; x<COLS; x++)
  {
    if (!(real = get_flags(y, x)&F_REAL)) standout();
    mvaddch(y, x, get_tile(y, x));
    if (!real) standend();
  }
  show_win("---More (level map)---");
  wrestor(0);
}

int get_num(short *place)
{
  char numbuf[12];

  getinfo(numbuf, 10);
  *place = atoi(numbuf);
  return (*place);
}
