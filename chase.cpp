//Code for one creature to chase another
//chase.c     1.32    (A.I. Design) 12/12/84

#include <stdlib.h>

#include "rogue.h"
#include "chase.h"
#include "fight.h"
#include "move.h"
#include "io.h"
#include "sticks.h"
#include "misc.h"
#include "output_interface.h"
#include "main.h"
#include "monsters.h"
#include "list.h"
#include "level.h"
#include "weapons.h"
#include "scrolls.h"
#include "pack.h"
#include "room.h"
#include "game_state.h"
#include "hero.h"
#include "monster.h"


//todo: move to env or remove
//orcs should pick up gold in a room, then chase the player.
//a bug prevented orcs from picking up gold, so they'd just
//remain on the gold space.
const bool alt_orc_behavior = true;

#define DRAGONSHOT  5 //one chance in DRAGONSHOT that a dragon will flame

Coord ch_ret; //Where chasing takes you

//runners: Make all the running monsters move.
void runners()
{
    //Todo: Major refactor needed.  Monsters can die during do_chase.  Leprechauns
    //and Nymphs disappear as part of their attack.  Ice Monsters and Dragons can
    //kill themselves or others with their projectiles.  The logic here to avoid
    //iterator invalidation is horrendous.
    for (auto it = game->level().monsters.begin(); it != game->level().monsters.end();)
    {
        //save the next iterator in case the monster dies during its own turn
        auto next = it;
        ++next;

        Monster* monster = *(it);
        if (!monster->is_held() && monster->is_running())
        {
            Monster* victim = 0;
            int dist = distance(game->hero().pos, monster->pos);
            if (!(monster->is_slow() || (monster->can_divide() && dist > 3)) || monster->turn) {
                victim = monster->do_chase();
                if (victim == monster) {
                    it = next;
                    continue;
                }
            }

            // fast monsters get an extra turn
            if (!victim && monster->is_fast()) {
                victim = monster->do_chase();
                if (victim == monster) {
                    it = next;
                    continue;
                }
            }

            // flying monsters get an extra turn to close the distance
            dist = distance(game->hero().pos, monster->pos);
            if (!victim && monster->is_flying() && dist > 3) {
                Monster* victim = monster->do_chase();
                if (victim == monster) {
                    it = next;
                    continue;
                }
            }
            monster->turn ^= true;
        }
        ++it;
    }
}

