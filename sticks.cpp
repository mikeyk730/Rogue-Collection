//Functions to implement the various sticks one might find while wandering around the dungeon.
//@(#)sticks.c1.2 (AI Design) 2/12/84

#include <stdio.h>

#include "rogue.h"
#include "sticks.h"
#include "monsters.h"
#include "pack.h"
#include "io.h"
#include "curses.h"
#include "main.h"
#include "chase.h"
#include "fight.h"
#include "new_leve.h"
#include "rooms.h"
#include "misc.h"
#include "weapons.h"
#include "rip.h"
#include "thing.h"
#include "level.h"
#include "list.h"
#include "mach_dep.h"
#include "things.h"
#include "hero.h"

bool ws_know[MAXSTICKS];    //Does he know what a stick does
char *ws_guess[MAXSTICKS];         //Players guess at what wand is
const char *ws_made[MAXSTICKS]; //What sticks are made of
const char *ws_type[MAXSTICKS]; //Is it a wand or a staff

static char *wood[] =
{
  "avocado wood",
  "balsa",
  "bamboo",
  "banyan",
  "birch",
  "cedar",
  "cherry",
  "cinnibar",
  "cypress",
  "dogwood",
  "driftwood",
  "ebony",
  "elm",
  "eucalyptus",
  "fall",
  "hemlock",
  "holly",
  "ironwood",
  "kukui wood",
  "mahogany",
  "manzanita",
  "maple",
  "oaken",
  "persimmon wood",
  "pecan",
  "pine",
  "poplar",
  "redwood",
  "rosewood",
  "spruce",
  "teak",
  "walnut",
  "zebrawood"
};

#define NWOOD (sizeof(wood)/sizeof(char *))

static char *metal[] =
{
  "aluminum",
  "beryllium",
  "bone",
  "brass",
  "bronze",
  "copper",
  "electrum",
  "gold",
  "iron",
  "lead",
  "magnesium",
  "mercury",
  "nickel",
  "pewter",
  "platinum",
  "steel",
  "silver",
  "silicon",
  "tin",
  "titanium",
  "tungsten",
  "zinc"
};

#define NMETAL (sizeof(metal)/sizeof(char *))

struct MagicItem ws_magic[MAXSTICKS] =
{
  {"light",          12, 250},
  {"striking",        9,  75},
  {"lightning",       3, 330},
  {"fire",            3, 330},
  {"cold",            3, 330},
  {"polymorph",      15, 310},
  {"magic missile",  10, 170},
  {"haste monster",   9,   5},
  {"slow monster",   11, 350},
  {"drain life",      9, 300},
  {"nothing",         1,   5},
  {"teleport away",   5, 340},
  {"teleport to",     5,  50},
  {"cancellation",    5, 280}
};

int does_know_stick(int type)
{
  return ws_know[type];
}

void discover_stick(int type)
{
  ws_know[type] = true;
}

const char* get_stick_guess(int type)
{
  return ws_guess[type];
}

void set_stick_guess(int type, const char* value)
{
  strcpy(ws_guess[type], value);
}

int get_stick_value(int type)
{
  return ws_magic[type].worth;
}

const char* get_stick_name(int type)
{
  return ws_magic[type].name;
}

void init_new_stick(ITEM* stick)
{
  stick->type = STICK;
  stick->which = pick_one(ws_magic, MAXSTICKS);
  fix_stick(stick);
}

//init_materials: Initialize the construction materials for wands and staffs
void init_materials()
{
  int i, j;
  char *str;
  bool metused[NMETAL], woodused[NWOOD];

  for (i = 0; i<NWOOD; i++) 
      woodused[i] = false;
  for (i = 0; i<NMETAL; i++) 
      metused[i] = false;
  for (i = 0; i<MAXSTICKS; i++)
  {
    for (;;) if (rnd(2)==0)
    {
      j = rnd(NMETAL);
      if (!metused[j]) {
          ws_type[i] = "wand"; 
          str = metal[j];
          metused[j] = true;
          break;
      }
    }
    else
    {
      j = rnd(NWOOD);
      if (!woodused[j]) {
          ws_type[i] = "staff"; 
          str = wood[j]; 
          woodused[j] = true; 
          break;
      }
    }
    ws_made[i] = str;
    ws_know[i] = false;
    ws_guess[i] = (char *)&_guesses[iguess++];
    if (i>0) ws_magic[i].prob += ws_magic[i-1].prob;
  }
}

const char* get_material(int type)
{
  return ws_made[type];
}

const char* get_stick_type(int type)
{
  return ws_type[type];
}

