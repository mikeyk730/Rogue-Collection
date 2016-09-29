//Functions to implement the various sticks one might find while wandering around the dungeon.
//@(#)sticks.c1.2 (AI Design) 2/12/84

#include <stdio.h>

#include "rogue.h"
#include "item_class.h"
#include "game_state.h"
#include "sticks.h"
#include "monsters.h"
#include "pack.h"
#include "io.h"
#include "output_interface.h"
#include "main.h"
#include "chase.h"
#include "fight.h"
#include "rooms.h"
#include "misc.h"
#include "weapons.h"
#include "rip.h"
#include "level.h"
#include "list.h"
#include "mach_dep.h"
#include "things.h"
#include "hero.h"
#include "room.h"
#include "monster.h"

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

StickInfo::StickInfo()
{
    m_magic_props =
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

    int i, j;
    char *str;
    bool metused[NMETAL], woodused[NWOOD];

    for (i = 0; i < NWOOD; i++)
        woodused[i] = false;
    for (i = 0; i < NMETAL; i++)
        metused[i] = false;
    for (i = 0; i < MAXSTICKS; i++)
    {
        for (;;) if (rnd(2) == 0)
        {
            j = rnd(NMETAL);
            if (!metused[j]) {
                m_type.push_back("wand");
                str = metal[j];
                metused[j] = true;
                break;
            }
        }
        else
        {
            j = rnd(NWOOD);
            if (!woodused[j]) {
                m_type.push_back("staff");
                str = wood[j];
                woodused[j] = true;
                break;
            }
        }
        m_identifier.push_back(str);
        if (i > 0)
            m_magic_props[i].prob += m_magic_props[i - 1].prob;
    }
}

bool StickInfo::is_staff(int which) const
{
    return m_type[which] == "staff";
}

std::string StickInfo::get_type(int which) const
{
    return game->sticks().m_type[which];
}


Item* create_stick()
{
    int which = pick_one(game->sticks().m_magic_props);
    return new Stick(which);
}

void zap_light()
{
    //Ready Kilowatt wand.  Light up the room
    if (game->hero().is_blind()) msg("you feel a warm glow around you");
    else
    {
        game->sticks().discover(WS_LIGHT);
        if (game->hero().room->is_gone()) msg("the corridor glows and then fades");
        else msg("the room is lit by a shimmering blue light");
    }
    if (!game->hero().room->is_gone())
    {
        game->hero().room->set_dark(false);
        //Light the room and put the player back up
        enter_room(game->hero().pos);
    }
}

void zap_striking(Coord delta, Item* obj)
{
    Agent* monster;
    Coord coord = game->hero().pos + delta;

    if ((monster = game->level().monster_at(coord)) != NULL)
    {
        obj->randomize_damage();
        game->hero().fight(&coord, obj, false);
    }
}

void zap_bolt(Coord delta, int which, const char* name)
{
    fire_bolt(&game->hero().pos, &delta, name);
    game->sticks().discover(which);
}

void zap_vorpalized_weapon(Item* weapon, Monster* monster)
{
    if (weapon->is_vorpalized_against(monster))
    {
        msg("the %s vanishes in a puff of smoke", monster->get_name().c_str());
        killed(monster, false);
    }
    else
        msg("you hear a maniacal chuckle in the distance.");
}

void zap_polymorph(Monster* monster, Coord p)
{
    if (game->hero().can_see_monster(monster))
        game->screen().mvaddch(p, game->level().get_tile(p));

    Monster* new_monster = Monster::CreateMonster(rnd(26) + 'A', &p, get_level());
    game->level().monsters.remove(new_monster);

    new_monster->oldch = monster->oldch;
    new_monster->pack = monster->pack;
    if (new_monster->type != monster->type)
        game->sticks().discover(WS_POLYMORPH);

    *monster = *new_monster;
    delete new_monster;

    //move to front of list to maintain original behavior
    game->level().monsters.remove(monster);
    game->level().monsters.push_front(monster);

    if (game->hero().can_see_monster(monster))
        game->screen().mvaddch(p, monster->type);
}

