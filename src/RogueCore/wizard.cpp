//Special wizard commands (some of which are also non-wizard commands under strange circumstances)
//wizard.c    1.4 (AI Design) 12/14/84

#include <sstream>
#include <iomanip>
#include <ctype.h>
#include <stdlib.h>

#include "rogue.h"
#include "random.h"
#include "game_state.h"
#include "daemons.h"
#include "pack.h"
#include "output_shim.h"
#include "io.h"
#include "wizard.h"
#include "daemon.h"
#include "main.h"
#include "things.h"
#include "rooms.h"
#include "misc.h"
#include "monsters.h"
#include "sticks.h"
#include "rings.h"
#include "scrolls.h"
#include "potions.h"
#include "weapons.h"
#include "level.h"
#include "armor.h"
#include "hero.h"
#include "food.h"
#include "scrolls.h"
#include "monster.h"
#include "amulet.h"

//whatis: What a certain object is
void whatis()
{
    if (game->hero().m_pack.empty()) {
        msg("You don't have anything in your pack to identify");
        return;
    }

    Item *obj;
    for (;;) {
        if ((obj = get_item("identify", 0)) == NULL)
        {
            msg("You must identify something");
            unsaved_msg(" ");
        }
        else
            break;
    }

    obj->discover();

    //If it is vorpally enchanted, then reveal what type of monster it is vorpally enchanted against
    obj->set_revealed();

    msg(obj->inventory_name(game->hero(), false).c_str());
}

int get_which(int type, int limit)
{
    byte ch;
    unsaved_msg("which %c do you want? (0-%x)", type, limit);
    int option = (isdigit((ch = readchar())) ? ch - '0' : ch - 'a' + 10);
    return (option >= 0 && option <= limit) ? option : 0;
}

char get_bless_char() {
    unsaved_msg("blessing? (+,-,n)");
    byte bless = readchar();
    return bless;
}

//summon_object: Wizard command for getting anything he wants
bool do_summon_object()
{
    int which;
    Item* obj;

    unsaved_msg("type of item !:%c ?:%c /:%c =:%c ):%c ]:%c ,:%c $:%c ", POTION, SCROLL, STICK, RING, WEAPON, ARMOR, AMULET, FOOD);
    switch (readchar())
    {
    case ESCAPE:
        msg("");
        return false;
    case '!':
        which = get_which(POTION, MAXPOTIONS - 1);
        obj = new Potion(which);
        break;
    case '?':
        which = get_which(SCROLL, MAXSCROLLS - 1);
        obj = new Scroll(which);
        break;
    case '/':
        which = get_which(STICK, MAXSTICKS - 1);
        obj = new Stick(which);
        break;
    case '=':
    {
        char bless;
        which = get_which(RING, MAXRINGS - 1);

        int level = 0;
        switch (which)
        {
        case R_PROTECT: case R_ADDSTR: case R_ADDHIT: case R_ADDDAM:
            bless = get_bless_char();
            level = (bless == '-') ? -1 : rnd(2) + 1;
            break;
        }

        obj = new Ring(which, level);
        break;
    }
    case ')':
    {
        which = get_which(WEAPON, MAXWEAPONS - 1);
        char bless = get_bless_char();
        int hit_plus = 0;
        if (bless == '-')
            hit_plus -= rnd(3) + 1;
        else if (bless == '+')
            hit_plus += rnd(3) + 1;

        obj = new Weapon(which, hit_plus, 0);
        break;
    }
    case ']':
    {
        which = get_which(ARMOR, MAXARMORS - 1);
        char bless = get_bless_char();

        int ac_mod = 0;
        if (bless == '-')
            ac_mod += rnd(3) + 1;
        else if (bless == '+')
            ac_mod -= rnd(3) + 1;

        obj = new Armor(which, ac_mod);
        break;
    }
    case ',':
        which = get_which(AMULET, 0);
        obj = new Amulet();
        break;
    default:
        which = get_which(FOOD, 1);
        obj = new Food(which);
        break;
    }

    game->hero().add_to_pack(obj, false);
    return false;
}

//add_pass: Add the passages to the current window (wizard command)
void add_pass()
{
    int y, x, ch;
    const int COLS = game->screen().columns();

    for (y = 1; y < maxrow(); y++)
        for (x = 0; x < COLS; x++)
            if ((ch = game->level().get_tile({ x, y })) == DOOR || ch == PASSAGE)
                game->screen().add_tile({ x, y }, ch);
}

bool do_show_map()
{
    game->level().show_map(true);
    game->level().illuminate_rooms();
    detect_monsters(true);
    return false;
}

//show_map: Print out the map for the wizard
void show_map(bool show_monsters)
{
    game->screen().wdump();
    game->screen().clear();

    const int COLS = game->screen().columns();
    for (int y = 1; y < maxrow(); y++) {
        for (int x = 0; x < COLS; x++) {
            Coord c = { x, y };
            byte tile = game->level().get_tile(c, show_monsters, false);
            int real = game->level().is_real(c);
            if (!real)
                game->screen().standout();
            game->screen().add_tile(c, tile);
            if (!real)
                game->screen().standend();
        }
    }

    show_win("---More (level map)---");
    game->screen().wrestor();
}

