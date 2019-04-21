//Functions to implement the various sticks one might find while wandering around the dungeon.
//@(#)sticks.c1.2 (AI Design) 2/12/84
#include <sstream>
#include <stdio.h>

#include "random.h"
#include "item_category.h"
#include "game_state.h"
#include "sticks.h"
#include "monsters.h"
#include "pack.h"
#include "io.h"
#include "output_shim.h"
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

Monster* get_monster_in_direction(Coord dir, bool check_distant)
{
    Coord pos = game->hero().position() + dir;
    bool throws_affect_mimics(game->options.throws_affect_mimics());
    while (check_distant && step_ok(game->level().get_tile_or_monster(pos, throws_affect_mimics))) {
        pos = pos + dir;
    }
    return game->level().monster_at(pos);
}

struct MagicMissile : public Item
{
    MagicMissile() : 
        Item(MISSILE, 0)
    {
        m_throw_damage = "1d8";
        m_hit_plus = 1000;
        m_damage_plus = 1;
        m_flags = IS_MISL;
        m_launcher = NO_WEAPON;
    }

    virtual Item* Clone() const { 
        return new MagicMissile(*this);
    }

    virtual std::string TypeName() const { 
        return "magic missile"; 
    }

    virtual std::string InventoryName() const { 
        return "magic missile";
    }

    virtual bool IsMagic() const {
        return true;
    }

    virtual bool IsEvil() const { 
        return false; 
    }

    virtual int Worth() const {
        return 0; 
    }
};


Light::Light()
{
    m_charges = 10 + rnd(10);
}

bool Light::Zap(Coord dir)
{
    //Ready Kilowatt wand.  Light up the room
    if (game->hero().is_blind()) 
        msg("you feel a warm glow around you");
    else
    {
        discover(false);
        if (game->hero().room()->is_gone()) 
            msg("the corridor glows and then fades");
        else 
            msg("the room is lit by a shimmering blue light");
    }

    if (!game->hero().room()->is_gone())
    {
        game->hero().room()->set_dark(false);
        //Light the room and put the player back up
        enter_room(game->hero().position());
    }

    return true;
}

Striking::Striking()
{
    m_hit_plus = 100;
    //mdk:bugfix: The striking staff was made more powerful in the PC version.
    //The new stats were updated above, but neglected here.  This would only
    //come into play if the staff were wielded before the first zap.
    m_damage_plus = 4; //originally 3
    m_damage = "2d8";  //originally 1d8
}

bool Striking::Zap(Coord dir)
{
    Monster* monster = get_monster_in_direction(dir, false);
    if (!monster)
        return true;

    set_striking_damage();
    game->hero().fight(monster->position(), this, false);

    //mdk:bugfix: originally the stick would be drained here.  Since it
    //also gets drained in Hero::fight, it would use 2 charges per use.
    //I'm returning 'false' to fix the bug.  This problem was introduced
    //in Unix Rogue 5.2.
    return false;
}

bool Lightning::Zap(Coord dir)
{
    fire_bolt(game->hero().position(), &dir, "bolt");
    discover(false);
    return true;
}

bool Fire::Zap(Coord dir)
{
    fire_bolt(game->hero().position(), &dir, "flame");
    discover(false);
    return true;
}

bool Cold::Zap(Coord dir)
{
    fire_bolt(game->hero().position(), &dir, "ice");
    discover(false);
    return true;
}

bool Polymorph::Zap(Coord dir)
{
    Monster* monster = get_monster_in_direction(dir, true);
    if (!monster)
        return true;

    //cancel the holding effect as the monster is changing
    if (game->hero().is_held_by(monster)) {
        game->hero().clear_hold();
    }

    //restore the level tile, as the new monster may be invisible
    Coord p = monster->position();
    if (game->hero().can_see_monster(monster))
        game->screen().add_tile(p, game->level().get_tile(p));

    //create a random monster
    Monster* new_monster = Monster::CreateMonster(rnd(26) + 'A', &p, game->get_level());
    new_monster->set_tile_beneath(monster->tile_beneath());
    new_monster->take_pack_from(monster);
    if (new_monster->m_type != monster->m_type)
        discover(false);

    if (game->hero().can_see_monster(new_monster))
        game->screen().add_tile(p, new_monster->m_type);

    //the monster chases the player
    new_monster->start_run(&game->hero());

    //destroy the original
    game->level().monsters.remove(monster);
    delete monster;

    return true;
}

bool MagicMissileStick::Zap(Coord dir)
{
    discover(false);

    Item* missile = new MagicMissile;
    do_motion(missile, dir);

    Agent* monster;
    if ((monster = game->level().monster_at(missile->position())) != NULL && !save_throw(VS_MAGIC, monster))
        projectile_hit(missile->position(), missile);
    else
        msg("the missile vanishes with a puff of smoke");

    return true;
}

bool HasteMonster::IsEvil() const
{
    return true;
}

bool HasteMonster::Zap(Coord dir)
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

