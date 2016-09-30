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

bool(Stick::*stick_functions[MAXSTICKS])(Coord) =
{
  &Stick::zap_light,
  &Stick::zap_striking,
  &Stick::zap_lightning,
  &Stick::zap_fire,
  &Stick::zap_cold,
  &Stick::zap_polymorph,
  &Stick::zap_magic_missile,
  &Stick::zap_haste_monster,
  &Stick::zap_slow_monster,
  &Stick::zap_drain_life,
  &Stick::zap_nothing,
  &Stick::zap_teleport_away,
  &Stick::zap_teleport_to,
  &Stick::zap_cancellation
};

bool StickInfo::is_staff(int which) const
{
    return m_type[which] == "staff";
}

std::string StickInfo::get_type(int which) const
{
    return m_type[which];
}

Item* create_stick()
{
    int which = pick_one(game->sticks().m_magic_props);
    return new Stick(which);
}

Monster* get_monster_in_direction(Coord dir, bool check_distant)
{
    Coord pos = game->hero().pos + dir;
    while (check_distant && step_ok(game->level().get_tile_or_monster(pos))) {
        pos = pos + dir;
    }
    return game->level().monster_at(pos);
}

struct MagicMissile : public Item
{
    MagicMissile() : 
        Item(MISSILE, 0)
    {
        throw_damage = "1d8";
        hit_plus = 1000;
        damage_plus = 1;
        flags = IS_MISL;
        m_launcher = NONE;

        bool use_throw_damage(game->get_environment("use_throw_damage") != "false");
        if (!use_throw_damage) {
            //mdk: this was a hack to get magic missles to use throw damage in the fight code.
            //it's not needed with my change to use throw damage.
            if (game->hero().get_current_weapon() != NULL) {
                m_launcher = game->hero().get_current_weapon()->which;
            }
        }
    }

    virtual Item* Clone() const { 
        return new MagicMissile(*this);
    }

    virtual std::string Name() const { 
        return "magic missile"; 
    }

    virtual std::string InventoryName() const { 
        return "magic missile";
    }

    virtual bool IsEvil() const { 
        return false; 
    }

    virtual int Worth() const {
        return 0; 
    }
};


bool Stick::zap_light(Coord dir) 
{
    //Ready Kilowatt wand.  Light up the room
    if (game->hero().is_blind()) 
        msg("you feel a warm glow around you");
    else
    {
        discover();
        if (game->hero().room->is_gone()) 
            msg("the corridor glows and then fades");
        else 
            msg("the room is lit by a shimmering blue light");
    }

    if (!game->hero().room->is_gone())
    {
        game->hero().room->set_dark(false);
        //Light the room and put the player back up
        enter_room(game->hero().pos);
    }

    return true;
}

bool Stick::zap_striking(Coord dir)
{
    Monster* monster = get_monster_in_direction(dir, false);
    if (!monster)
        return true;

    randomize_damage();
    game->hero().fight(monster->position(), this, false);

    //mdk:bugfix: originally the stick would be drained here, but it
    //also gets drained in Hero::fight.  I don't think the double 
    //draining was intentional.
    bool double_drain(game->get_environment("double_drain") == "bug_on");

    return double_drain ? true : false;
}

bool Stick::zap_lightning(Coord dir)
{
    fire_bolt(&game->hero().pos, &dir, "bolt");
    discover();
    return true;
}

bool Stick::zap_fire(Coord dir)
{
    fire_bolt(&game->hero().pos, &dir, "flame");
    discover();
    return true;
}

bool Stick::zap_cold(Coord dir)
{
    fire_bolt(&game->hero().pos, &dir, "ice");
    discover();
    return true;
}

bool Stick::zap_polymorph(Coord dir)
{
    Monster* monster = get_monster_in_direction(dir, true);
    if (!monster)
        return true;

    //cancel the holding effect as the monster is changing
    if (monster->can_hold())
        game->hero().set_is_held(false);

    //restore the level tile, as the new monster may be invisible
    Coord p = monster->position();
    if (game->hero().can_see_monster(monster))
        game->screen().mvaddch(p, game->level().get_tile(p));

    //create a random monster
    Monster* new_monster = Monster::CreateMonster(rnd(26) + 'A', &p, get_level());
    new_monster->oldch = monster->oldch;
    new_monster->pack = monster->pack;
    if (new_monster->type != monster->type)
        discover();
    if (game->hero().can_see_monster(new_monster))
        game->screen().mvaddch(p, new_monster->type);

    //the monster chases the player
    new_monster->dest = &game->hero().pos;
    new_monster->set_running(true);

    //destroy the original
    game->level().monsters.remove(monster);
    delete monster;

    return true;
}