void zap_cancellation(Monster* monster)
{
    monster->set_cancelled(true);
    monster->set_invisible(false);
    monster->set_can_confuse(false);
    monster->reveal_disguise();
}

void zap_teleport(Coord delta, Monster* monster, Coord p, int which)
{
    Coord new_pos;

    if (game->hero().can_see_monster(monster))
        game->screen().mvaddch(p, monster->oldch);

    if (which == WS_TELAWAY)
    {
        monster->oldch = MDK;
        find_empty_location(&new_pos, true);
        monster->pos = new_pos;
    }
    else { //it MUST BE at WS_TELTO
        monster->pos = game->hero().pos + delta;
    }

    if (monster->can_hold())
        game->hero().set_is_held(false);
}

void zap_generic(Coord delta, Item* wand, int which)
{
    Monster* monster;
    Coord pos = game->hero().pos;

    while (step_ok(game->level().get_tile_or_monster(pos))) {
        pos = pos + delta;
    }
    if ((monster = game->level().monster_at(pos)) != NULL)
    {
        if (monster->can_hold())
            game->hero().set_is_held(false);
        if (which == MAXSTICKS)
        {
            zap_vorpalized_weapon(wand, monster);
        }
        else if (which == WS_POLYMORPH)
        {
            zap_polymorph(monster, pos);
        }
        else if (which == WS_CANCEL)
        {
            zap_cancellation(monster);
        }
        else
        {
            zap_teleport(delta, monster, pos, which);
        }
        monster->dest = &game->hero().pos;
        monster->set_running(true);
    }
}

struct MagicMissile : public Item
{
    MagicMissile() : Item(MISSILE, 0, "magic missile")
    {
        throw_damage = "1d8";
        hit_plus = 1000;
        damage_plus = 1;
        flags = IS_MISL;

        if (game->hero().get_current_weapon() != NULL)
            m_launcher = game->hero().get_current_weapon()->which;
    }
    virtual Item* Clone() const { return new MagicMissile(*this); }
};

void zap_magic_missile(Coord delta)
{
    game->sticks().discover(WS_MISSILE);

    Item* bolt = new MagicMissile;
    do_motion(bolt, delta);

    Agent* monster;
    if ((monster = game->level().monster_at(bolt->pos)) != NULL && !save_throw(VS_MAGIC, monster))
        projectile_hit(bolt->pos, bolt);
    else
        msg("the missile vanishes with a puff of smoke");
}

void zap_speed_monster(Coord delta, int which)
{
    Monster* monster;

    Coord pos = game->hero().pos;
    while (step_ok(game->level().get_tile_or_monster(pos))) {
        pos = pos + delta;
    }
    if (monster = game->level().monster_at(pos))
    {
        if (which == WS_HASTE_M)
        {
            if (monster->is_slow())
                monster->set_is_slow(false);
            else
                monster->set_is_fast(true);
        }
        else
        {
            if (monster->is_fast())
                monster->set_is_fast(false);
            else
                monster->set_is_slow(true);
            monster->turn = true;
        }
        monster->start_run();
    }
}

int zap_drain_life()
{
    //Take away 1/2 of hero's hit points, then take it away evenly from the monsters in the room (or next to hero if he is in a passage)
    if (game->hero().get_hp() < 2) {
        msg("you are too weak to use it");
        return false;
    }
    drain();
    return true;
}

//do_zap: Perform a zap with a wand
void do_zap(Coord delta)
{
    Item *obj;
    int which_one;

    if ((obj = get_item("zap with", STICK)) == NULL)
        return;
    which_one = obj->which;

    if (obj->type != STICK)
    {
        if (obj->is_vorpalized() && obj->get_charges())
            which_one = MAXSTICKS;
        else {
            msg("you can't zap with that!");
            game->counts_as_turn = false;
            return;
        }
    }

    if (obj->get_charges() == 0) {
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
        zap_generic(delta, obj, which_one);
        break;

    case WS_MISSILE:
        zap_magic_missile(delta);
        break;

    case WS_HIT:
        zap_striking(delta, obj);
        break;

    case WS_HASTE_M: case WS_SLOW_M:
        zap_speed_monster(delta, which_one);
        break;

    case WS_ELECT:
        zap_bolt(delta, which_one, "bolt");
        break;

    case WS_FIRE:
        zap_bolt(delta, which_one, "flame");
        break;

    case WS_COLD:
        zap_bolt(delta, which_one, "ice");
        break;

    default:
        debug("what a bizarre schtick!");
        break;
    }

    obj->use_charge();
}