bool SlowMonster::Zap(Coord dir)
{
    Monster* monster = get_monster_in_direction(dir, true);
    if (monster)
    {
        if (monster->is_fast())
            monster->set_is_fast(false);
        else
            monster->set_is_slow(true);
        monster->m_turn = true;
        monster->start_run();
    }
    return true;
}

bool DrainLife::Zap(Coord dir)
{
    //Take away 1/2 of hero's hit points, then take it away evenly from the monsters in the room (or next to hero if he is in a passage)
    if (game->hero().get_hp() < 2) {
        msg("you are too weak to use it");
        return false;
    }
    drain();
    return true;
}

bool Nothing::Zap(Coord dir) 
{
    msg("what a bizarre schtick!");
    return true;
}

bool TeleportAway::Zap(Coord dir)
{
    Monster* monster = get_monster_in_direction(dir, true);
    if (!monster)
        return true;
    
    //erase the monster from the screen
    if (game->hero().can_see_monster(monster))
        game->screen().add_tile(monster->position(), monster->tile_beneath());

    //pick a new location for the monster
    Coord new_pos;
    monster->invalidate_tile_beneath();
    find_empty_location(&new_pos, true);
    monster->set_position(new_pos);

    //the monster can no longer hold the player
    if (game->hero().is_held_by(monster)) {
        game->hero().clear_hold();
    }

    //the monster chases the player
    monster->start_run(&game->hero());

    return true;
}

bool TeleportTo::IsEvil() const
{
    return true;
}

bool TeleportTo::Zap(Coord dir)
{
    Monster* monster = get_monster_in_direction(dir, true);
    if (!monster)
        return true;

    //erase the monster from the screen
    if (game->hero().can_see_monster(monster))
        game->screen().add_tile(monster->position(), monster->tile_beneath());
    else
        game->screen().add_tile(monster->position(), ' ');

    //move the monster to beside the player
    monster->set_position(game->hero().position() + dir);

    //the monster chases the player
    monster->start_run(&game->hero());

    return true;
}

bool Cancellation::Zap(Coord dir)
{
    Monster* monster = get_monster_in_direction(dir, true);
    if (!monster)
        return true;
    
    if (game->hero().is_held_by(monster)) {
        game->hero().clear_hold();
    }

    monster->set_cancelled(true);
    monster->set_invisible(false);
    monster->set_can_confuse(false);
    monster->reveal_disguise();

    //the monster chases the player
    monster->start_run(&game->hero());

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
        killed_by_hero(monster, false);
    }
    else {
        msg("you hear a maniacal chuckle in the distance.");

        //the monster chases the player
        monster->start_run(&game->hero());
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
    if (!(stick || weapon && weapon->is_vorpalized() && weapon->charges())) {
        //mdk: zapping with non-stick doesn't count as turn
        msg("you can't zap with that!");
        return false;
    }

    // Handle the special case of a vorpalized weapon
    if (weapon) {
        game->screen().play_sound("zap");
        if (weapon->zap_vorpalized_weapon(delta)) {
            weapon->use_charge();
        }
        return true;
    }

    // Zap the appropriate stick
    if (stick->charges() == 0) {
        msg("nothing happens");
    }
    else{
        game->screen().play_sound("zap");
        if (stick->Zap(delta)) {
            stick->use_charge();
        }
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
    if (game->level().get_tile(game->hero().position()) == DOOR)
        room = game->level().get_passage(game->hero().position());
    else room = NULL;
    in_passage = game->hero().room()->is_gone();
    dp = drainee;
    for (auto it = game->level().monsters.begin(); it != game->level().monsters.end(); ++it) {
        monster = *it;
        if (monster->room() == game->hero().room() || monster->room() == room ||
            (in_passage && game->level().get_tile(monster->position()) == DOOR && game->level().get_passage(monster->position()) == game->hero().room())) {
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
            killed_by_hero(monster, game->hero().can_see_monster(monster));
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
        msg("the %s whizzes by you", bolt->name().c_str());
        return false;
    }

    if (bolt->is_frost())
    {
        msg("You are frozen by a blast of frost.");
        if (game->hero().get_sleep_turns() < 20)
            game->hero().increase_sleep_turns(spread(7));
    }
    else {
        game->log("battle", "Flame 6d6 damage to player");
        if (!game->hero().decrease_hp(roll(6, 6), true)) {
            if (bolt->from_player)
                death('b');
            else
                death(game->level().monster_at(start)->m_type);
        }
        msg("you are hit by the %s", bolt->name().c_str());
    }

    return true;
}

bool bolt_vs_monster(MagicBolt* bolt, Monster* monster, Monster**victim)
{
    bool hit = false;

    Coord pos = monster->position();
    //todo: test code removal
    //if (monster->has_tile_beneath())
    //    monster->reload_tile_beneath();

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
    else if (!monster->is_disguised())
    {
        if (bolt->from_player)
            monster->start_run();
        msg("the %s whizzes past the %s", bolt->name().c_str(), monster->get_name().c_str());
    }

    return hit;
}

//fire_bolt: Fire a bolt in a given direction from a specific starting place
//shared between player and monsters (ice monster, dragon)
Monster* fire_bolt(Coord start, Coord *dir, MagicBolt* bolt)
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
    bolt->set_position(start);
    bool hero_is_target = !bolt->from_player;
    bolt_hit_something = false;
    changed = false;
    for (i = 0; i < BOLT_LENGTH && !bolt_hit_something; i++)
    {
        bolt->set_position(bolt->position() + *dir);

        bool throws_affect_mimics(game->options.throws_affect_mimics());
        ch = game->level().get_tile_or_monster(bolt->position(), throws_affect_mimics);
        spotpos[i].s_pos = bolt->position();
        if ((spotpos[i].s_under = game->screen().mvinch(bolt->position())) == dirch)
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
            msg("the %s bounces", bolt->name().c_str());
            break;

        default:
            if (!hero_is_target)
            {
                monster = game->level().monster_at(bolt->position(), throws_affect_mimics);
                if (monster) {
                    hero_is_target = true;
                    changed = !changed;
                    if (bolt_vs_monster(bolt, monster, &victim))
                    {
                        bolt_hit_something = true;
                        if (game->screen().mvinch(bolt->position()) != dirch)
                            spotpos[i].s_under = game->screen().mvinch(bolt->position());
                    }
                }
            }

            else if (hero_is_target && equal(bolt->position(), game->hero().position()))
            {
                hero_is_target = false;
                changed = !changed;
                if (bolt_vs_hero(bolt, start)) {
                    bolt_hit_something = true;
                }
            }

            //draw bolt
            int standout = 0;
            if (game->level().use_standout(bolt->position(), dirch)) {
                standout = 0x70;
            }
            if (bolt->is_frost() || bolt->is_ice())
                game->screen().set_attr(standout | 0x01);
            else if (bolt->is_lightning())
                game->screen().set_attr(standout | 0x0e);
            else
                game->screen().set_attr(standout | 0x04);
            tick_pause();
            game->screen().add_tile(bolt->position(), dirch);
            game->screen().standend();
        }
    }

    //restore ground
    for (j = 0; j < i; j++)
    {
        tick_pause();
        if (spotpos[j].s_under)
            game->screen().add_tile(spotpos[j].s_pos, spotpos[j].s_under);
    }

    return victim;
}

