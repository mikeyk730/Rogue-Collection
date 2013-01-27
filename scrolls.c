//Read a scroll and let it happen
//scrolls.c   1.4 (AI Design) 12/14/84

#include "rogue.h"
#include "scrolls.h"
#include "monsters.h"
#include "pack.h"
#include "list.h"
#include "curses.h"
#include "io.h"
#include "main.h"
#include "misc.h"
#include "wizard.h"
#include "slime.h"
#include "level.h"
#include "thing.h"
#include "weapons.h"

//Scroll types
#define S_CONFUSE   0
#define S_MAP       1
#define S_HOLD      2
#define S_SLEEP     3
#define S_ARMOR     4
#define S_IDENT     5
#define S_SCARE     6
#define S_GFIND     7
#define S_TELEP     8
#define S_ENCH      9
#define S_CREATE    10
#define S_REMOVE    11
#define S_AGGR      12
#define S_NOP       13
#define S_VORPAL    14

char *laugh = "you hear maniacal laughter%s.";
char *in_dist = " in the distance";

void read_monster_confusion()
{
  //Scroll of monster confusion.  Give him that power.
  player.flags |= CANHUH;
  msg("your hands begin to glow red");
}

void read_magic_mapping()
{
  //Scroll of magic mapping.
  int x, y;
  byte ch;
  AGENT* monster;
  
  s_know[S_MAP] = TRUE;
  msg("oh, now this scroll has a map on it");
  //Take all the things we want to keep hidden out of the window
  for (y = 1; y<maxrow; y++) for (x = 0; x<COLS; x++)
  {
    switch (ch = get_tile(y, x))
    {
    case VWALL: case HWALL: case ULWALL: case URWALL: case LLWALL: case LRWALL:
      if (!(get_flags(y, x)&F_REAL)) {
        ch = DOOR; 
        set_tile(y, x, DOOR);
        unset_flag(y, x, F_REAL);
      }
    case DOOR: case PASSAGE: case STAIRS:
      if ((monster = monster_at(y, x)) != NULL) 
        if (monster->oldch == ' ')
          monster->oldch = ch;
      break;
    default: ch = ' ';
    }
    if (ch==DOOR)
    {
      move(y, x);
      if (curch()!=DOOR) 
        standout();
    }
    if (ch!=' ') 
      mvaddch(y, x, ch);
    standend();
  }
}

void read_hold_monster()
{
  //Hold monster scroll.  Stop all monsters within two spaces from chasing after the hero.
  int x, y;
  AGENT* monster;

  for (x = player.pos.x-3; x<=player.pos.x+3; x++) {
    if (x>=0 && x<COLS) {
      for (y = player.pos.y-3; y<=player.pos.y+3; y++) {
        if ((y>0 && y<maxrow) && ((monster = monster_at(y, x)) != NULL))
        {
          monster->flags &= ~ISRUN;
          monster->flags |= ISHELD;
        }
      }
    }
  }
}

void read_sleep()
{
  //Scroll which makes you fall asleep
  s_know[S_SLEEP] = TRUE;
  no_command += rnd(SLEEP_TIME)+4;
  player.flags &= ~ISRUN;
  msg("you fall asleep");
}

void read_enchant_armor()
{
  if (cur_armor!=NULL)
  {
    cur_armor->armor_class--;
    cur_armor->flags &= ~ISCURSED;
    ifterse("your armor glows faintly", "your armor glows faintly for a moment");
  }
}

void read_identify()
{
  //Identify, let the rogue figure something out
  s_know[S_IDENT] = TRUE;
  msg("this scroll is an identify scroll");
  if (strcmp(s_menu, "on") == 0) 
    more(" More ");
  whatis(TRUE);
}

void read_scare_monster()
{
  //Reading it is a mistake and produces laughter at the poor rogue's boo boo.
  msg(laugh, terse || expert ? "" : in_dist);
}

void read_food_detection()
{
  //Scroll of food detection
  ITEM* item;
  byte discover = FALSE;

  for (item = lvl_obj; item != NULL; item = next(item))
  {
    if (item->type==FOOD)
    {
      discover = TRUE;
      standout();
      mvaddch(item->pos.y, item->pos.x, FOOD);
      standend();
    }
    //as a bonus this will detect amulets as well
    else if (item->type==AMULET)
    {
      discover = TRUE;
      standout();
      mvaddch(item->pos.y, item->pos.x, AMULET);
      standend();
    }
  }
  if (discover) {
    s_know[S_GFIND] = TRUE; 
    msg("your nose tingles as you sense food");
  }
  else 
    ifterse("you hear a growling noise close by", "you hear a growling noise very close to you");
}