//do_chase: Make one thing chase another.
Monster* Monster::do_chase()
{
    //If gold has been taken, target the hero
    if (is_greedy() && room->gold_val == 0)
        dest = &game->hero().pos;

    //Find room of the target
    Room* destination_room = game->hero().room;
    if (dest != &game->hero().pos)
        destination_room = get_room_from_position(dest);
    if (destination_room == NULL)
        return 0;


    int mindist = 32767, i, dist;
    Item *obj;
    Room *oroom;
    Coord tempdest; //Temporary destination for chaser


    Room* monster_room = this->room; //Find room of chaser
    //We don't count doors as inside rooms for this routine
    bool door = game->level().get_tile(pos) == DOOR;

    //If the object of our desire is in a different room, and we are not in a maze, run to the door nearest to our goal.

over:

    if (monster_room != destination_room && (monster_room->is_maze()) == 0)
    {
        //loop through doors
        for (i = 0; i < monster_room->num_exits; i++)
        {
            dist = distance(*(this->dest), monster_room->exits[i]);
            if (dist < mindist) {
                tempdest = monster_room->exits[i];
                mindist = dist;
            }
        }
        if (door)
        {
            monster_room = game->level().get_passage(pos);
            door = false;
            goto over;
        }
    }
    else
    {
        tempdest = *this->dest;
        //For monsters which can fire bolts at the poor hero, we check to see if 
        // (a) the hero is on a straight line from it, and 
        // (b) that it is within shooting distance, but outside of striking range.
        if ((this->shoots_fire() || this->shoots_ice()) &&
            (this->pos.y == game->hero().pos.y || this->pos.x == game->hero().pos.x || abs(this->pos.y - game->hero().pos.y) == abs(this->pos.x - game->hero().pos.x)) &&
            ((dist = distance(this->pos, game->hero().pos)) > 2 && dist <= BOLT_LENGTH*BOLT_LENGTH) && !this->powers_cancelled() && rnd(DRAGONSHOT) == 0)
        {
            game->modifiers.m_running = false;
            delta.y = sign(game->hero().pos.y - this->pos.y);
            delta.x = sign(game->hero().pos.x - this->pos.x);
            return fire_bolt(&this->pos, &delta, this->shoots_fire() ? "flame" : "frost");
        }
    }
    //This now contains what we want to run to this time so we run to it. If we hit it we either want to fight it or stop running
    this->chase(&tempdest);
    if (equal(ch_ret, game->hero().pos)) {
        return attack_player();
    }
    else if (equal(ch_ret, *this->dest))
    {
        for (auto it = game->level().items.begin(); it != game->level().items.end(); ) {
            obj = *(it++);
            if (alt_orc_behavior && (*this->dest == obj->pos) ||
                !alt_orc_behavior && (this->dest == &obj->pos))
            {
                byte oldchar;
                game->level().items.remove(obj);
                this->pack.push_front(obj);
                oldchar = (this->room->is_gone()) ? PASSAGE : FLOOR;
                game->level().set_tile(obj->pos, oldchar);
                if (game->hero().can_see(obj->pos))
                    game->screen().mvaddch(obj->pos, oldchar);
                set_destination();
                break;
            }
        }
    }
    if (this->is_stationary())
        return 0;
    //If the chasing thing moved, update the screen
    if (this->oldch != MDK)
    {
        if (this->oldch == ' ' && game->hero().can_see(this->pos) && game->level().get_tile(this->pos) == FLOOR)
            game->screen().mvaddch(this->pos, (char)FLOOR);
        else if (this->oldch == FLOOR && !game->hero().can_see(this->pos) && !game->hero().detects_others())
            game->screen().mvaddch(this->pos, ' ');
        else
            game->screen().mvaddch(this->pos, this->oldch);
    }
    oroom = this->room;
    if (!equal(ch_ret, this->pos))
    {
        if ((this->room = get_room_from_position(&ch_ret)) == NULL) {
            this->room = oroom;
            return 0;
        }
        if (oroom != this->room)
            set_destination();
        this->pos = ch_ret;
    }
    if (game->hero().can_see_monster(this))
    {
        if (game->level().get_flags(ch_ret)&F_PASS) game->screen().standout();
        this->oldch = game->screen().mvinch(ch_ret.y, ch_ret.x);
        game->screen().mvaddch(ch_ret, this->disguise);
    }
    else if (game->hero().detects_others())
    {
        game->screen().standout();
        this->oldch = game->screen().mvinch(ch_ret.y, ch_ret.x);
        game->screen().mvaddch(ch_ret, this->type);
    }
    else
        this->oldch = MDK;
    if (this->oldch == FLOOR && oroom->is_dark())
        this->oldch = ' ';
    game->screen().standend();
    return 0;
}

//can_see_monster: Return true if the hero can see the monster
bool Hero::can_see_monster(Agent *monster)
{
  // player is blind
  if (is_blind())
    return false;

  //monster is invisible, and can't see invisible
  if (monster->is_invisible() && !sees_invisible())
    return false;
  
  if (distance(monster->pos, pos) >= LAMP_DIST &&
    (monster->room != room || monster->room->is_dark() || monster->room->is_maze()))
    return false;
  
  //If we are seeing the enemy of a vorpally enchanted weapon for the first time, 
  //give the player a hint as to what that weapon is good for.
  Item* weapon = get_current_weapon();
  if (weapon && weapon->is_vorpalized_against(monster) && !weapon->did_flash())
  {
    weapon->set_flashed();
    msg(flash, get_weapon_name(weapon->which), short_msgs()?"":intense);
  }
  return true;
}