Monster* fire_bolt(Coord start, Coord *dir, const std::string& name) {
    bool from_player(start == game->hero().position());
    return fire_bolt(start, dir, new MagicBolt(name, from_player)); //todo:who owns memory?
}

//charge_str: Return an appropriate string for a wand charge
const char *get_charge_string(const Item *obj)
{
    static char buf[20];

    if (!obj->is_known() && !game->wizard().reveal_items()) buf[0] = '\0';
    else sprintf(buf, " [%d charges]", obj->charges());
    return buf;
}

//e.g. we'd crash if we started the hero off with a wand
Stick::Stick(const std::string& kind)
    : Item(STICK, 0)
{
    if (kind == "staff")
        m_damage = "2d3";  //mdk: A staff is more powerful than a wand for striking 
    else
        m_damage = "1d1";

    m_throw_damage = "1d1";
    m_charges = 3 + rnd(5);
}

std::string Stick::TypeName() const
{
    return Category()->kind();
}

std::string Stick::InventoryName() const
{
    std::ostringstream ss;

    ItemCategory& info = *Category();

    if (info.is_discovered() || game->wizard().reveal_items()) {
        ss << "A " << info.kind() << " of " << info.name() << get_charge_string(this);
        if (!short_msgs())
            ss << "(" << info.identifier() << ")";
    }
    else if (!info.guess().empty()) {
        ss << "A " << info.kind() << " called " << info.guess();
        if (!short_msgs())
            ss << "(" << info.identifier() << ")";
    }
    else
        ss << "A " << info.identifier() << " " << info.kind();

    return ss.str();
}

bool Stick::IsMagic() const
{
    return true;
}

bool Stick::IsEvil() const
{
    return false;
}

int Stick::Worth() const
{
    int worth = Category()->worth();
    worth += 20 * charges();
    if (!is_known())
        worth /= 2;
    return worth;
}

void Striking::drain_striking()
{
    if (--m_charges < 0) {
        m_damage = "0d0";
        m_hit_plus = 0;
        m_damage_plus = 0;
        m_charges = 0;
    }
}

void Striking::set_striking_damage()
{
    if (rnd(20) == 0) {
        m_damage = "3d8";
        m_damage_plus = 9;
    }
    else {
        m_damage = "2d8";
        m_damage_plus = 4;
    }
}

ItemCategory Light::info;
ItemCategory Striking::info;
ItemCategory Lightning::info;
ItemCategory Fire::info;
ItemCategory Cold::info;
ItemCategory Polymorph::info;
ItemCategory MagicMissileStick::info;
ItemCategory HasteMonster::info;
ItemCategory SlowMonster::info;
ItemCategory DrainLife::info;
ItemCategory Nothing::info;
ItemCategory TeleportAway::info;
ItemCategory TeleportTo::info;
ItemCategory Cancellation::info;
