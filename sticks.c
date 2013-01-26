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

//fix_stick: Set up a new stick
void fix_stick(ITEM *cur)
{
  if (strcmp(ws_type[cur->which], "staff")==0) cur->damage = "2d3";
  else cur->damage = "1d1";
  cur->throw_damage = "1d1";
  cur->charges = 3+rnd(5);
  switch (cur->which)
  {
  case WS_HIT: cur->hit_plus = 100; cur->damage_plus = 3; cur->damage = "1d8"; break;
  case WS_LIGHT: cur->charges = 10+rnd(10); break;
  }
}

//do_zap: Perform a zap with a wand
void do_zap()
{
  ITEM *obj;
  AGENT *tp;
  int y, x;
  char *name;
  int which_one;

  if ((obj = get_item("zap with", STICK))==NULL) return;
  which_one = obj->which;
  if (obj->type!=STICK)
  {
    if (obj->enemy && obj->charges) which_one = MAXSTICKS;
    else {msg("you can't zap with that!"); after = FALSE; return;}
  }
  if (obj->charges==0) {msg("nothing happens"); return;}
  switch (which_one)
  {
  case WS_LIGHT: //Ready Kilowat wand.  Light up the room
    if (on(player,ISBLIND)) msg("you feel a warm glow around you");
    else
    {
      ws_know[WS_LIGHT] = TRUE;
      if (player.t_room->r_flags&ISGONE) msg("the corridor glows and then fades");
      else msg("the room is lit by a shimmering blue light");
    }
    if (!(player.t_room->r_flags&ISGONE))
    {
      player.t_room->r_flags &= ~ISDARK;
      //Light the room and put the player back up
      enter_room(&player.t_pos);
    }
    break;

  case WS_DRAIN: //Take away 1/2 of hero's hit points, then take it away evenly from the monsters in the room (or next to hero if he is in a passage)
    if (player.t_stats.s_hpt<2) {msg("you are too weak to use it"); return;}
    else drain();
    break;

  case WS_POLYMORPH: case WS_TELAWAY: case WS_TELTO: case WS_CANCEL: case MAXSTICKS: //Special case for vorpal weapon
    {
      byte monster, oldch;
      int rm;
      coord new_yx;

      y = player.t_pos.y;
      x = player.t_pos.x;
      while (step_ok(display_character(y, x))) {y += delta.y; x += delta.x;}
      if ((tp = monster_at(y, x))!=NULL)
      {
        byte omonst;

        omonst = monster = tp->t_type;
        if (monster=='F') player.t_flags &= ~ISHELD;
        if (which_one==MAXSTICKS)
        {
          if (monster==obj->enemy)
          {
            msg("the %s vanishes in a puff of smoke", monsters[monster-'A'].m_name);
            killed(tp, FALSE);
          }
          else msg("you hear a maniacal chuckle in the distance.");
        }
        else if (which_one==WS_POLYMORPH)
        {
          ITEM *pp;

          pp = tp->t_pack;
          detach_agent(&mlist, tp);
          if (can_see_monst(tp)) mvaddch(y, x, get_tile(y, x));
          oldch = tp->t_oldch;
          delta.y = y;
          delta.x = x;
          new_monster(tp, monster = rnd(26)+'A', &delta);
          if (can_see_monst(tp)) mvaddch(y, x, monster);
          tp->t_oldch = oldch;
          tp->t_pack = pp;
          ws_know[WS_POLYMORPH] |= (monster!=omonst);
        }
        else if (which_one==WS_CANCEL)
        {
          tp->t_flags |= ISCANC;
          tp->t_flags &= ~(ISINVIS|CANHUH);
          tp->t_disguise = tp->t_type;
        }
        else
        {
          if (can_see_monst(tp)) mvaddch(y, x, tp->t_oldch);
          if (which_one==WS_TELAWAY)
          {
            tp->t_oldch = '@';
            do {
              rm = rnd_room(); 
              new_yx = tp->t_pos; 
              rnd_pos(&rooms[rm], &new_yx);
            } while (!(isfloor(display_character(new_yx.y, new_yx.x))));
            tp->t_pos = new_yx;
            if (can_see_monst(tp)) 
              mvaddch(tp->t_pos.y, tp->t_pos.x, tp->t_disguise);
            else if (on(player, SEEMONST)) {
              standout(); 
              mvaddch(tp->t_pos.y, tp->t_pos.x, tp->t_disguise); 
              standend();
            }
          }
          else {
            tp->t_pos.y = player.t_pos.y+delta.y; 
            tp->t_pos.x = player.t_pos.x+delta.x;
          } //it MUST BE at WS_TELTO
          if (tp->t_type=='F') 
            player.t_flags &= ~ISHELD;
          if (tp->t_pos.y!=y || tp->t_pos.x!=x)
            tp->t_oldch = mvinch(tp->t_pos.y, tp->t_pos.x);
        }
        tp->t_dest = &player.t_pos;
        tp->t_flags |= ISRUN;
      }
      break;
    }

  case WS_MISSILE:
    {
      ITEM bolt;

      ws_know[WS_MISSILE] = TRUE;
      bolt.type = '*';
      bolt.throw_damage = "1d8";
      bolt.hit_plus = 1000;
      bolt.damage_plus = 1;
      bolt.flags = ISMISL;
      if (cur_weapon!=NULL) bolt.launcher = cur_weapon->which;
      do_motion(&bolt, delta.y, delta.x);
      if ((tp = monster_at(bolt.pos.y, bolt.pos.x))!=NULL && !save_throw(VS_MAGIC, tp))
        hit_monster(bolt.pos.y, bolt.pos.x, &bolt);
      else msg("the missile vanishes with a puff of smoke");

      break;
    }

  case WS_HIT:
    delta.y += player.t_pos.y;
    delta.x += player.t_pos.x;
    if ((tp = monster_at(delta.y, delta.x))!=NULL)
    {
      if (rnd(20)==0) {obj->damage = "3d8"; obj->damage_plus = 9;}
      else {obj->damage = "2d8"; obj->damage_plus = 4;}
      fight(&delta, tp->t_type, obj, FALSE);
    }
    break;

  case WS_HASTE_M: case WS_SLOW_M:
    y = player.t_pos.y;
    x = player.t_pos.x;
    while (step_ok(display_character(y, x))) {y += delta.y; x += delta.x;}
    if ((tp = monster_at(y, x))!=NULL)
    {
      if (which_one==WS_HASTE_M)
      {
        if (on(*tp, ISSLOW)) tp->t_flags &= ~ISSLOW;
        else tp->t_flags |= ISHASTE;
      }
      else
      {
        if (on(*tp, ISHASTE)) tp->t_flags &= ~ISHASTE;
        else tp->t_flags |= ISSLOW;
        tp->t_turn = TRUE;
      }
      delta.y = y;
      delta.x = x;
      start_run(&delta);
    }
    break;

  case WS_ELECT: case WS_FIRE: case WS_COLD:
    if (which_one==WS_ELECT) name = "bolt";
    else if (which_one==WS_FIRE) name = "flame";
    else name = "ice";
    fire_bolt(&player.t_pos, &delta, name);
    ws_know[which_one] = TRUE;
    break;

  default: debug("what a bizarre schtick!"); break;
  }
  if (--obj->charges<0) obj->charges = 0;
}