//chase: Find the spot for the chaser(er) to move closer to the chasee(ee). Returns true if we want to keep on chasing later. false if we reach the goal.
void Monster::chase(Coord *chasee_pos)
{
  int x, y;
  int dist, thisdist;
  Coord *chaser_pos;
  byte ch;
  int plcnt = 1;

  chaser_pos = &this->pos;
  //If the thing is confused, let it move randomly. Phantoms are slightly confused all of the time, and bats are quite confused all the time
  if (this->is_monster_confused_this_turn())
  {
    //get a valid random move
    rndmove(this, &ch_ret);
    dist = distance(ch_ret, *chasee_pos);
    //Small chance that it will become un-confused
    if (rnd(30)==17) 
        this->set_confused(false);
  }
  //Otherwise, find the empty spot next to the chaser that is closest to the chasee.
  else
  {
    int ey, ex;

    //This will eventually hold where we move to get closer. If we can't find an empty spot, we stay where we are.
    dist = distance(*chaser_pos, *chasee_pos);
    ch_ret = *chaser_pos;
    ey = chaser_pos->y+1;
    ex = chaser_pos->x+1;
    for (x = chaser_pos->x-1; x<=ex; x++)
    {
      for (y = chaser_pos->y-1; y<=ey; y++)
      {
        Coord try_pos;

        try_pos.x = x;
        try_pos.y = y;
        if (offmap({x,y}) || !diag_ok(chaser_pos, &try_pos)) continue;
        ch = game->level().get_tile_or_monster({x,y});
        if (step_ok(ch))
        {
          //If it is a scroll, it might be a scare monster scroll so we need to look it up to see what type it is.
            if (ch == SCROLL)
            {
                Item *obj = 0;
                for (auto it = game->level().items.begin(); it != game->level().items.end(); ++it)
                {
                    obj = *it;
                    if (equal(try_pos, obj->pos))
                        break;
                    obj = 0;
                }
                if (is_scare_monster_scroll(obj)) 
                    continue;
            }
          //If we didn't find any scrolls at this place or it wasn't a scare scroll, then this place counts
          thisdist = distance({ x, y }, *chasee_pos);
          if (thisdist<dist) {plcnt = 1; ch_ret = try_pos; dist = thisdist;}
          else if (thisdist==dist && rnd(++plcnt)==0) {ch_ret = try_pos; dist = thisdist;}
        }
      }
    }
  }
}

//diag_ok: Check to see if the move is legal if it is diagonal
int diag_ok(const Coord *sp, const Coord *ep )
{
  if (ep->x==sp->x || ep->y==sp->y) return true;
  return (step_ok(game->level().get_tile({sp->x,ep->y})) && step_ok(game->level().get_tile({ep->x,sp->y})));
}

//can_see: Returns true if the hero can see a certain coordinate.
int Hero::can_see(Coord p)
{
    if (is_blind()) 
        return false;
    //if the coordinate is close.
    if (distance(p, pos) < LAMP_DIST)
        return true;
    //if the coordinate is in the same room as the hero, and the room is lit
    return (room == get_room_from_position(&p) && !room->is_dark());
}

//find_dest: find the proper destination for the monster
Coord* Monster::find_dest()
{
    // if we're in the same room as the player, or can see the player, then we go after the player
    // if we have a chance to carry an item, we may go after an unclaimed item in the same room
    int carry_prob;
    if ((carry_prob = this->get_carry_probability()) <= 0 || this->in_same_room_as(&game->hero()) || game->hero().can_see_monster(this))
        return &game->hero().pos;

    for (auto i = game->level().items.begin(); i != game->level().items.end(); ++i)
    {
        Item* obj = *i;
        if (is_scare_monster_scroll(obj))
            continue;

        if (this->in_same_room_as(obj) && rnd(100) < carry_prob)
        {
            // don't go after the same object as another monster
            Agent* monster = 0;
            for (auto m = game->level().monsters.begin(); m != game->level().monsters.end(); ++m){
                if ((*m)->is_seeking(obj)) {
                    monster = *m;
                    break;
                }
            }
            if (monster == NULL)
                return &obj->pos;
        }
    }
    return &game->hero().pos;
}

bool Agent::in_same_room_as(Agent* other)
{
    return room == other->room;
}

bool Agent::in_same_room_as(Item * obj)
{
    return room == obj->get_room();
}
