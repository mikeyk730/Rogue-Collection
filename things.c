//Contains functions for dealing with things like potions, scrolls, and other items.
//things.c     1.4 (AI Design) 12/14/84

#include <stdio.h>
#include <ctype.h>

#include "rogue.h"
#include "things.h"
#include "pack.h"
#include "list.h"
#include "sticks.h"
#include "io.h"
#include "misc.h"
#include "daemons.h"
#include "rings.h"
#include "weapons.h"
#include "curses.h"
#include "main.h"
#include "armor.h"
#include "daemon.h"
#include "mach_dep.h"
#include "level.h"
#include "thing.h"
#include "monsters.h"

#define MAX(a,b,c,d) (a>b?(a>c?(a>d?a:d):(c>d?c:d)):(b>c?(b>d?b:d):(c>d?c:d)))

static int line_cnt = 0;
static bool newpage = FALSE;
static char *lastfmt, *lastarg;

//inv_name: Return the name of something as it would appear in an inventory.

char *inv_name(ITEM *obj, bool drop)
{
  int which = obj->which;
  char *pb;

  pb = prbuf;
  switch (obj->type)
  {
  case SCROLL:
    if (obj->count==1) {
      strcpy(pb, "A scroll ");
      pb = &prbuf[9];
    }
    else {
      sprintf(pb, "%d scrolls ", obj->count); 
      pb = &prbuf[strlen(prbuf)];
    }
    if (s_know[which] || wizard) 
      sprintf(pb, "of %s", s_magic[which].name);
    else if (*s_guess[which]) 
      sprintf(pb, "called %s", s_guess[which]);
    else
      chopmsg(pb, "titled '%.17s'", "titled '%s'", &s_names[which]);
    break;

  case POTION:
    if (obj->count==1) {
      strcpy(pb, "A potion ");
      pb = &prbuf[9];
    }
    else {
      sprintf(pb, "%d potions ", obj->count); 
      pb = &pb[strlen(prbuf)];
    }
    if (p_know[which] || wizard) {
      chopmsg(pb, "of %s", "of %s(%s)", p_magic[which].name, p_colors[which]);
    }
    else if (*p_guess[which]) {
      chopmsg(pb, "called %s", "called %s(%s)", p_guess[which], p_colors[which]);
    }
    else if (obj->count==1) 
      sprintf(prbuf, "A%s %s potion", vowelstr(p_colors[which]), p_colors[which]);
    else sprintf(prbuf, "%d %s potions", obj->count, p_colors[which]);
    break;

  case FOOD:
    if (which==1) 
      if (obj->count==1)
        sprintf(pb, "A%s %s", vowelstr(fruit), fruit); 
      else sprintf(pb, "%d %ss", obj->count, fruit);
    else if (obj->count==1) 
      strcpy(pb, "Some food");
    else sprintf(pb, "%d rations of food", obj->count);
    break;

  case WEAPON:
    if (obj->count>1) 
      sprintf(pb, "%d ", obj->count);
    else
      sprintf(pb, "A%s ", vowelstr(w_names[which]));
    pb = &prbuf[strlen(prbuf)];
    if (obj->flags&ISKNOW || wizard) 
      sprintf(pb, "%s %s", num(obj->hit_plus, obj->damage_plus, WEAPON), w_names[which]);
    else
      sprintf(pb, "%s", w_names[which]);
    if (obj->count>1) strcat(pb, "s");
    if (obj->enemy && (obj->flags&ISREVEAL || wizard))
    {
      strcat(pb, " of ");
      strcat(pb, get_monster_name(obj->enemy));
      strcat(pb, " slaying");
    }
    break;

  case ARMOR:
    if (obj->flags&ISKNOW || wizard)
      chopmsg(pb, "%s %s", "%s %s [armor class %d]", num(a_class[which]-obj->armor_class, 0, ARMOR), a_names[which], -(obj->armor_class-11));
    else
      sprintf(pb, "%s", a_names[which]);
    break;

  case AMULET:
    strcpy(pb, "The Amulet of Yendor");
    break;

  case STICK:
    sprintf(pb, "A%s %s ", vowelstr(ws_type[which]), ws_type[which]);
    pb = &prbuf[strlen(prbuf)];
    if (ws_know[which] || wizard)
      chopmsg(pb, "of %s%s", "of %s%s(%s)", ws_magic[which].name, charge_str(obj), ws_made[which]);
    else if (*ws_guess[which])
      chopmsg(pb, "called %s", "called %s(%s)", ws_guess[which], ws_made[which]);
    else
      sprintf(pb = &prbuf[2], "%s %s", ws_made[which], ws_type[which]);
    break;

  case RING:
    if (r_know[which] || wizard)
      chopmsg(pb, "A%s ring of %s", "A%s ring of %s(%s)", ring_num(obj), r_magic[which].name, r_stones[which]);
    else if (*r_guess[which]) 
      chopmsg(pb, "A ring called %s", "A ring called %s(%s)", r_guess[which], r_stones[which]);
    else 
      sprintf(pb, "A%s %s ring", vowelstr(r_stones[which]), r_stones[which]);
    break;

#ifdef DEBUG

  case GOLD:
    sprintf(pb, "Gold at %d,%d", obj->pos.y, obj->pos.x);
    break;

  default:
    debug("Picked up someting bizarre %s", unctrl(obj->type));
    sprintf(pb, "Something bizarre %c(%d)", obj->type, obj->type);
    break;

#endif
  }
  if (obj==cur_armor) 
    strcat(pb, " (being worn)");
  if (obj==cur_weapon) 
    strcat(pb, " (weapon in hand)");
  if (obj==cur_ring[LEFT]) 
    strcat(pb, " (on left hand)");
  else if (obj==cur_ring[RIGHT])
    strcat(pb, " (on right hand)");
  if (drop && isupper(prbuf[0])) 
    prbuf[0] = tolower(prbuf[0]);
  else if (!drop && islower(*prbuf)) 
    *prbuf = toupper(*prbuf);
  return prbuf;
}

