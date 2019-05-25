//Code for handling the various special properties of the slime
//slime.c     1.0     (A.I. Design 1.42)      1/17/85

#include "random.h"
#include "slime.h"
#include "misc.h"
#include "output_shim.h"
#include "io.h"
#include "monsters.h"
#include "main.h"
#include "level.h"
#include "scrolls.h"
#include "game_state.h"
#include "hero.h"
#include "monster.h"

static Coord slime_pos;

//Slime_split: Called when it has been decided that A slime should divide itself

void slime_split(Monster* monster)
{
    if (new_slime(monster) == 0)
        return;

    msg("The %s divides.  Ick!", monster->get_name().c_str());
    Monster* nslime = Monster::CreateMonster(monster->m_type, &slime_pos, game->get_level());
    if (game->hero().can_see(slime_pos))
    {
        nslime->reload_tile_beneath();
        nslime->render();
    }
    nslime->start_run();
}

int new_slime(Monster *slime)
{
    int y, x, ty, tx, ret;
    Monster *ntp;
    Coord sp;

    ret = 0;
    slime->set_dirty(true);
    if (plop_monster((ty = slime->position().y), (tx = slime->position().x), &sp) == 0)
    {
        //There were no open spaces next to this slime, look for other slimes that might have open spaces next to them.
        for (y = ty - 1; y <= ty + 1; y++)
            for (x = tx - 1; x <= tx + 1; x++)
                if (game->level().get_tile_or_monster({ x, y }) == slime->m_type && (ntp = game->level().monster_at({ x, y })))
                {
                    if (ntp->is_dirty())
                        continue; //Already done this one
                    if (new_slime(ntp)) { y = ty + 2; x = tx + 2; }
                }
    }
    else { ret = 1; slime_pos = sp; }
    slime->set_dirty(false);
    return ret;
}

bool plop_monster(int r, int c, Coord *cp)
{
    int y, x;
    int appear = 0;
    byte ch;

    for (y = r - 1; y <= r + 1; y++)
        for (x = c - 1; x <= c + 1; x++)
        {
            Coord pos = { x, y };
            //Don't put a monster on top of the player.
            if (pos == game->hero().position() || offmap({ x,y }))
                continue;
            //Or anything else nasty
            if (step_ok(ch = game->level().get_tile_or_monster(pos)))//todo:bug: on mimic?
            {
                if (ch == SCROLL && is_scare_monster_scroll(find_obj(pos, false)))
                    continue;
                if (rnd(++appear) == 0) {
                    *cp = pos;
                }
            }
        }
    return appear;
}