//drain: Do drain hit points from player schtick
void drain()
{
    Monster* monster;
    int cnt;
    struct Room *room;
    Monster **dp;
    bool in_passage;
    Monster *drainee[40];

    //First count how many things we need to spread the hit points among
    cnt = 0;
    if (game->level().get_tile(game->hero().pos) == DOOR)
        room = game->level().get_passage(game->hero().pos);
    else room = NULL;
    in_passage = game->hero().room->is_gone();
    dp = drainee;
    for (auto it = game->level().monsters.begin(); it != game->level().monsters.end(); ++it) {
        monster = *it;
        if (monster->room == game->hero().room || monster->room == room ||
            (in_passage && game->level().get_tile(monster->pos) == DOOR && game->level().get_passage(monster->pos) == game->hero().room)) {
            *dp++ = monster;
        }
    }
    if ((cnt = dp - drainee) == 0) {
        msg("you have a tingling feeling");
        return;
    }
    *dp = NULL;
    cnt = game->hero().drain_hp() / cnt + 1;
    //Now zot all of the monsters
    for (dp = drainee; *dp; dp++)
    {
        monster = *dp;
        if (!monster->decrease_hp(cnt, true))
            killed(monster, game->hero().can_see_monster(monster));
        else
            monster->start_run();
    }
}

//fire_bolt: Fire a bolt in a given direction from a specific starting place
//shared between player and monsters (ice monster, dragon)
//todo: player bolts don't disappear when kill a monster
Monster* fire_bolt(Coord *start, Coord *dir, const std::string& name)
{
    byte dirch, ch;
    Monster* monster;
    bool hit_hero, used, changed;
    int i, j;
    Coord pos;
    struct { Coord s_pos; byte s_under; } spotpos[BOLT_LENGTH * 2];
    bool is_frost(name == "frost");
    bool is_flame(name == "flame");
    Monster* victim = 0;

    Item* bolt = new Weapon(FLAME, 30, 0);
    bolt->set_name(name);
    switch (dir->y + dir->x)
    {
    case 0: dirch = '/'; break;
    case 1: case -1: dirch = (dir->y == 0 ? '-' : '|'); break;
    case 2: case -2: dirch = '\\'; break;
    }
    pos = *start;
    hit_hero = (start != &game->hero().pos);
    used = false;
    changed = false;
    for (i = 0; i < BOLT_LENGTH && !used; i++)
    {
        pos.y += dir->y;
        pos.x += dir->x;
        ch = game->level().get_tile_or_monster(pos);
        spotpos[i].s_pos = pos;
        if ((spotpos[i].s_under = game->screen().mvinch(pos.y, pos.x)) == dirch)
            spotpos[i].s_under = 0;
        switch (ch)
        {
        case DOOR: case HWALL: case VWALL: case ULWALL: case URWALL: case LLWALL: case LRWALL: case ' ':
            if (!changed) hit_hero = !hit_hero;
            changed = false;
            dir->y = -dir->y;
            dir->x = -dir->x;
            i--;
            msg("the %s bounces", name.c_str());
            break;

        default:
            if (!hit_hero && (monster = game->level().monster_at(pos)) != NULL)
            {
                hit_hero = true;
                changed = !changed;
                if (monster->oldch != MDK)
                    monster->oldch = game->level().get_tile(pos);
                if (!save_throw(VS_MAGIC, monster) || is_frost)
                {
                    bolt->pos = pos;
                    used = true;
                    if (is_flame && monster->immune_to_fire())
                        msg("the flame bounces off the %s", monster->get_name().c_str());
                    else
                    {
                        victim = projectile_hit(pos, bolt); //todo: look into this hack, monster projectiles treated as hero's weapon
                        if (game->screen().mvinch(pos.y, pos.x) != dirch)
                            spotpos[i].s_under = game->screen().mvinch(pos.y, pos.x);
                    }
                }
                else if (!monster->is_disguised())
                {
                    if (start == &game->hero().pos)
                        monster->start_run();
                    msg("the %s whizzes past the %s", name.c_str(), get_monster_name(ch));
                }
            }
            else if (hit_hero && equal(pos, game->hero().pos))
            {
                hit_hero = false;
                changed = !changed;
                if (!save(VS_MAGIC))
                {
                    if (is_frost)
                    {
                        msg("You are frozen by a blast of frost.");
                        if (game->sleep_timer < 20)
                            game->sleep_timer += spread(7);
                    }
                    else {
                        game->log("battle", "Flame 6d6 damage to player");
                        if (!game->hero().decrease_hp(roll(6, 6), true)) {
                            if (start == &game->hero().pos)
                                death('b');
                            else
                                death(game->level().monster_at(*start)->type);
                        }
                    }
                    used = true;
                    if (!is_frost)
                        msg("you are hit by the %s", name.c_str());
                }
                else msg("the %s whizzes by you", name.c_str());
            }
            if (is_frost || name == "ice")
                game->screen().blue();
            else if (name == "bolt")
                game->screen().yellow();
            else
                game->screen().red();
            tick_pause();
            game->screen().mvaddch(pos, dirch);
            game->screen().standend();
        }
    }
    for (j = 0; j < i; j++)
    {
        tick_pause();
        if (spotpos[j].s_under)
            game->screen().mvaddch(spotpos[j].s_pos, spotpos[j].s_under);
    }
    return victim;
}