void chopmsg(char *s, char *shmsg, char *lnmsg, int arg1, int arg2, int arg3)
{
  sprintf(s, lnmsg, arg1, arg2, arg3);
  if (terse || expert) sprintf(s, shmsg, arg1, arg2, arg3);
}

//drop: Put something down
void drop()
{
  byte ch;
  ITEM *nobj, *op;

  ch = get_tile(player.pos.y, player.pos.x);
  if (ch!=FLOOR && ch!=PASSAGE) {msg("there is something there already"); return;}
  if ((op = get_item("drop", 0))==NULL) return;
  if (!can_drop(op)) return;
  //Take it out of the pack
  if (op->count>=2 && op->type!=WEAPON)
  {
    if ((nobj = create_item())==NULL) {msg("%sit appears to be stuck in your pack!", noterse("can't drop it, ")); return;}
    op->count--;
    bcopy(*nobj, *op);
    nobj->count = 1;
    op = nobj;
    if (op->group!=0) inpack++;
  }
  else detach_item(&player.pack, op);
  inpack--;
  //Link it into the level object list
  attach_item(&lvl_obj, op);
  set_tile(player.pos.y, player.pos.x, op->type);
  bcopy(op->pos, player.pos);
  if (op->type==AMULET) amulet = FALSE;
  msg("dropped %s", inv_name(op, TRUE));
}

//can_drop: Do special checks for dropping or unweilding|unwearing|unringing

int can_drop(ITEM *op)
{
  if (op==NULL) return TRUE;
  if (op!=cur_armor && op!=cur_weapon && op!=cur_ring[LEFT] && op!=cur_ring[RIGHT]) return TRUE;
  if (op->flags&ISCURSED) {msg("you can't.  It appears to be cursed"); return FALSE;}
  if (op==cur_weapon) cur_weapon = NULL;
  else if (op==cur_armor) {waste_time(); cur_armor = NULL;}
  else
  {
    int hand;

    if (op!=cur_ring[hand = LEFT]) if (op!=cur_ring[hand = RIGHT])
    {
      debug("Candrop called with funny thing");
      return TRUE;
    }
    cur_ring[hand] = NULL;
    switch (op->which)
    {
    case R_ADDSTR: chg_str(-op->armor_class); break;
    case R_SEEINVIS: unsee(); extinguish(unsee); break;
    }
  }
  return TRUE;
}

//new_thing: Return a new thing
ITEM *new_item()
{
  ITEM *cur;
  int j, k;

  if ((cur = create_item())==NULL) return NULL;
  cur->hit_plus = cur->damage_plus = 0;
  cur->damage = cur->throw_damage = "0d0";
  cur->armor_class = 11;
  cur->count = 1;
  cur->group = 0;
  cur->flags = 0;
  cur->enemy = 0;
  //Decide what kind of object it will be. If we haven't had food for a while, let it be food.
  switch (no_food>3?2:pick_one(things, NUMTHINGS))
  {
  case 0:
    cur->type = POTION;
    cur->which = pick_one(p_magic, MAXPOTIONS);
    break;

  case 1:
    cur->type = SCROLL;
    cur->which = pick_one(s_magic, MAXSCROLLS);
    break;

  case 2:
    no_food = 0;
    cur->type = FOOD;
    if (rnd(10)!=0) cur->which = 0; else cur->which = 1;
    break;

  case 3:
    cur->type = WEAPON;
    cur->which = rnd(MAXWEAPONS);
    init_weapon(cur, cur->which);
    if ((k = rnd(100))<10) {cur->flags |= ISCURSED; cur->hit_plus -= rnd(3)+1;}
    else if (k<15) cur->hit_plus += rnd(3)+1;
    break;

  case 4:
    cur->type = ARMOR;
    for (j = 0, k = rnd(100); j<MAXARMORS; j++) if (k<a_chances[j]) break;
    if (j==MAXARMORS) {debug("Picked a bad armor %d", k); j = 0;}
    cur->which = j;
    cur->armor_class = a_class[j];
    if ((k = rnd(100))<20) {cur->flags |= ISCURSED; cur->armor_class += rnd(3)+1;}
    else if (k<28) cur->armor_class -= rnd(3)+1;
    break;

  case 5:
    cur->type = RING;
    cur->which = pick_one(r_magic, MAXRINGS);
    switch (cur->which)
    {
    case R_ADDSTR: case R_PROTECT: case R_ADDHIT: case R_ADDDAM:
      if ((cur->armor_class = rnd(3))==0) {cur->armor_class = -1; cur->flags |= ISCURSED;}
      break;

    case R_AGGR: case R_TELEPORT:
      cur->flags |= ISCURSED;
      break;
    }
    break;

  case 6:
    cur->type = STICK;
    cur->which = pick_one(ws_magic, MAXSTICKS);
    fix_stick(cur);
    break;

  default:
    debug("Picked a bad kind of object");
    wait_for(' ');
    break;

  }
  return cur;
}

