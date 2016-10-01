//File with various monster functions in it
//monsters.c  1.4 (A.I. Design)       12/14/84

#include <fstream>
#include <sstream>
#include <string>
#include <algorithm>

#include "rogue.h"
#include "monsters.h"
#include "daemons.h"
#include "list.h"
#include "main.h"
#include "rooms.h"
#include "things.h"
#include "io.h"
#include "misc.h"
#include "daemon.h"
#include "fight.h"
#include "rings.h"
#include "rooms.h"
#include "level.h"
#include "room.h"
#include "game_state.h"
#include "wizard.h"
#include "hero.h"
#include "monster.h"

#pragma warning(disable:4996)

namespace
{
    //Array containing information on all the various types of monsters
    struct MonsterEntry
    {
        std::string name;   //What to call the monster
        int carry;          //Probability of carrying something
        short flags;        //Things about the monster
        struct Agent::Stats stats; //Initial stats
        int confuse_roll;
        int exflags;        //Things about special roles
    };

#define ___  1
#define XX  10

    //mdk: IS_REGEN has no effect
    struct MonsterEntry monsters[26] =
    {
        // Name           CARRY                        FLAG    str,  exp,lvl,amr, hpt, dmg
        { "aquator",          0,                    IS_MEAN,  { XX,   20,  5,  2, ___, "0d0/0d0"         }, 0, EX_RUSTS_ARMOR },
        { "bat",              0,                     IS_FLY,  { XX,    1,  1,  3, ___, "1d2"             }, 2, 0 },
        { "centaur",         15,                          0,  { XX,   25,  4,  4, ___, "1d6/1d6"         }, 0, 0 },
        { "dragon",         100,                    IS_MEAN,  { XX, 6800, 10, -1, ___, "1d8/1d8/3d10"    }, 0, EX_SHOOTS_FIRE },
        { "emu",              0,                    IS_MEAN,  { XX,    2,  1,  7, ___, "1d2"             }, 0, 0 },
        { "venus flytrap",    0,                    IS_MEAN,  { XX,   80,  8,  3, ___, "0d1"             }, 0, EX_HOLDS | EX_STATIONARY | EX_HOLD_ATTACKS },
        { "griffin",         20, IS_MEAN | IS_FLY | IS_REGEN, { XX, 2000, 13,  2, ___, "4d3/3d5/4d3"     }, 0, 0 },
        { "hobgoblin",        0,                    IS_MEAN,  { XX,    3,  1,  5, ___, "1d8"             }, 0, 0 },
        { "ice monster",      0,                    IS_MEAN,  { XX,   15,  1,  9, ___, "1d2"             }, 0, EX_SHOOTS_ICE },
        { "jabberwock",      70,                          0,  { XX, 4000, 15,  6, ___, "2d12/2d4"        }, 0, 0 },
        { "kestral",          0,           IS_MEAN | IS_FLY,  { XX,    1,  1,  7, ___, "1d4"             }, 0, 0 },
        { "leprechaun",       0,                          0,  { XX,   10,  3,  8, ___, "1d2"             }, 0, EX_STEALS_GOLD | EX_DROPS_GOLD | EX_SUICIDES },
        { "medusa",          40,                    IS_MEAN,  { XX,  200,  8,  2, ___, "3d4/3d4/2d5"     }, 0, EX_CONFUSES },
        { "nymph",          100,                          0,  { XX,   37,  3,  9, ___, "0d0"             }, 0, EX_STEALS_MAGIC | EX_SUICIDES },
        { "orc",             15,                   IS_GREED,  { XX,    5,  1,  6, ___, "1d8"             }, 0, 0 },
        { "phantom",          0,                   IS_INVIS,  { XX,  120,  8,  3, ___, "4d4"             }, 5, 0 },
        { "quagga",          30,                    IS_MEAN,  { XX,   32,  3,  2, ___, "1d2/1d2/1d4"     }, 0, 0 },
        { "rattlesnake",      0,                    IS_MEAN,  { XX,    9,  2,  3, ___, "1d6"             }, 0, EX_DRAINS_STR },
        { "slime",            0,                    IS_MEAN,  { XX,    1,  2,  8, ___, "1d3"             }, 0, EX_DIVIDES },
        { "troll",           50,         IS_REGEN | IS_MEAN,  { XX,  120,  6,  4, ___, "1d8/1d8/2d6"     }, 0, 0 },
        { "ur-vile",          0,                    IS_MEAN,  { XX,  190,  7, -2, ___, "1d3/1d3/1d3/4d6" }, 0, 0 },
        { "vampire",         20,         IS_REGEN | IS_MEAN,  { XX,  350,  8,  1, ___, "1d10"            }, 0, EX_DRAINS_MAXHP },
        { "wraith",           0,                          0,  { XX,   55,  5,  4, ___, "1d6"             }, 0, EX_DRAINS_EXP },
        { "xeroc",           30,                          0,  { XX,  100,  7,  7, ___, "3d4"             }, 0, EX_MIMICS },
        { "yeti",            30,                          0,  { XX,   50,  4,  6, ___, "1d6/1d6"         }, 0, 0 },
        { "zombie",           0,                    IS_MEAN,  { XX,    6,  2,  8, ___, "1d8"             }, 0, 0 }
    };

#undef ___
#undef XX