void zap_light()
{
  //Ready Kilowatt wand.  Light up the room
  if (player.is_blind()) msg("you feel a warm glow around you");
  else
  {
    ws_know[WS_LIGHT] = true;
    if (player.room->is_gone()) msg("the corridor glows and then fades");
    else msg("the room is lit by a shimmering blue light");
  }
  if (!player.room->is_gone())
  {
    player.room->set_dark(false);
    //Light the room and put the player back up
    enter_room(&player.pos);
  }
}

void zap_striking(ITEM* obj)
{
  AGENT* monster;
  Coord coord = delta;

  coord.y += player.pos.y;
  coord.x += player.pos.x;
  if ((monster = monster_at(coord.y, coord.x))!=NULL)
  {
    if (rnd(20)==0) {obj->damage = "3d8"; obj->damage_plus = 9;}
    else {obj->damage = "2d8"; obj->damage_plus = 4;}
    fight(&coord, obj, false);
  }
}

void zap_bolt(int which, const char* name)
{
  fire_bolt(&player.pos, &delta, name);
  ws_know[which] = true;
}

void zap_vorpalized_weapon(ITEM* weapon, AGENT* monster)
{
  if (monster->type == weapon->enemy)
  {
    msg("the %s vanishes in a puff of smoke", monster->get_monster_name());
    killed(monster, false);
  }
  else 
    msg("you hear a maniacal chuckle in the distance.");
}

void zap_polymorph(AGENT* monster, int y, int x)
{
  ITEM *pack;
  byte ch, old_type;
  Coord coord = delta;

  pack = monster->pack;
  ch = monster->oldch;
  old_type = monster->type;
  detach_agent(&mlist, monster);
  if (can_see_monst(monster)) 
    mvaddch(y, x, get_tile(y, x));

  coord.y = y;
  coord.x = x;
  new_monster(monster, rnd(26)+'A', &coord, get_level());
  monster->oldch = ch;
  monster->pack = pack;
  if (can_see_monst(monster)) 
    mvaddch(y, x, monster->type);

  ws_know[WS_POLYMORPH] |= (monster->type != old_type);
}

void zap_cancellation(AGENT* monster)
{
  monster->flags |= IS_CANC;
  monster->flags &= ~(IS_INVIS|CAN_HUH);
  monster->disguise = monster->type;
}

void zap_teleport(AGENT* monster, int y, int x, int which)
{
  Coord new_pos;

  if (can_see_monst(monster)) 
    mvaddch(y, x, monster->oldch);

  if (which==WS_TELAWAY)
  {
    monster->oldch = '@';
    find_empty_location(&new_pos, true);
    monster->pos = new_pos;
  }
  else { //it MUST BE at WS_TELTO
    monster->pos.y = player.pos.y+delta.y; 
    monster->pos.x = player.pos.x+delta.x;
  } 

  if (monster->can_hold()) 
    player.flags &= ~IS_HELD;
}

void zap_generic(ITEM* wand, int which)
{
  int x, y;
  AGENT* monster;

  y = player.pos.y;
  x = player.pos.x;
  while (step_ok(get_tile_or_monster(y, x))) {
      y += delta.y; 
      x += delta.x;
  }
  if ((monster = monster_at(y, x))!=NULL)
  {
    if (monster->can_hold())
        player.flags &= ~IS_HELD;
    if (which==MAXSTICKS)
    {
      zap_vorpalized_weapon(wand, monster);
    }
    else if (which==WS_POLYMORPH)
    {
      zap_polymorph(monster, y, x);
    }
    else if (which==WS_CANCEL)
    {
      zap_cancellation(monster);
    }
    else
    {
      zap_teleport(monster, y, x, which);      
    }
    monster->dest = &player.pos;
    monster->flags |= IS_RUN;
  }
}

void zap_magic_missile()
{
  AGENT* monster;
  ITEM bolt;

  ws_know[WS_MISSILE] = true;
  bolt.type = '*';
  bolt.throw_damage = "1d8";
  bolt.hit_plus = 1000;
  bolt.damage_plus = 1;
  bolt.flags = IS_MISL;
  if (get_current_weapon()!=NULL) 
      bolt.launcher = get_current_weapon()->which;
  do_motion(&bolt, delta.y, delta.x);
  if ((monster = monster_at(bolt.pos.y, bolt.pos.x))!=NULL && !save_throw(VS_MAGIC, monster))
      hit_monster(bolt.pos.y, bolt.pos.x, &bolt);
  else
      msg("the missile vanishes with a puff of smoke");
}