//drain: Do drain hit points from player schtick
void drain()
{
  AGENT *mp;
  int cnt;
  struct room *corp;
  AGENT **dp;
  bool inpass;
  AGENT *drainee[40];

  //First count how many things we need to spread the hit points among
  cnt = 0;
  if (get_tile(player.t_pos.y, player.t_pos.x)==DOOR) corp = &passages[get_flags(player.t_pos.y, player.t_pos.x)&F_PNUM];
  else corp = NULL;
  inpass = (player.t_room->r_flags&ISGONE);
  dp = drainee;
  for (mp = mlist; mp!=NULL; mp = next(mp)){
    if (mp->t_room==player.t_room || mp->t_room==corp || (inpass && get_tile(mp->t_pos.y, mp->t_pos.x)==DOOR && &passages[get_flags(mp->t_pos.y, mp->t_pos.x)&F_PNUM]==player.t_room)) {
      *dp++ = mp;
    }
  }
  if ((cnt = dp-drainee)==0) {
    msg("you have a tingling feeling"); 
    return;
  }
  *dp = NULL;
  player.t_stats.s_hpt /= 2;
  cnt = player.t_stats.s_hpt/cnt+1;
  //Now zot all of the monsters
  for (dp = drainee; *dp; dp++)
  {
    mp = *dp;
    if ((mp->t_stats.s_hpt -= cnt)<=0) killed(mp, can_see_monst(mp));
    else start_run(&mp->t_pos);
  }
}