//charge_str: Return an appropriate string for a wand charge
const char *get_charge_string(Item *obj)
{
    static char buf[20];

    if (!obj->is_known() && !game->hero().is_wizard()) buf[0] = '\0';
    else sprintf(buf, " [%d charges]", obj->get_charges());
    return buf;
}

std::string StickInfo::get_inventory_name(int which, const std::string& charge) const
{
    char *pb = prbuf;
    std::string type = get_type(which);
    std::string material = get_identifier(which);

    sprintf(pb, "A%s %s ", vowelstr(type.c_str()), type.c_str());
    pb = &prbuf[strlen(prbuf)];
    if (is_discovered(which) || game->hero().is_wizard())
        chopmsg(pb, "of %s%s", "of %s%s(%s)", get_name(which).c_str(), charge.c_str(), material.c_str());
    else if (!get_guess(which).empty())
        chopmsg(pb, "called %s", "called %s(%s)", get_guess(which).c_str(), material.c_str());
    else
        sprintf(pb = &prbuf[2], "%s %s", material.c_str(), type.c_str());

    return prbuf;
}

std::string StickInfo::get_inventory_name(Item * obj) const
{
    return get_inventory_name(obj->which, get_charge_string(obj));
}

std::string StickInfo::get_inventory_name(int which) const
{
    return get_inventory_name(which, "");
}

//todo: it's problematic to need to poke into the game here
//e.g. we'd crash if we started the hero off with a wand
Stick::Stick(int which)
    : Item(STICK, which, game->sticks().get_type(which))
{
    //mdk: A staff is more powerful than a wand for striking 
    if (game->sticks().is_staff(which))
        damage = "2d3";
    else
        damage = "1d1";

    throw_damage = "1d1";
    charges = 3 + rnd(5);

    switch (which)
    {
    case WS_HIT:
        hit_plus = 100;
        damage_plus = 3;
        damage = "1d8";
        break;
    case WS_LIGHT:
        charges = 10 + rnd(10);
        break;
    }
}

Item * Stick::Clone() const
{
    return new Stick(*this);
}
