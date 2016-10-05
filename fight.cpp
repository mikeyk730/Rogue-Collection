//All the fighting gets done here
//@(#)fight.c          1.43 (AI Design)                1/19/85

#include <cstdlib>
#include <cstdio>
#include <cctype>
#include <algorithm>
#include <sstream>
#include <iomanip>
#include "rogue.h"
#include "game_state.h"
#include "hero.h"
#include "fight.h"
#include "list.h"
#include "weapons.h"
#include "monsters.h"
#include "io.h"
#include "rip.h"
#include "things.h"
#include "main.h"
#include "curses.h"
#include "slime.h"
#include "potions.h"
#include "misc.h"
#include "mach_dep.h"
#include "level.h"
#include "rings.h"
#include "armor.h"
#include "pack.h"
#include "monster.h"
#include "gold.h"

char tbuf[MAXSTR];

const char* it = "it";
const char* you = "you";

//attempt_swing: Returns true if the swing hits
bool attempt_swing(int lvl, int defender_amr, int hplus)
{
    int roll = rnd(20);
    int got = roll + hplus;
    int need = (20 - lvl) - defender_amr;
    bool hit(got >= need);

    std::ostringstream ss;
    ss << (hit ? "HIT: " : "MISS: ") << got << " ? " << need
        << " (1d20=" << roll << " + hplus=" << hplus << ") ? (20 - lvl=" << lvl << " - amr=" << defender_amr << ")";
    game->log("battle", ss.str());

    return hit;
}

//prname: The print name of a combatant
char *prname(const char *who, bool upper)
{
    *tbuf = '\0';
    if (who == 0) strcpy(tbuf, you);
    else if (game->hero().is_blind()) strcpy(tbuf, it);
    else { strcpy(tbuf, "the "); strcat(tbuf, who); }
    if (upper) *tbuf = toupper(*tbuf);
    return tbuf;
}

//hit: Print a message to indicate a successful hit
void display_hit_msg(const char *er, const char *ee)
{
    char *s;

    addmsg(prname(er, true));
    switch ((short_msgs()) ? 1 : rnd(4))
    {
    case 0: s = " scored an excellent hit on "; break;
    case 1: s = " hit "; break;
    case 2: s = (er == 0 ? " have injured " : " has injured "); break;
    case 3: s = (er == 0 ? " swing and hit " : " swings and hits "); break;
    }
    msg("%s%s", s, prname(ee, false));
}

//display_miss_msg: Print a message to indicate a poor swing
void display_miss_msg(const char *er, const char *ee)
{
    char *s;

    addmsg(prname(er, true));
    switch ((short_msgs()) ? 1 : rnd(4))
    {
    case 0: s = (er == 0 ? " swing and miss" : " swings and misses"); break;
    case 1: s = (er == 0 ? " miss" : " misses"); break;
    case 2: s = (er == 0 ? " barely miss" : " barely misses"); break;
    case 3: s = (er == 0 ? " don't hit" : " doesn't hit"); break;
    }
    msg("%s %s", s, prname(ee, false));
}

//save_throw: See if a creature save against something
int save_throw(int which, Agent *monster)
{
    int need = 14 + which - monster->m_stats.m_level / 2;
    int r = roll(1, 20);
    bool save(r >= need);

    std::ostringstream ss;
    ss << "1d20 save throw " << (save ? "success" : "failed") << " " << r << " ? " << need << " (14+w:" << which << "-lvl:" << monster->m_stats.m_level << "/2)";
    game->log("battle", ss.str());

    return save;
}

//save: See if he saves against various nasty things
int save(int which)
{
    if (which == VS_MAGIC)
    {
        if (game->hero().is_ring_on_hand(LEFT, R_PROTECT))
            which -= game->hero().get_ring(LEFT)->get_ring_level();
        if (game->hero().is_ring_on_hand(RIGHT, R_PROTECT))
            which -= game->hero().get_ring(RIGHT)->get_ring_level();
    }
    return save_throw(which, &game->hero());
}

//str_plus: Compute bonus/penalties for strength on the "to hit" roll
int str_plus(unsigned int str)
{
    int add = 4;

    if (str < 8) return str - 7;
    if (str < 31) add--;
    if (str < 21) add--;
    if (str < 19) add--;
    if (str < 17) add--;
    return add;
}

//add_dam: Compute additional damage done for exceptionally high or low strength
int add_dam(unsigned int str)
{
    int add = 6;

    if (str < 8) return str - 7;
    if (str < 31) add--;
    if (str < 22) add--;
    if (str < 20) add--;
    if (str < 18) add--;
    if (str < 17) add--;
    if (str < 16) add--;
    return add;
}

//display_throw_msg: A projectile hit or missed a monster
void display_throw_msg(Item *item, const char *name, char *does, char *did)
{
    addmsg("the %s %s ", item->name().c_str(), does);
    game->hero().is_blind() ? msg(it) : msg("the %s", name);
}

//remove: Remove a monster from the screen
void remove_monster(Monster* monster, bool waskill)
{
    Coord monster_pos = monster->position();
    for (auto it = monster->m_pack.begin(); it != monster->m_pack.end();) {
        Item* obj = *(it++);
        obj->set_position(monster->position());
        monster->m_pack.remove(obj);
        if (waskill)
            fall(obj, false);
        else
            delete(obj);
    }
    if (game->level().get_tile(monster_pos) == PASSAGE)
        game->screen().standout();
    if (monster->tile_beneath() == FLOOR && !game->hero().can_see(monster_pos))
        game->screen().mvaddch(monster_pos, ' ');
    else if (monster->has_tile_beneath())
        game->screen().mvaddch(monster_pos, monster->tile_beneath());
    game->screen().standend();

    game->level().monsters.remove(monster);
    delete monster;
}

//killed_by_hero: Called to put a monster to death
void killed_by_hero(Monster* monster, bool print)
{
    //If the monster was a flytrap, un-hold him
    if (game->hero().is_held_by(monster)) {
        game->hero().clear_hold();
    }

    if (monster->drops_gold()) {
        int value = rnd_gold();
        if (save(VS_MAGIC))
            value += rnd_gold() + rnd_gold() + rnd_gold() + rnd_gold();
        Item *gold = new Gold(value);
        monster->m_pack.push_front(gold);
    }

    if (print)
    {
        addmsg("you have defeated ");
        if (game->hero().is_blind())
            msg(it);
        else
            msg("the %s", monster->get_name().c_str());
    }

    game->hero().gain_experience(monster->experience());

    //Get rid of the monster.
    remove_monster(monster, true);
}