void read_teleportation()
{
  //Scroll of teleportation: Make him disappear and reappear
  struct Room *cur_room;
  cur_room = player.room;
  teleport();
  if (cur_room != player.room) 
    s_know[S_TELEP] = TRUE;
}

void read_enchant_weapon()
{
  if (cur_weapon == NULL || cur_weapon->type != WEAPON)
    msg("you feel a strange sense of loss");
  else
  {
    cur_weapon->flags &= ~ISCURSED;
    if (rnd(2)==0) cur_weapon->hit_plus++;
    else cur_weapon->damage_plus++;
    ifterse("your %s glows blue", "your %s glows blue for a moment", get_weapon_name(cur_weapon->which));
  }
}

void read_create_monster()
{
  AGENT* monster;
  Coord position;

  if (plop_monster(player.pos.y, player.pos.x, &position) && (monster = create_agent()) != NULL)
    new_monster(monster, randmonster(FALSE), &position);
  else 
    ifterse("you hear a faint cry of anguish", "you hear a faint cry of anguish in the distance");
}

void read_remove_curse()
{
  if (cur_armor) 
    cur_armor->flags &= ~ISCURSED;
  if (cur_weapon) 
    cur_weapon->flags &= ~ISCURSED;
  if (cur_ring[LEFT]) 
    cur_ring[LEFT]->flags &= ~ISCURSED;
  if (cur_ring[RIGHT])
    cur_ring[RIGHT]->flags &= ~ISCURSED;

  ifterse("somebody is watching over you", "you feel as if somebody is watching over you");
}

void read_aggravate_monsters()
{
  //This scroll aggravates all the monsters on the current level and sets them running towards the hero
  aggravate();
  ifterse("you hear a humming noise", "you hear a high pitched humming noise");
}

void read_blank_paper()
{
  msg("this scroll seems to be blank");
}

void read_vorpalize_weapon()
{
  //Extra Vorpal Enchant Weapon
  //    Give weapon +1,+1
  //    Is extremely vorpal against one certain type of monster
  //    Against this type (o_enemy) the weapon gets:
  // +4,+4
  // The ability to zap one such monster into oblivion
  //
  //    Some of these are cursed and if the rogue misses her saving
  //    throw she will be forced to attack monsters of this type
  //    whenever she sees one (not yet implemented)
  //
  //If he doesn't have a weapon I get to chortle again!
  if (cur_weapon==NULL || cur_weapon->type!=WEAPON) 
    msg(laugh, terse || expert?"":in_dist);
  else
  {
    //You aren't allowed to doubly vorpalize a weapon.
    if (cur_weapon->enemy!=0)
    {
      msg("your %s vanishes in a puff of smoke", get_weapon_name(cur_weapon->which));
      detach_item(&player.pack, cur_weapon);
      discard_item(cur_weapon);
      cur_weapon = NULL;
    }
    else
    {
      cur_weapon->enemy = pick_monster();
      cur_weapon->hit_plus++;
      cur_weapon->damage_plus++;
      cur_weapon->charges = 1;
      msg(flash, get_weapon_name(cur_weapon->which), terse || expert?"":intense);
    }
  }
}

void(*scroll_functions[MAXSCROLLS])() =
{
  read_monster_confusion,
  read_magic_mapping,
  read_hold_monster,
  read_sleep,
  read_enchant_armor,
  read_identify,
  read_scare_monster,
  read_food_detection,
  read_teleportation,
  read_enchant_weapon,
  read_create_monster,
  read_remove_curse,
  read_aggravate_monsters,
  read_blank_paper,
  read_vorpalize_weapon
};

//read_scroll: Read a scroll from the pack and do the appropriate thing
void read_scroll()
{
  ITEM *scroll;

  scroll = get_item("read", SCROLL);
  if (scroll==NULL) return;
  if (scroll->type!=SCROLL) {msg("there is nothing on it to read"); return;}
  ifterse("the scroll vanishes", "as you read the scroll, it vanishes");
  if (scroll==cur_weapon) cur_weapon = NULL;

  //Call the function for this scroll
  if (scroll->which >= 0 && scroll->which < MAXSCROLLS)
    scroll_functions[scroll->which]();
  else {
    msg("what a puzzling scroll!");
    return;
  }

  look(TRUE); //put the result of the scroll on the screen
  status();
  call_it(s_know[scroll->which], &s_guess[scroll->which]);

  //Get rid of the thing
  if (scroll->count > 1)
    scroll->count--;
  else {
    detach_item(&player.pack, scroll); 
    discard_item(scroll);
  }
}

int is_scare_monster_scroll(ITEM* item)
{
  return item && item->type == SCROLL && 
    item->which == S_SCARE;
}

int is_bad_scroll(ITEM* item)
{
  return item && item->type == SCROLL &&
    (item->which == S_SLEEP || item->which == S_CREATE || item->which == S_AGGR);
}