void zap_speed_monster(int which)
{
  int x, y;
  AGENT* monster;

  y = player.pos.y;
  x = player.pos.x;
  while (step_ok(get_tile_or_monster(y, x))) {
    y += delta.y; 
    x += delta.x;
  }
  if (monster = monster_at(y, x))
  {
    if (which==WS_HASTE_M)
    {
      if (monster->is_slow())
          monster->flags &= ~IS_SLOW;
      else 
          monster->flags |= IS_HASTE;
    }
    else
    {
      if (monster->is_fast()) 
          monster->flags &= ~IS_HASTE;
      else 
          monster->flags |= IS_SLOW;
      monster->turn = true;
    }
    start_run(monster);
  }
}

int zap_drain_life()
{
  //Take away 1/2 of hero's hit points, then take it away evenly from the monsters in the room (or next to hero if he is in a passage)
  if (player.stats.get_hp() < 2) {
    msg("you are too weak to use it"); 
    return false;
  }  
  drain();
  return true;
}

//fix_stick: Set up a new stick
void fix_stick(ITEM *cur)
{
  if (strcmp(ws_type[cur->which], "staff")==0) 
      cur->damage = "2d3";
  else
      cur->damage = "1d1";
  cur->throw_damage = "1d1";
  cur->charges = 3+rnd(5);
  switch (cur->which)
  {
  case WS_HIT: 
      cur->hit_plus = 100;
      cur->damage_plus = 3;
      cur->damage = "1d8"; 
      break;
  case WS_LIGHT: 
      cur->charges = 10+rnd(10); 
      break;
  }
}

//do_zap: Perform a zap with a wand
void do_zap()
{
  ITEM *obj;
  int which_one;

  if ((obj = get_item("zap with", STICK)) == NULL) 
    return;
  which_one = obj->which;
  
  if (obj->type != STICK)
  {
    if (obj->enemy && obj->charges) 
      which_one = MAXSTICKS;
    else {
      msg("you can't zap with that!"); 
      counts_as_turn = false; 
      return;
    }
  }
  
  if (obj->charges==0) {
    msg("nothing happens"); 
    return;
  }

  switch (which_one)
  {
  case WS_LIGHT: 
    zap_light();
    break;

  case WS_DRAIN: 
    if (!zap_drain_life()) 
        return;
    break;

  case WS_POLYMORPH: case WS_TELAWAY: case WS_TELTO: case WS_CANCEL: case MAXSTICKS: //Special case for vorpal weapon
    zap_generic(obj, which_one);
    break;
 
  case WS_MISSILE:
    zap_magic_missile();
    break;

  case WS_HIT:
    zap_striking(obj);
    break;

  case WS_HASTE_M: case WS_SLOW_M:
    zap_speed_monster(which_one);
    break;

  case WS_ELECT: 
    zap_bolt(which_one, "bolt");
    break;

  case WS_FIRE: 
    zap_bolt(which_one, "flame");
    break;

  case WS_COLD:
    zap_bolt(which_one, "ice");
    break;

  default: 
    debug("what a bizarre schtick!"); 
    break;
  }

  if (--obj->charges < 0) 
    obj->charges = 0;
}

//drain: Do drain hit points from player schtick
void drain()
{
  AGENT *monster;
  int cnt;
  struct Room *room;
  AGENT **dp;
  bool inpass;
  AGENT *drainee[40];

  //First count how many things we need to spread the hit points among
  cnt = 0;
  if (get_tile(player.pos.y, player.pos.x)==DOOR) room = &passages[get_flags(player.pos.y, player.pos.x)&F_PNUM];
  else room = NULL;
  inpass = (player.room->is_gone()) != 0;
  dp = drainee;
  for (monster = mlist; monster!=NULL; monster = next(monster)){
    if (monster->room==player.room || monster->room==room || (inpass && get_tile(monster->pos.y, monster->pos.x)==DOOR && &passages[get_flags(monster->pos.y, monster->pos.x)&F_PNUM]==player.room)) {
      *dp++ = monster;
    }
  }
  if ((cnt = dp-drainee)==0) {
    msg("you have a tingling feeling"); 
    return;
  }
  *dp = NULL;
  cnt = player.stats.drain_hp() / cnt + 1;
  //Now zot all of the monsters
  for (dp = drainee; *dp; dp++)
  {
    monster = *dp;
    if (!monster->stats.decrease_hp(cnt, true))
        killed(monster, can_see_monst(monster));
    else start_run(monster);
  }
}