bool Stick::zap_magic_missile(Coord dir) 
{
    discover();

    Item* missile = new MagicMissile;
    do_motion(missile, dir);

    Agent* monster;
    if ((monster = game->level().monster_at(missile->pos)) != NULL && !save_throw(VS_MAGIC, monster))
        projectile_hit(missile->pos, missile);
    else
        msg("the missile vanishes with a puff of smoke");

    return true;
}

bool Stick::zap_haste_monster(Coord dir)
{
    Monster* monster = get_monster_in_direction(dir, true);
    if (monster)
    {
        if (monster->is_slow())
            monster->set_is_slow(false);
        else
            monster->set_is_fast(true);
        monster->start_run();
    }
    return true;
}

bool Stick::zap_slow_monster(Coord dir)
{
    Monster* monster = get_monster_in_direction(dir, true);
    if (monster)
    {
        if (monster->is_fast())
            monster->set_is_fast(false);
        else
            monster->set_is_slow(true);
        monster->turn = true;
        monster->start_run();
    }
    return true;
}

bool Stick::zap_drain_life(Coord dir) 
{
    //Take away 1/2 of hero's hit points, then take it away evenly from the monsters in the room (or next to hero if he is in a passage)
    if (game->hero().get_hp() < 2) {
        msg("you are too weak to use it");
        return false;
    }
    drain();
    return true;
}

bool Stick::zap_nothing(Coord dir) 
{
    debug("what a bizarre schtick!");
    return true;
}

bool Stick::zap_teleport_away(Coord dir)
{
    Monster* monster = get_monster_in_direction(dir, true);
    if (!monster)
        return true;
    
    //erase the monster from the screen
    if (game->hero().can_see_monster(monster))
        game->screen().mvaddch(monster->position(), monster->oldch);

    //pick a new location for the monster
    Coord new_pos;
    monster->oldch = UNSET;
    find_empty_location(&new_pos, true);
    monster->pos = new_pos;

    //the monster can no longer hold the player
    if (monster->can_hold())
        game->hero().set_is_held(false);

    //the monster chases the player
    monster->dest = &game->hero().pos;
    monster->set_running(true);

    return true;
}

bool Stick::zap_teleport_to(Coord dir)
{
    Monster* monster = get_monster_in_direction(dir, true);
    if (!monster)
        return true;

    //erase the monster from the screen
    if (game->hero().can_see_monster(monster))
        game->screen().mvaddch(monster->position(), monster->oldch);

    //move the monster to beside the player
    monster->pos = game->hero().pos + dir;

    //mdk:bugfix: originally zapping a flytrap would release the hold,
    //but this doesn't make sense
    bool zap_release(game->get_environment("zap_release") == "bug_on");
    if (zap_release && monster->can_hold())
        game->hero().set_is_held(false);

    //the monster chases the player
    monster->dest = &game->hero().pos;
    monster->set_running(true);

    return true;
}

bool Stick::zap_cancellation(Coord dir) 
{
    Monster* monster = get_monster_in_direction(dir, true);
    if (!monster)
        return true;
    
    if (monster->can_hold())    
        game->hero().set_is_held(false);

    monster->set_cancelled(true);
    monster->set_invisible(false);
    monster->set_can_confuse(false);
    monster->reveal_disguise();

    //the monster chases the player
    monster->dest = &game->hero().pos;
    monster->set_running(true);

    return true;
}

bool Weapon::zap_vorpalized_weapon(Coord dir)
{
    Monster* monster = get_monster_in_direction(dir, true);
    if (!monster)
        return true;
        
    //a vorpalized weapon kills the target instantly
    if (is_vorpalized_against(monster))
    {
        msg("the %s vanishes in a puff of smoke", monster->get_name().c_str());
        killed(monster, false);
    }
    else {
        msg("you hear a maniacal chuckle in the distance.");

        //mdk:bugfix: originally zapping a flytrap would release the hold,
        //but this doesn't make sense
        bool zap_release(game->get_environment("zap_release") == "bug_on");
        if (zap_release && monster->can_hold())
            game->hero().set_is_held(false);

        //the monster chases the player
        monster->dest = &game->hero().pos;
        monster->set_running(true);
    }
    return true;
}