//pick_one: Pick an item out of a list of nitems possible magic items
int pick_one(struct MagicItem *magic, int nitems)
{
  struct MagicItem *end;
  int i;
  struct MagicItem *start;

  start = magic;
  for (end = &magic[nitems], i = rnd(100); magic<end; magic++) if (i<magic->prob) break;
  if (magic==end)
  {

    if (wizard)
    {
      msg("bad pick_one: %d from %d items", i, nitems);
      for (magic = start; magic<end; magic++) msg("%s: %d%%", magic->name, magic->prob);
    }

    magic = start;
  }
  return magic-start;
}

//discovered: list what the player has discovered in this game of a certain type
void discovered()
{
  print_disc(POTION);
  add_line("", " ", 0);
  print_disc(SCROLL);
  add_line("", " ", 0);
  print_disc(RING);
  add_line("", " ", 0);
  print_disc(STICK);
  end_line("");
}

//print_disc: Print what we've discovered of type 'type'
void print_disc(byte type)
{
  bool *know;
  char **guess;
  int i, maxnum, num_found;
  static ITEM obj;
  static short order[MAX(MAXSCROLLS, MAXPOTIONS, MAXRINGS, MAXSTICKS)];

  switch (type)
  {
  case SCROLL:
    maxnum = MAXSCROLLS; 
    know = s_know;
    guess = s_guess;
    break;
  case POTION:
    maxnum = MAXPOTIONS;
    know = p_know;
    guess = p_guess; 
    break;
  case RING:
    maxnum = MAXRINGS;
    know = r_know;
    guess = r_guess;
    break;
  case STICK:
    maxnum = MAXSTICKS;
    know = ws_know;
    guess = ws_guess;
    break;
  }
  set_order(order, maxnum);
  obj.count = 1;
  obj.flags = 0;
  num_found = 0;
  for (i = 0; i<maxnum; i++) if (know[order[i]] || *guess[order[i]])
  {
    obj.type = type;
    obj.which = order[i];
    add_line("", "%s", inv_name(&obj, FALSE));
    num_found++;
  }
  if (num_found==0) add_line("", nothing(type), 0);
}

//set_order: Set up order for list
void set_order(short *order, int numthings)
{
  int i, r, t;

  for (i = 0; i<numthings; i++) order[i] = i;
  for (i = numthings; i>0; i--)
  {
    r = rnd(i);
    t = order[i-1];
    order[i-1] = order[r];
    order[r] = t;
  }
}

//add_line: Add a line to the list of discoveries
int add_line(char *use, char *fmt, char *arg)
{
  int x, y;
  int retchar = ' ';

  if (line_cnt==0) {wdump(0); clear();}
  if (line_cnt>=LINES-1 || fmt==NULL)
  {
    move(LINES-1, 0);
    if (*use) printw("-Select item to %s. Esc to cancel-", use);
    else addstr("-Press space to continue-");
    do retchar = readchar(); while (retchar!=ESCAPE && retchar!=' ' && (!islower(retchar)));
    clear();
    newpage = TRUE;
    line_cnt = 0;
  }
  if (fmt!=NULL && !(line_cnt==0 && *fmt=='\0'))
  {
    move(line_cnt, 0);
    printw(fmt, arg);
    getrc(&x, &y);
    //if the line wrapped but nothing was printed on this line you might as well use it for the next item
    if (y!=0) line_cnt = x+1;
    lastfmt = fmt;
    lastarg = arg;
  }
  return (retchar);
}

//end_line: End the list of lines
int end_line(char *use)
{
  int retchar;

  retchar = add_line(use, 0, 0);
  wrestor(0);
  line_cnt = 0;
  newpage = FALSE;
  return (retchar);
}

//nothing: Set up prbuf so that message for "nothing found" is there
char *nothing(byte type)
{
  char *sp, *tystr;

  sprintf(prbuf, "Haven't discovered anything");
  if (terse) sprintf(prbuf, "Nothing");
  sp = &prbuf[strlen(prbuf)];
  switch (type)
  {
  case POTION: tystr = "potion"; break;
  case SCROLL: tystr = "scroll"; break;
  case RING: tystr = "ring"; break;
  case STICK: tystr = "stick"; break;
  }
  sprintf(sp, " about any %ss", tystr);
  return prbuf;
}