//fire_bolt: Fire a bolt in a given direction from a specific starting place
void fire_bolt(coord *start, coord *dir, char *name)
{
  byte dirch, ch;
  AGENT *tp;
  bool hit_hero, used, changed;
  int i, j;
  coord pos;
  struct {coord s_pos; byte s_under;} spotpos[BOLT_LENGTH*2];
  ITEM bolt;
  bool is_frost;

  is_frost = (strcmp(name, "frost")==0);
  bolt.type = WEAPON;
  bolt.which = FLAME;
  bolt.damage = bolt.throw_damage = "6d6";
  bolt.hit_plus = 30;
  bolt.damage_plus = 0;
  w_names[FLAME] = name;
  switch (dir->y+dir->x)
  {
  case 0: dirch = '/'; break;
  case 1: case -1: dirch = (dir->y==0?'-':'|'); break;
  case 2: case -2: dirch = '\\'; break;
  }
  pos = *start;
  hit_hero = (start!=&player.t_pos);
  used = FALSE;
  changed = FALSE;
  for (i = 0; i<BOLT_LENGTH && !used; i++)
  {
    pos.y += dir->y;
    pos.x += dir->x;
    ch = display_character(pos.y, pos.x);
    spotpos[i].s_pos = pos;
    if ((spotpos[i].s_under = mvinch(pos.y, pos.x))==dirch) spotpos[i].s_under = 0;
    switch (ch)
    {
    case DOOR: case HWALL: case VWALL: case ULWALL: case URWALL: case LLWALL: case LRWALL: case ' ':
      if (!changed) hit_hero = !hit_hero;
      changed = FALSE;
      dir->y = -dir->y;
      dir->x = -dir->x;
      i--;
      msg("the %s bounces", name);
      break;

    default:
      if (!hit_hero && (tp = monster_at(pos.y, pos.x))!=NULL)
      {
        hit_hero = TRUE;
        changed = !changed;
        if (tp->t_oldch!='@') tp->t_oldch = get_tile(pos.y, pos.x);
        if (!save_throw(VS_MAGIC, tp) || is_frost)
        {
          bolt.pos = pos;
          used = TRUE;
          if (tp->t_type=='D' && strcmp(name, "flame")==0) msg("the flame bounces off the dragon");
          else
          {
            hit_monster(pos.y, pos.x, &bolt);
            if (mvinch(pos.y, pos.x)!=dirch) spotpos[i].s_under = mvinch(pos.y, pos.x);
          }
        }
        else if (ch!='X' || tp->t_disguise=='X')
        {
          if (start==&player.t_pos) start_run(&pos);
          msg("the %s whizzes past the %s", name, monsters[ch-'A'].m_name);
        }
      }
      else if (hit_hero && ce(pos, player.t_pos))
      {
        hit_hero = FALSE;
        changed = !changed;
        if (!save(VS_MAGIC))
        {
          if (is_frost)
          {
            msg("You are frozen by a blast of frost%s.", noterse(" from the Ice Monster"));
            if (no_command<20) no_command += spread(7);
          }
          else if ((player.t_stats.s_hpt -= roll(6, 6))<=0) if (start==&player.t_pos) death('b'); else death(monster_at(start->y, start->x)->t_type);
          used = TRUE;
          if (!is_frost) msg("you are hit by the %s", name);
        }
        else msg("the %s whizzes by you", name);
      }
      if (is_frost) blue(); else red();
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
}

//charge_str: Return an appropriate string for a wand charge
char *charge_str(ITEM *obj)
{
  static char buf[20];

  if (!(obj->flags&ISKNOW) && !wizard) buf[0] = '\0';
  else sprintf(buf, " [%d charges]", obj->charges);
  return buf;
}
