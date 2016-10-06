//All the daemon and fuse functions are in here
//@(#)daemons.c       5.1 (Berkeley) 5/11/82

#include "rogue.h"
#include "game_state.h"
#include "daemons.h"
#include "daemon.h"
#include "main.h"
#include "monsters.h"
#include "rooms.h"
#include "io.h"
#include "misc.h"
#include "rip.h"
#include "curses.h"
#include "rings.h"
#include "hero.h"
#include "room.h"
#include "level.h"
#include "monster.h"

//doctor: A healing daemon that restores hit points after rest
void doctor()
{
    int lvl = game->hero().m_stats.m_level;
    int original_hp = game->hero().get_hp();

    game->turns_since_heal++;

    if (lvl < 8)
    {
        if (game->turns_since_heal + (lvl << 1) > 20)
            game->hero().increase_hp(1, false, false);
    }
    else if (game->turns_since_heal >= 3)
        game->hero().increase_hp(rnd(lvl - 7) + 1, false, false);

    if (game->hero().is_ring_on_hand(LEFT, R_REGEN))
        game->hero().increase_hp(1, false, false);
    if (game->hero().is_ring_on_hand(RIGHT, R_REGEN))
        game->hero().increase_hp(1, false, false);

    if (original_hp != game->hero().get_hp())
    {
        game->turns_since_heal = 0;
    }
}

//Swander: Called when it is time to start rolling for wandering monsters
void start_wander()
{
    daemon(roll_for_wanderer, 0);
}

//roll_for_wanderer: Called to roll to see if a wandering monster starts up
void roll_for_wanderer()
{
    static int between = 0;

    if (++between >= 3 + rnd(3))
    {
        if (roll(1, 6) == 4) {
            create_wandering_monster();
            extinguish(roll_for_wanderer);
            fuse(start_wander, 0, WANDER_TIME);
        }
        between = 0;
    }
}

//unconfuse: Release the poor player from his confusion
void unconfuse()
{
    game->hero().set_confused(false);
    msg("you feel less confused now");
}

//unsee_invisible: Turn off the ability to see invisible
void unsee_invisible()
{
    Monster *th;

    for (auto it = game->level().monsters.begin(); it != game->level().monsters.end(); ++it) {
        th = *it;
        if (th->is_invisible() && game->hero().can_see_monster(th) && th->has_tile_beneath())
            game->screen().mvaddch(th->position(), th->tile_beneath());
    }
    game->hero().set_sees_invisible(false);
}

//sight: He gets his sight back
void sight()
{
    if (game->hero().is_blind())
    {
        extinguish(sight);
        game->hero().set_blind(false);
        if (!game->hero().room()->is_gone())
            enter_room(game->hero().position());
        msg("the veil of darkness lifts");
    }
}

//nohaste: End the hasting
void nohaste()
{
    game->hero().set_is_fast(false);
    msg("you feel yourself slowing down");
}

//stomach: Digest the hero's food
void stomach()
{
    game->hero().digest();
}

//run_monsters: Make all the running monsters move.
void run_monsters()
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
            int dist = distance(game->hero().position(), monster->position());

            //normal turn. slow monsters only get a chance every other turn
            //mdk: slimes are slow when far from the player
            if (!(monster->is_slow() || (monster->slow_when_far() && dist > 3)) || monster->m_turn) {
                victim = monster->do_chase();
                if (victim == monster) {
                    it = next;
                    continue;
                }
            }

            //fast monsters get an extra turn
            if (!victim && monster->is_fast()) {
                victim = monster->do_chase();
                if (victim == monster) {
                    it = next;
                    continue;
                }
            }

            //flying monsters get an extra turn when far away to close the distance
            dist = distance(game->hero().position(), monster->position());
            if (!victim && monster->is_flying() && dist > 3) {
                Monster* victim = monster->do_chase();
                if (victim == monster) {
                    it = next;
                    continue;
                }
            }
            monster->m_turn ^= true;
        }
        ++it;
    }
}