//fire_bolt: Fire a bolt in a given direction from a specific starting place
bool fire_bolt(Coord *start, Coord *dir, const char *name)
{
  byte dirch, ch;
  AGENT *monster;
  bool hit_hero, used, changed;
  int i, j;
  Coord pos;
  struct {Coord s_pos; byte s_under;} spotpos[BOLT_LENGTH*2];
  ITEM bolt;
  bool is_frost;

  is_frost = (strcmp(name, "frost")==0);
  bolt.type = WEAPON;
  bolt.which = FLAME;
  bolt.damage = bolt.throw_damage = "6d6";
  bolt.hit_plus = 30;
  bolt.damage_plus = 0;
  //TODO:weapon_names[FLAME] = name;
  switch (dir->y+dir->x)
  {
  case 0: dirch = '/'; break;
  case 1: case -1: dirch = (dir->y==0?'-':'|'); break;
  case 2: case -2: dirch = '\\'; break;
  }
  pos = *start;
  hit_hero = (start!=&player.pos);
  used = false;
  changed = false;
  for (i = 0; i<BOLT_LENGTH && !used; i++)
  {
    pos.y += dir->y;
    pos.x += dir->x;
    ch = get_tile_or_monster(pos.y, pos.x);
    spotpos[i].s_pos = pos;
    if ((spotpos[i].s_under = mvinch(pos.y, pos.x))==dirch) spotpos[i].s_under = 0;
    switch (ch)
    {
    case DOOR: case HWALL: case VWALL: case ULWALL: case URWALL: case LLWALL: case LRWALL: case ' ':
      if (!changed) hit_hero = !hit_hero;
      changed = false;
      dir->y = -dir->y;
      dir->x = -dir->x;
      i--;
      msg("the %s bounces", name);
      break;

    default:
      if (!hit_hero && (monster = monster_at(pos.y, pos.x))!=NULL)
      {
        hit_hero = true;
        changed = !changed;
        if (monster->oldch!='@') monster->oldch = get_tile(pos.y, pos.x);
        if (!save_throw(VS_MAGIC, monster) || is_frost)
        {
          bolt.pos = pos;
          used = true;
          if (monster->immune_to_fire() && strcmp(name, "flame")==0) 
              msg("the flame bounces off the %s", monster->get_monster_name());
          else
          {
            hit_monster(pos.y, pos.x, &bolt);
            if (mvinch(pos.y, pos.x)!=dirch) spotpos[i].s_under = mvinch(pos.y, pos.x);
            return false; //zapping monster may have killed self, not safe to go on
          }
        }
        else if (!monster->is_disguised())
        {
          if (start==&player.pos) 
              start_run(monster);
          msg("the %s whizzes past the %s", name, get_monster_name(ch));
        }
      }
      else if (hit_hero && equal(pos, player.pos))
      {
        hit_hero = false;
        changed = !changed;
        if (!save(VS_MAGIC))
        {
          if (is_frost)
          {
            msg("You are frozen by a blast of frost.");
            if (sleep_timer<20) 
                sleep_timer += spread(7);
          }
          else if (!player.stats.decrease_hp(roll(6, 6), true)) {
              if (start == &player.pos)
                  death('b');
              else
                  death(monster_at(start->y, start->x)->type);
          }
          used = true;
          if (!is_frost)
              msg("you are hit by the %s", name);
        }
        else msg("the %s whizzes by you", name);
      }
      if (is_frost || strcmp(name, "ice")==0) 
        blue(); 
      else if (strcmp(name, "bolt")==0) 
        yellow(); 
      else 
        red();
      tick_pause();
      mvaddch(pos.y, pos.x, dirch);
      standend();
    }
  }
  for (j = 0; j<i; j++)
  {
    tick_pause();
    if (spotpos[j].s_under) mvaddch(spotpos[j].s_pos.y, spotpos[j].s_pos.x, spotpos[j].s_under);
  }
  return true;
}

//charge_str: Return an appropriate string for a wand charge
const char *get_charge_string(ITEM *obj)
{
  static char buf[20];

  if (!(obj->flags&IS_KNOW) && !is_wizard()) buf[0] = '\0';
  else sprintf(buf, " [%d charges]", obj->charges);
  return buf;
}

const char* get_inv_name_stick(ITEM* stick)
{
  char *pb = prbuf;
  int which = stick->which;

  sprintf(pb, "A%s %s ", vowelstr(get_stick_type(which)), get_stick_type(which));
  pb = &prbuf[strlen(prbuf)];
  if (does_know_stick(which) || is_wizard())
    chopmsg(pb, "of %s%s", "of %s%s(%s)", get_stick_name(which), get_charge_string(stick), get_material(which));
  else if (*get_stick_guess(which))
    chopmsg(pb, "called %s", "called %s(%s)", get_stick_guess(which), get_material(which));
  else
    sprintf(pb = &prbuf[2], "%s %s", get_material(which), get_stick_type(which));

  return prbuf;
}
