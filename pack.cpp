//Routines to deal with the pack
//pack.c      1.4 (A.I. Design)       12/14/84

#include <stdio.h>

#include "rogue.h"
#include "game_state.h"
#include "pack.h"
#include "io.h"
#include "misc.h"
#include "curses.h"
#include "list.h"
#include "things.h"
#include "mach_dep.h"
#include "level.h"
#include "thing.h"
#include "scrolls.h"
#include "hero.h"
#include "room.h"

static bool s_had_amulet = false;

Item *cur_armor;   //What a well dresssed rogue wears
Item *cur_weapon;  //Which weapon he is wielding
Item *cur_ring[2]; //Which rings are being worn

Item* get_ring(int hand)
{
  return cur_ring[hand];
}

void set_ring(int hand, Item* item)
{
  cur_ring[hand] = item;
}

Item* get_current_weapon()
{
  return cur_weapon;
}

void set_current_weapon(Item* item)
{
  cur_weapon = item;
}

Item* get_current_armor()
{
  return cur_armor;
}

void set_current_armor(Item* item)
{
  cur_armor = item;
}

int does_item_group(int type) {
  return (type==POTION || type==SCROLL || type==FOOD || type==GOLD);
}

Item *pack_obj(byte ch, byte *chp)
{
  byte och = 'a';

  for (auto it = player.pack.begin(); it != player.pack.end(); ++it, och++){ 
      if (ch == och)
          return *it; 
  }
  *chp = och;
  return NULL;
}

int get_pack_size()
{
  int count = 0;
  for (auto it = player.pack.begin(); it != player.pack.end(); ++it){
      Item* item = *it;
      count += item->group ? 1 : item->count;
  }
  return count;
}

//add_pack: Pick up an object and add it to the pack.  If the argument is non-null use it as the linked_list pointer instead of getting it off the ground.
void add_pack(Item *obj, bool silent)
{
  Agent *monster;
  bool from_floor;
  byte floor;

  if (obj==NULL)
  {
    from_floor = true;
    if ((obj = find_obj(player.pos.y, player.pos.x))==NULL) return;
    floor = (player.room->is_gone())?PASSAGE:FLOOR;
  }
  else from_floor = false;
  //Link it into the pack.  Search the pack for a object of similar type
  //if there isn't one, stuff it at the beginning, if there is, look for one
  //that is exactly the same and just increment the count if there is.
  //Food is always put at the beginning for ease of access, but it
  //is not ordered so that you can't tell good food from bad.  First check
  //to see if there is something in the same group and if there is then
  //increment the count.

  if (obj->group)
  {
    for (auto it = player.pack.begin(); it != player.pack.end(); ++it){
      Item* op = *it;
      if (op->group==obj->group)
      {
        //Put it in the pack and notify the user
        op->count += obj->count;
        if (from_floor) {
          level_items.remove(obj);
          mvaddch(player.pos.y, player.pos.x, floor);
          set_tile(player.pos.y, player.pos.x, floor);
        }
        delete(obj);
        obj = op;
        goto picked_up;
      }
    }
  }
  //Check if there is room
  if (get_pack_size() >= MAXPACK-1) {
      msg("you can't carry anything else");
      return;
  }
  //Check for and deal with scare monster scrolls
  if (is_scare_monster_scroll(obj)) {
      if (obj->is_found())
      {
          level_items.remove(obj); //todo: delete?
          mvaddch(player.pos.y, player.pos.x, floor);
          set_tile(player.pos.y, player.pos.x, floor);
          msg("the scroll turns to dust%s.", noterse(" as you pick it up"));
          return;
      }
      else obj->set_found();
  }
  if (from_floor) {
    level_items.remove(obj);
    mvaddch(player.pos.y, player.pos.x, floor); 
    set_tile(player.pos.y, player.pos.x, floor);
  }
  
  //todo: fuck this code is infuriating
  auto it = player.pack.begin();

  //Search for an object of the same type
  bool found_type = false;
  for (; it != player.pack.end(); ++it){
      if ((*it)->type == obj->type){
          found_type = true;
          break;
      }
  }
  //Put it at the end of the pack since it is a new type
  if (!found_type){
      (obj->type == FOOD) ? player.pack.push_front(obj) :
          player.pack.push_back(obj);
      goto picked_up;
  }
  //Search for an object which is exactly the same
  bool exact = false;
  for (; it != player.pack.end(); ++it){
      if ((*it)->type != obj->type)
          break;
      if ((*it)->which == obj->which) {
          exact = true;
          break;
      }
  }
  //If we found an exact match.  If it is a potion, food, or a scroll, increase the count, otherwise put it with its clones.
  if (exact && does_item_group(obj->type))
  {
      (*it)->count++;
      delete(obj);
      obj = (*it);
      goto picked_up;
  }

  player.pack.insert(it, obj);

picked_up:
  //If this was the object of something's desire, that monster will get mad and run at the hero
  for (auto it = level_monsters.begin(); it != level_monsters.end(); ++it){
      monster = *it;
      if (monster->dest && (monster->dest->x == obj->pos.x) && (monster->dest->y == obj->pos.y))
          monster->dest = &player.pos;
  }
  if (obj->type==AMULET) { 
      s_had_amulet = true;
  }
  //Notify the user
  if (!silent) 
      msg("%s%s (%c)", noterse("you now have "), inv_name(obj, true), pack_char(obj));
}

