//Contains functions for dealing with things like potions, scrolls, and other items.
//things.c     1.4 (AI Design) 12/14/84

#include <stdio.h>
#include <ctype.h>
#include <stdarg.h>

#include "rogue.h"
#include "game_state.h"
#include "things.h"
#include "pack.h"
#include "list.h"
#include "sticks.h"
#include "io.h"
#include "misc.h"
#include "daemons.h"
#include "rings.h"
#include "scrolls.h"
#include "potions.h"
#include "weapons.h"
#include "curses.h"
#include "main.h"
#include "armor.h"
#include "daemon.h"
#include "mach_dep.h"
#include "level.h"
#include "thing.h"
#include "food.h"
#include "hero.h"

struct MagicItem things[NUMTHINGS] =
{
  {0, 27 }, //potion
  {0, 30 }, //scroll
  {0, 17 }, //food
  {0,  8 }, //weapon
  {0,  8 }, //armor
  {0,  5 }, //ring
  {0,  5 }  //stick
};

#define MAX(a,b,c,d) (a>b?(a>c?(a>d?a:d):(c>d?c:d)):(b>c?(b>d?b:d):(c>d?c:d)))

static int line_cnt = 0;

//init_things: Initialize the probabilities for types of things
void init_things()
{
  struct MagicItem *mp;
  for (mp = &things[1]; mp<=&things[NUMTHINGS-1]; mp++) 
    mp->prob += (mp-1)->prob;
}

//inv_name: Return the name of something as it would appear in an inventory.
char *inv_name(Item *obj, bool drop)
{
  char *pb = prbuf;
  switch (obj->type)
  {

  case SCROLL:
  case POTION:
  case STICK:
  case RING:
    game->item_class(obj->type).get_inventory_name(obj);
    break;

  case FOOD:
    get_inv_name_food(obj);
    break;

  case WEAPON:
    obj->get_inv_name_weapon();
    break;

  case ARMOR:
    get_inv_name_armor(obj);
    break;

  case AMULET:
    strcpy(pb, "The Amulet of Yendor");
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
  if (obj==get_current_armor()) 
    strcat(pb, " (being worn)");
  if (obj==get_current_weapon()) 
    strcat(pb, " (weapon in hand)");
  if (obj==get_ring(LEFT)) 
    strcat(pb, " (on left hand)");
  else if (obj==get_ring(RIGHT))
    strcat(pb, " (on right hand)");
  if (drop && isupper(prbuf[0])) 
    prbuf[0] = tolower(prbuf[0]);
  else if (!drop && islower(*prbuf)) 
    *prbuf = toupper(*prbuf);
  return prbuf;
}

void chopmsg(char *s, char *shmsg, char *lnmsg, ...)
{
   va_list argptr;
   va_start(argptr, lnmsg);
   vsprintf(s, short_msgs() ? shmsg : lnmsg, argptr);
   va_end(argptr);
}

//drop: Put something down
void drop()
{
  byte ch;
  Item *nobj, *op;

  ch = get_tile(player.pos.y, player.pos.x);
  if (ch!=FLOOR && ch!=PASSAGE) {msg("there is something there already"); return;}
  if ((op = get_item("drop", 0))==NULL) return;
  if (!can_drop(op)) return;
  //Take it out of the pack
  if (op->count >= 2 && op->type != WEAPON)
  {
      op->count--;
      nobj = op->Clone();
      nobj->count = 1;
      op = nobj;
  }
  else 
      player.pack.remove(op);
  //Link it into the level object list
  level_items.push_front(op);
  op->pos = player.pos;
  set_tile(op->pos.y, op->pos.x, op->type);
  msg("dropped %s", inv_name(op, true));
}

//can_drop: Do special checks for dropping or unweilding|unwearing|unringing
bool can_drop(Item *op)
{
  if (op==NULL) return true;
  if (op!=get_current_armor() && op!=get_current_weapon() && op!=get_ring(LEFT) && op!=get_ring(RIGHT)) return true;
  if (op->is_cursed()) {
      msg("you can't.  It appears to be cursed"); 
      return false;
  }
  if (op==get_current_weapon()) set_current_weapon(NULL);
  else if (op==get_current_armor()) {
    waste_time(); 
    set_current_armor(NULL);
  }
  else
  {
    int hand;

    if (op!=get_ring(hand = LEFT)) if (op!=get_ring(hand = RIGHT))
    {
      debug("Candrop called with funny thing");
      return true;
    }
    set_ring(hand, NULL);
    switch (op->which)
    {
    case R_ADDSTR: 
        player.adjust_strength(-op->ring_level);
        break;
    case R_SEEINVIS:
        unsee();
        extinguish(unsee); 
        break;
    }
  }
  return true;
}

//new_thing: Return a new thing
Item *create_item()
{
  //Decide what kind of object it will be. If we haven't had food for a while, let it be food.
  switch (no_food > 3 ? 2 : pick_one(things, NUMTHINGS))
  {
  case 0:
    return create_potion();
    break;

  case 1:
    return create_scroll();
    break;

  case 2:
    return create_food();
    break;

  case 3:
    return create_weapon();
    break;

  case 4:
    return create_armor();
    break;

  case 5:
    return create_ring();
    break;

  case 6:
    return create_stick();
    break;

  default:
    debug("Picked a bad kind of object");
    wait_for(' ');
    break;
  }
  return 0;
}

//todo: remove this wrapper
int pick_one(std::vector<MagicItem> magic)
{
    MagicItem* a = new MagicItem[magic.size()];
    for (size_t i = 0; i < magic.size(); ++i)
        a[i] = magic[i];
    int r = pick_one(a, magic.size());
    delete[] a;
    return r;
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
    debug("bad pick_one: %d from %d items", i, nitems);
    for (magic = start; magic<end; magic++)
      debug("%s: %d%%", magic->name, magic->prob);

    magic = start;
  }
  return magic-start;
}