    //List of monsters in rough order of vorpalness
    char  lvl_mons[] = "K BHISOR LCA NYTWFP GMXVJD";
    char wand_mons[] = "KEBHISORZ CAQ YTW PUGM VJ ";
}

//todo: validation, move to better location
void load_monster_cfg(const std::string& filename)
{
    std::ifstream file(filename, std::ios::in);
    std::string line;
    while (std::getline(file, line)) {
        if (line.empty() || line[0] == '#')
            continue;

        if (line.substr(0, 7) == "l_order") {
            std::string l = line.substr(9, 26);
            l.copy(lvl_mons, 26);
        }
        else if (line.substr(0, 7) == "w_order") {
            std::string w = line.substr(9, 26);
            w.copy(wand_mons, 26);
        }

        std::istringstream ss(line);

        char type;
        MonsterEntry m;
        ss >> type;
        ss >> m.name;
        ss >> m.carry;
        ss >> std::hex >> m.flags;
        ss >> std::dec >> m.stats.m_str >> m.stats.m_exp >> m.stats.level >> m.stats.ac >> m.stats.hp;
        ss >> m.stats.damage;
        ss >> std::dec >> m.confuse_roll;
        ss >> std::hex >> m.exflags;

        std::replace(m.name.begin(), m.name.end(), '_', ' ');

        monsters[type - 'A'] = m;
        //printf("%c %s %d %x %d %d %d %d %d %s %x\n", type, m.name.c_str(), m.carry, m.flags, m.stats.str, m.stats.exp, m.stats.level, m.stats.ac, m.stats.hp, m.stats.damage.c_str(), m.exflags);
    }
}

const char* get_monster_name(char monster)
{
    return monsters[monster - 'A'].name.c_str();
}

int Monster::get_carry_probability() const
{
    return monsters[type - 'A'].carry;
}

//randmonster: Pick a monster to show up.  The lower the level, the meaner the monster.
char randmonster(bool wander, int level)
{
    int d;
    const char* mons = wander ? wand_mons : lvl_mons;
    do
    {
        int r10 = rnd(5) + rnd(6);
        d = level + (r10 - 5);
        if (d < 1) d = rnd(5) + 1;
        if (d > 26) d = rnd(5) + 22;
    } while (mons[--d] == ' ');
    return mons[d];
}

void set_disguise(Monster* X)
{
    switch (rnd(get_level() >= AMULETLEVEL ? 9 : 8))
    {
    case 0: X->disguise = GOLD; break;
    case 1: X->disguise = POTION; break;
    case 2: X->disguise = SCROLL; break;
    case 3: X->disguise = STAIRS; break;
    case 4: X->disguise = WEAPON; break;
    case 5: X->disguise = ARMOR; break;
    case 6: X->disguise = RING; break;
    case 7: X->disguise = STICK; break;
    case 8: X->disguise = AMULET; break;
    }
}