//inventory: List what is in the pack
int inventory(std::list<Item *>& list, int type, char *lstr)
{
  byte ch = 'a';
  int n_objs;
  char inv_temp[MAXSTR];

  n_objs = 0;
  for (auto it = player.pack.begin(); it != player.pack.end(); ++it, ch++)
  {
    Item* item = *it;
    //Don't print this one if: the type doesn't match the type we were passed AND it isn't a callable type AND it isn't a zappable weapon
    if (type && type!=item->type && 
        !(type==CALLABLE && (item->type==SCROLL || item->type==POTION || item->type==RING || item->type==STICK)) &&
        !(type==WEAPON && item->type==POTION) &&
        !(type==STICK && item->is_vorpalized() && item->charges)) //todo: does this work?
        continue;
    n_objs++;
    sprintf(inv_temp, "%c) %%s", ch);
    add_line(lstr, inv_temp, inv_name(item, false));
  }
  if (n_objs==0)
  {
    msg(type == 0 ? "you are empty handed" : "you don't have anything appropriate");
    return false;
  }
  return (end_line(lstr));
}

//pick_up: Add something to characters pack.
void pick_up(byte ch)
{
  Item *obj;

  switch (ch)
  {
  case GOLD:
    if ((obj = find_obj(player.pos.y, player.pos.x))==NULL)
        return;
    money(obj->gold_value);
    level_items.remove(obj);
    delete obj;
    player.room->goldval = 0;
    break;
  default:
  case ARMOR: case POTION: case FOOD: case WEAPON: case SCROLL: case AMULET: case RING: case STICK:
    add_pack(NULL, false);
    break;
  }
}

//get_item: Pick something out of a pack for a purpose
Item *get_item(char *purpose, int type)
{
  Item *obj;
  byte ch;
  byte och;
  static byte lch;
  static Item *wasthing = NULL;
  byte gi_state; //get item sub state
  int once_only = false;

  if ("on" == game->get_environment("menu"))
      once_only = true;
  gi_state = again;
  if (player.pack.empty())
      msg("you aren't carrying anything");
  else
  {
    ch = lch;
    for (;;)
    {
      //if we are doing something AGAIN, and the pack hasn't changed then don't ask just give him the same thing he got on the last command.
      if (gi_state && wasthing==pack_obj(ch, &och)) 
          goto skip;
      if (once_only) {
          ch = '*'; 
          goto skip;
      }
      if (!short_msgs()) 
          addmsg("which object do you want to ");
      msg("%s? (* for list): ", purpose);
      //ignore any alt characters that may be typed
      ch = readchar();
skip:
      mpos = 0;
      gi_state = false;
      once_only = false;
      if (ch=='*')
      {
        if ((ch = inventory(player.pack, type, purpose))==0) {
            counts_as_turn = false; 
            return NULL;
        }
        if (ch==' ') continue;
        lch = ch;
      }
      //Give the poor player a chance to abort the command
      if (ch==ESCAPE) {
          counts_as_turn = false; 
          msg(""); 
          return NULL;
      }
      if ((obj = pack_obj(ch, &och))==NULL)
      {
        ifterse("range is 'a' to '%c'","please specify a letter between 'a' and '%c'", och-1);
        continue;
      }
      else
      {
        //If you find an object reset flag because you really don't know if the object he is getting is going to change the pack.  If he detaches the thing from the pack later this flag will get set.
        if (strcmp(purpose, "identify")) {
            lch = ch;
            wasthing = obj;
        }
        return obj;
      }
    }
  }
  return NULL;
}

//pack_char: Return which character would address a pack object
int pack_char(Item *obj)
{
    byte c = 'a';
    for (auto it = player.pack.begin(); it != player.pack.end(); ++it){
        if (*it == obj)
            return c;
        else
            c++;
    }
    return '?';
}

//money: Add or subtract gold from the pack
void money(int value)
{
  byte floor;

  floor = (player.room->is_gone()) ? PASSAGE : FLOOR;
  game->hero().adjust_purse(value);
  mvaddch(player.pos.y, player.pos.x, floor);
  set_tile(player.pos.y, player.pos.x, floor);
  if (value>0) msg("you found %d gold pieces", value);
}

bool has_amulet()
{
    for (auto it = player.pack.begin(); it != player.pack.end(); ++it){
        Item* item = *it;
        if (item->type == AMULET)
            return true;
    }

    return false;
}

//true if player ever had amulet
bool had_amulet()
{
  return s_had_amulet;
}