void debug_screen()
{
  add_line(0, "hey", 0);
  add_line("", "mike", 0);
  end_line("");
}

//discovered: list what the player has discovered in this game of a certain type
void discovered()
{
  if (game->hero().is_wizard())
    return debug_screen();

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
    int i, maxnum, num_found;
    static short order[MAX(MAXSCROLLS, MAXPOTIONS, MAXRINGS, MAXSTICKS)];

    ItemClass& item_class = game->item_class(type);
    maxnum = item_class.get_max_items();

    set_order(order, maxnum);
    num_found = 0;
    for (i = 0; i < maxnum; i++) {
        if (item_class.is_discovered(order[i]) || !item_class.get_guess(order[i]).empty())
        {
            std::string line = game->item_class(type).get_inventory_name(order[i]);
            add_line("", "%s", line.c_str());
            num_found++;
        }

    }
    if (num_found == 0)
        add_line("", nothing(type), 0);
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
int add_line(const char *use, const char *fmt, const char *arg)
{
  int x, y;
  int retchar = ' ';

  if (line_cnt==0) {wdump(); clear();}
  if (line_cnt>=LINES-1 || fmt==NULL)
  {
    move(LINES-1, 0);
    if (*use) printw("-Select item to %s. Esc to cancel-", use);
    else addstr("-Press space to continue-");
    do retchar = readchar(); while (retchar!=ESCAPE && retchar!=' ' && (!islower(retchar)));
    clear();
    line_cnt = 0;
  }
  if (fmt!=NULL && !(line_cnt==0 && *fmt=='\0'))
  {
    move(line_cnt, 0);
    printw(fmt, arg);
    getrc(&x, &y);
    //if the line wrapped but nothing was printed on this line you might as well use it for the next item
    if (y!=0) line_cnt = x+1;
  }
  return (retchar);
}

//end_line: End the list of lines
int end_line(char *use)
{
  int retchar;

  retchar = add_line(use, 0, 0);
  wrestor();
  line_cnt = 0;
  return (retchar);
}

//nothing: Set up prbuf so that message for "nothing found" is there
char *nothing(byte type)
{
  char *sp, *tystr;

  sprintf(prbuf, "Haven't discovered anything");
  if (in_small_screen_mode()) sprintf(prbuf, "Nothing");
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
