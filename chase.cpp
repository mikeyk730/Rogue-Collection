//Code for one creature to chase another
//chase.c     1.32    (A.I. Design) 12/12/84

#include <stdlib.h>

#include "rogue.h"
#include "chase.h"
#include "game_state.h"
#include "level.h"
#include "monster.h"
#include "misc.h"
#include "hero.h"

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