bool do_raise_level()
{
    game->hero().raise_level();
    return false;
}

bool do_toggle_detect()
{
    game->wizard().toggle_powers("detect_monsters");
    return false;
}

namespace
{
    using std::left;
    using std::setw;

    void add_debug_items(const std::list<Item*>& items, bool coord, const char* fmt)
    {
        for (auto it = items.begin(); it != items.end(); ++it)
        {
            Item* item = *it;
            std::ostringstream ss;

            Coord pos = item->position();
            ss << item->inventory_name(game->hero(), false);
            if (coord)
                ss << " at (" << pos.x << "," << pos.y << ")";

            add_line("", fmt, ss.str().c_str());
        }
    }

    void debug_items()
    {
        add_debug_items(game->level().items, true, "  %s");
    }

    void debug_monsters()
    {
        for (auto i = game->level().monsters.begin(); i != game->level().monsters.end(); ++i)
        {
            Monster* monster = *i;
            std::ostringstream ss;
            ss << (monster->is_running() ? "  R" : "   ");
            ss << " " << left << setw(14) << monster->get_name() << " ";
            ss << "hp:" << left << setw(4) << monster->get_hp() << " ";
            ss << "lvl:" << left << setw(3) << monster->level() << " ";
            ss << "amr:" << left << setw(4) << monster->calculate_armor() << " ";
            ss << "exp:" << left << setw(5) << monster->experience() << " ";
            ss << "dmg:" << left << setw(4) << monster->damage_string();
            add_line("", ss.str().c_str(), "");
            if (!monster->m_pack.empty()) {
                add_debug_items(monster->m_pack, false, "    * %s");
            }
        }
    }

    void debug_hero()
    {
        Coord pos = game->hero().position();
        std::ostringstream ss;
        ss << "Hero at (" << pos.x << "," << pos.y << ")";
        add_line("", ss.str().c_str(), "");
    }
}

void debug_screen()
{
    debug_hero();
    add_line("", " ", "");
    add_line("", "Level Items:", "");
    debug_items();
    add_line("", " ", "");
    add_line("", "Level Monsters:", "");
    debug_monsters();
    end_line("");
}

int get_num(short *place)
{
    char numbuf[12];

    getinfo(numbuf, 10);
    *place = atoi(numbuf);
    return (*place);
}

Cheats::Cheats()
{
}

void Cheats::toggle()
{
    m_enabled = !m_enabled;
    if (m_enabled)
        m_cheated = true;
    msg(m_enabled ? "You are now a wizard!" : "You feel your magic powers fade away");
    apply_powers();
}

bool Cheats::enabled() const
{
    return m_enabled;
}

bool Cheats::did_cheat() const
{
    return m_cheated;
}

void Cheats::add_powers(const std::string & powers)
{
    std::stringstream ss(powers);
    std::string item;
    while (getline(ss, item, ',')) {
        m_powers.insert(item);
    }
    if (enabled())
        apply_powers();
}

void Cheats::toggle_powers(const std::string & powers)
{
    std::stringstream ss(powers);
    std::string item;
    while (getline(ss, item, ',')) {
        auto i = m_powers.find(item);
        if (i != m_powers.end())
            m_powers.erase(i);
        else
            m_powers.insert(item);
    }
    if (enabled())
        apply_powers();
}

void Cheats::apply_powers()
{
    game->hero().m_invulnerable = (enabled() && invulnerability());
    ::detect_monsters(enabled() && (detect_monsters() || see_all()));
    if (enabled() && see_invisible())
        show_invisible();
    else
        unsee_invisible();
    if (enabled() && see_all())
        do_show_map();
}

void Cheats::on_new_level()
{
    apply_powers();
}

bool Cheats::is_enabled(const std::string& power) const
{
    return m_enabled && (m_powers.find(power) != m_powers.end());
}

bool Cheats::detect_monsters() const
{
    return is_enabled("detect_monsters");
}

bool Cheats::no_dark_rooms() const
{
    return is_enabled("no_dark_rooms");
}

bool Cheats::see_invisible() const
{
    return is_enabled("see_invisible");
}

bool Cheats::show_food_counter() const
{
    return is_enabled("show_food_counter");
}

bool Cheats::jump_levels() const
{
    return is_enabled("jump_levels");
}

bool Cheats::see_all() const
{
    return is_enabled("see_all");
}

bool Cheats::haste_self() const
{
    return is_enabled("haste_self");
}

bool Cheats::invulnerability() const
{
    return is_enabled("invulnerability");
}

bool Cheats::no_hunger() const
{
    return is_enabled("no_hunger");
}

bool Cheats::no_hidden_doors() const
{
    return is_enabled("no_hidden_doors");
}

bool Cheats::no_traps() const
{
    return is_enabled("no_traps");
}

bool Cheats::reveal_items() const
{
    return is_enabled("reveal_items");
}

bool Cheats::no_ring_hunger() const
{
    return is_enabled("no_ring_hunger");
}