//create_monster: Pick a new monster and add it to the list
Monster* Monster::CreateMonster(byte type, Coord *position, int level)
{
    Monster* monster = new Monster;
    int level_add = (level <= AMULETLEVEL) ? 0 : level - AMULETLEVEL;
    
    const MonsterEntry* defaults = &monsters[type - 'A'];
    monster->type = type;
    monster->disguise = type;
    monster->pos = *position;
    monster->invalidate_tile_beneath();
    monster->room = get_room_from_position(*position);
    monster->flags = defaults->flags;
    monster->exflags = defaults->exflags;
    monster->stats = defaults->stats;
    monster->stats.level += level_add;
    monster->stats.hp = monster->stats.max_hp = roll(monster->stats.level, 8);
    monster->stats.ac -= level_add;
    monster->stats.m_exp += level_add * 10 + exp_add(monster);
    monster->turn = true;
    monster->value = 0;
    monster->confuse_roll = defaults->confuse_roll;

    if (monster->is_mimic())
        set_disguise(monster);

    game->level().monsters.push_front(monster);

    if (game->hero().is_wearing_ring(R_AGGR))
        monster->start_run();

    return monster;
}

//expadd: Experience to add for this monster's level/hit points
int exp_add(Monster *monster)
{
    int divisor = (monster->stats.level == 1) ? 8 : 6;
    int value = monster->stats.max_hp / divisor;

    if (monster->stats.level > 9)
        value *= 20;
    else if (monster->stats.level > 6)
        value *= 4;

    return value;
}

//create_wandering_monster: Create a new wandering monster and aim it at the player
void create_wandering_monster()
{
    struct Room *room;
    Monster* monster;
    Coord cp;

    do
    {
        room = rnd_room();
        if (room == game->hero().room) continue;
        rnd_pos(room, &cp);
    } while (!(room != game->hero().room && step_ok(game->level().get_tile_or_monster(cp))));  //todo:bug: can start on mimic?
    monster = Monster::CreateMonster(randmonster(true, get_level()), &cp, get_level());
    if (game->invalid_position)
        debug("wanderer bailout");
    //debug("started a wandering %s", monsters[tp->type-'A'].m_name);
    monster->start_run();
}

//wake_monster: What to do when the hero steps next to a monster
Monster *wake_monster(Coord p)
{
    Monster *monster;

    if ((monster = game->level().monster_at(p)) == NULL)
        return monster;
    //Every time he sees mean monster, it might start chasing him
    if (!monster->is_running() && rnd(3) != 0 && monster->is_mean() && !monster->is_held() && !game->hero().is_wearing_ring(R_STEALTH))
    {
        monster->start_run(&game->hero().pos, false);
    }
    if (monster->causes_confusion() && !game->hero().is_blind() && !monster->is_found() && !monster->powers_cancelled() && monster->is_running())
    {
        int dst;
        Room* room = game->hero().room;
        dst = distance(p, game->hero().pos);
        if ((room != NULL && !(room->is_dark())) || dst < LAMP_DIST)
        {
            monster->set_found(true);
            if (!save(VS_MAGIC))
            {
                if (game->hero().is_confused()) lengthen(unconfuse, rnd(20) + HUH_DURATION);
                else fuse(unconfuse, 0, rnd(20) + HUH_DURATION);
                game->hero().set_confused(true);
                msg("the %s's gaze has confused you", monster->get_name().c_str());
            }
        }
    }
    //Let greedy ones guard gold
    if (monster->is_greedy() && !monster->is_running())
    {
        Coord* dest = &game->hero().pos;
        if (game->hero().room->gold_val)
            dest = &game->hero().room->gold;
        monster->start_run(dest);
    }
    return monster;
}

//pick_vorpal_monster: Choose a sort of monster for the enemy of a vorpally enchanted weapon
char pick_vorpal_monster()
{
    const char *p;
    do {
        // Start at end of enemies list and walk backwards, 
        // with a 10% chance of stopping at any given entry.
        // Default to M if we have bad luck.
        p = lvl_mons + strlen(lvl_mons);
        while (--p >= lvl_mons && rnd(10));
        if (p < lvl_mons)
            return 'M';
    } while (*p == ' ');

    return *p;
}