//do_zap: Perform a zap with a wand
bool do_zap()
{
    Coord delta;
    if (!get_dir(&delta))
        return false;

    Item *item = get_item("zap with", STICK);
    if (!item)
        return false;

    Stick* stick = dynamic_cast<Stick*>(item);
    Weapon* weapon = dynamic_cast<Weapon*>(item);
    if (!(stick || weapon && weapon->is_vorpalized() && weapon->get_charges())) {
        //mdk: zapping with non-stick doesn't count as turn
        msg("you can't zap with that!");
        return false;
    }

    // Handle the special case of a vorpalized weapon
    if (weapon) {
        if (weapon->zap_vorpalized_weapon(delta)) {
            weapon->use_charge();
        }
        return true;
    }

    // Zap the appropriate stick
    if (stick->get_charges() == 0) {
        msg("nothing happens");
    }
    else if ((stick->*stick_functions[stick->which])(delta)) {
        stick->use_charge();
    }

    return true;
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

struct MagicBolt : public Weapon
{
    MagicBolt(std::string name, bool from_player) :
        Weapon(MAGIC_BOLT, 30, 0),
        from_player(from_player)
    {
        m_name = name;
    }

    bool is_frost() const {
        return m_name == "frost";
    }

    bool is_flame() const {
        return m_name == "flame";
    }

    bool is_ice() const {
        return m_name == "ice";
    }

    bool is_lightning() const {
        return m_name == "bolt";
    }

    bool from_player;
};

bool bolt_vs_hero(MagicBolt* bolt, Coord start)
{
    if (save(VS_MAGIC)) {
        msg("the %s whizzes by you", bolt->Name().c_str());
        return false;
    }

    if (bolt->is_frost())
    {
        msg("You are frozen by a blast of frost.");
        if (game->sleep_timer < 20)
            game->sleep_timer += spread(7);
    }
    else {
        game->log("battle", "Flame 6d6 damage to player");
        if (!game->hero().decrease_hp(roll(6, 6), true)) {
            if (bolt->from_player)
                death('b');
            else
                death(game->level().monster_at(start)->type);
        }
        msg("you are hit by the %s", bolt->Name().c_str());
    }

    return true;
}

bool bolt_vs_monster(MagicBolt* bolt, Monster* monster, Monster**victim)
{
    bool hit = false;

    Coord pos = monster->position();
    if (monster->oldch != UNSET)
        monster->oldch = game->level().get_tile(pos);

    if (!save_throw(VS_MAGIC, monster) || bolt->is_frost())
    {
        hit = true;
        if (bolt->is_flame() && monster->immune_to_fire())
            msg("the flame bounces off the %s", monster->get_name().c_str());
        else
        {
            *victim = projectile_hit(pos, bolt); //todo: look into this hack, monster projectiles treated as hero's weapon
        }
    }
    else if (monster->is_disguised())
    {
        //todo:if throw items at xerox, they can land on top of him
        //todo:bug: this can cause xerox running around while still disguised
        if (bolt->from_player)
            monster->start_run();
        msg("the %s whizzes past the %s", bolt->Name().c_str(), monster->get_name().c_str());
    }

    return hit;
}

//fire_bolt: Fire a bolt in a given direction from a specific starting place
//shared between player and monsters (ice monster, dragon)
//todo: player bolts don't disappear when kill a monster
Monster* fire_bolt(Coord *start, Coord *dir, MagicBolt* bolt)
{
    byte dirch, ch;
    Monster* monster;
    bool bolt_hit_something, changed;
    int i, j;
    struct { Coord s_pos; byte s_under; } spotpos[BOLT_LENGTH * 2];
    Monster* victim = 0;

    switch (dir->y + dir->x)
    {
    case 0: dirch = '/'; break;
    case 1: case -1: dirch = (dir->y == 0 ? '-' : '|'); break;
    case 2: case -2: dirch = '\\'; break;
    }
    bolt->pos = *start;
    bool hero_is_target = !bolt->from_player;
    bolt_hit_something = false;
    changed = false;
    for (i = 0; i < BOLT_LENGTH && !bolt_hit_something; i++)
    {
        bolt->pos = bolt->pos + *dir;

        ch = game->level().get_tile_or_monster(bolt->pos);
        spotpos[i].s_pos = bolt->pos;
        if ((spotpos[i].s_under = game->screen().mvinch(bolt->pos.y, bolt->pos.x)) == dirch)
            spotpos[i].s_under = 0;
        switch (ch)
        {
        case DOOR: case HWALL: case VWALL: case ULWALL: case URWALL: case LLWALL: case LRWALL: case ' ':
            if (!changed) 
                hero_is_target = !hero_is_target;
            changed = false;
            dir->y = -dir->y;
            dir->x = -dir->x;
            i--;
            msg("the %s bounces", bolt->Name().c_str());
            break;

        default:
            if (!hero_is_target && (monster = game->level().monster_at(bolt->pos)) != NULL)
            {
                hero_is_target = true;
                changed = !changed;
                if (bolt_vs_monster(bolt, monster, &victim))
                {
                    bolt_hit_something = true;
                    if (game->screen().mvinch(bolt->pos.y, bolt->pos.x) != dirch)
                        spotpos[i].s_under = game->screen().mvinch(bolt->pos.y, bolt->pos.x);
                }
            }

            else if (hero_is_target && equal(bolt->pos, game->hero().pos))
            {
                hero_is_target = false;
                changed = !changed;
                if (bolt_vs_hero(bolt, *start)) {
                    bolt_hit_something = true;
                }
            }

            //draw bolt
            if (bolt->is_frost() || bolt->is_ice())
                game->screen().blue();
            else if (bolt->is_lightning())
                game->screen().yellow();
            else
                game->screen().red();
            tick_pause();
            game->screen().mvaddch(bolt->pos, dirch);
            game->screen().standend();
        }
    }

    //restore ground
    for (j = 0; j < i; j++)
    {
        tick_pause();
        if (spotpos[j].s_under)
            game->screen().mvaddch(spotpos[j].s_pos, spotpos[j].s_under);
    }

    return victim;
}

Monster* fire_bolt(Coord *start, Coord *dir, const std::string& name) {
    bool from_player(start == &game->hero().pos);
    return fire_bolt(start, dir, new MagicBolt(name, from_player)); //todo:who owns memory?
}

//charge_str: Return an appropriate string for a wand charge
const char *get_charge_string(const Item *obj)
{
    static char buf[20];

    if (!obj->is_known() && !game->wizard().reveal_items()) buf[0] = '\0';
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
    if (is_discovered(which) || game->wizard().reveal_items())
        chopmsg(pb, "of %s%s", "of %s%s(%s)", get_name(which).c_str(), charge.c_str(), material.c_str());
    else if (!get_guess(which).empty())
        chopmsg(pb, "called %s", "called %s(%s)", get_guess(which).c_str(), material.c_str());
    else
        sprintf(pb = &prbuf[2], "%s %s", material.c_str(), type.c_str());

    return prbuf;
}

std::string StickInfo::get_inventory_name(const Item * obj) const
{
    return get_inventory_name(obj->which, get_charge_string(obj));
}

std::string StickInfo::get_inventory_name(int which) const
{
    return get_inventory_name(which, "");
}

//e.g. we'd crash if we started the hero off with a wand
Stick::Stick(int which)
    : Item(STICK, which)
{
    StickInfo* sticks = dynamic_cast<StickInfo*>(item_class());
    if (sticks->is_staff(which))
        damage = "2d3";  //mdk: A staff is more powerful than a wand for striking 
    else
        damage = "1d1";

    throw_damage = "1d1";
    charges = 3 + rnd(5);

    switch (which)
    {
    case WS_HIT:
        hit_plus = 100;
        //mdk: i don't know why damage info is set, as they are overwritten the first time you
        //zap a monster.  I don't know if the intention was to have separate zap/melee stats,
        //but, as is, it's just odd.
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

std::string Stick::Name() const
{
    StickInfo* sticks = dynamic_cast<StickInfo*>(item_class());
    return sticks->get_type(which);
}

std::string Stick::InventoryName() const
{
    return item_class()->get_inventory_name(this);
}

bool Stick::IsEvil() const
{
    return (which == WS_HASTE_M || which == WS_TELTO);
}

int Stick::Worth() const
{
    int worth = item_class()->get_value(which);
    worth += 20 * get_charges();
    if (!is_known())
        worth /= 2;
    return worth;
}

