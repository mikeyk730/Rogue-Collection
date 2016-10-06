//Read a scroll and let it happen
//scrolls.c   1.4 (AI Design) 12/14/84

#include <stdio.h>

#include "rogue.h"
#include "item_class.h"
#include "game_state.h"
#include "scrolls.h"
#include "monsters.h"
#include "pack.h"
#include "list.h"
#include "curses.h"
#include "io.h"
#include "main.h"
#include "misc.h"
#include "wizard.h"
#include "slime.h"
#include "level.h"
#include "weapons.h"
#include "things.h"
#include "hero.h"
#include "monster.h"
#include "rings.h"
#include "armor.h"

//Scroll types
#define S_CONFUSE   0
#define S_MAP       1
#define S_HOLD      2
#define S_SLEEP     3
#define S_ARMOR     4
#define S_IDENT     5
#define S_SCARE     6
#define S_GFIND     7
#define S_TELEP     8
#define S_ENCH      9
#define S_CREATE    10
#define S_REMOVE    11
#define S_AGGR      12
#define S_NOP       13
#define S_VORPAL    14

const char *c_set = "bcdfghjklmnpqrstvwxyz";
const char *v_set = "aeiou";

const char *laugh = "you hear maniacal laughter%s.";
const char *in_dist = " in the distance";

//random_char_in(): return random character in given string
char random_char_in(const char *string)
{
    return (string[rnd(strlen(string))]);
}

//getsyl(): generate a random syllable
char* getsyl()
{
    static char _tsyl[4];

    _tsyl[3] = 0;
    _tsyl[2] = random_char_in(c_set);
    _tsyl[1] = random_char_in(v_set);
    _tsyl[0] = random_char_in(c_set);
    return (_tsyl);
}

ScrollInfo::ScrollInfo()
{
    m_magic_props = {
        { "monster confusion",   8, 140 },
        { "magic mapping",       5, 150 },
        { "hold monster",        3, 180 },
        { "sleep",               5,   5 },
        { "enchant armor",       8, 160 },
        { "identify",           27, 100 },
        { "scare monster",       4, 200 },
        { "food detection",      4,  50 },
        { "teleportation",       7, 165 },
        { "enchant weapon",     10, 150 },
        { "create monster",      5,  75 },
        { "remove curse",        8, 105 },
        { "aggravate monsters",  4,  20 },
        { "blank paper",         1,   5 },
        { "vorpalize weapon",    1, 300 }
    };

    int nsyl;
    char *cp, *sp;
    int i, nwords;

    for (i = 0; i < MAXSCROLLS; i++)
    {
        cp = prbuf;
        nwords = rnd(in_small_screen_mode() ? 3 : 4) + 2;
        while (nwords--)
        {
            nsyl = rnd(2) + 1;
            while (nsyl--)
            {
                sp = getsyl();
                if (&cp[strlen(sp)] > &prbuf[MAXNAME - 1]) { nwords = 0; break; }
                while (*sp) *cp++ = *sp++;
            }
            *cp++ = ' ';
        }
        *--cp = '\0';
        //I'm tired of thinking about this one so just in case .....
        prbuf[MAXNAME] = 0;
        std::string scroll_name(prbuf);
        if (scroll_name.back() == ' ')
            scroll_name.pop_back();
        m_identifier.push_back(scroll_name);

        if (i > 0)
            m_magic_props[i].prob += m_magic_props[i - 1].prob;
    }
}

Item* create_scroll()
{
    int which = pick_one(game->scrolls().m_magic_props);
    return new Scroll(which);
}

void Scroll::read_monster_confusion()
{
    //Scroll of monster confusion.  Give him that power.
    game->hero().set_can_confuse(true);
    msg("your hands begin to glow red");
}

void Scroll::read_magic_mapping()
{
    //Scroll of magic mapping.
    discover();
    msg("oh, now this scroll has a map on it");
    game->level().show_map(false);
}

void Scroll::read_hold_monster()
{
    //Hold monster scroll.  Stop all monsters within two spaces from chasing after the hero.
    int x, y;
    Monster* monster;

    const int COLS = game->screen().columns();
    for (x = game->hero().position().x - 3; x <= game->hero().position().x + 3; x++) {
        if (x >= 0 && x < COLS) {
            for (y = game->hero().position().y - 3; y <= game->hero().position().y + 3; y++) {
                if ((y > 0 && y < maxrow()) && ((monster = game->level().monster_at({ x, y })) != NULL))
                {
                    monster->hold();
                }
            }
        }
    }
}

void Scroll::read_sleep()
{
    //Scroll which makes you fall asleep
    discover();
    game->hero().increase_sleep_turns(rnd(SLEEP_TIME) + 4);
    msg("you fall asleep");
}

void Scroll::read_enchant_armor()
{
    if (game->hero().get_current_armor() != NULL)
    {
        game->hero().get_current_armor()->enchant_armor();
        ifterse("your armor glows faintly", "your armor glows faintly for a moment");
    }
}

void Scroll::read_identify()
{
    //Identify, let the rogue figure something out
    discover();
    msg("this scroll is an identify scroll");
    if (game->options.show_inventory_menu())
        more(" More ");
    whatis();
}

void Scroll::read_scare_monster()
{
    //Reading it is a mistake and produces laughter at the poor rogue's boo boo.
    msg(laugh, short_msgs() ? "" : in_dist);
}

void Scroll::read_food_detection()
{
    //Scroll of food detection
    byte discovered = false;

    for (auto it = game->level().items.begin(); it != game->level().items.end(); ++it)
    {
        Item* item = *it;
        if (item->m_type == FOOD)
        {
            discovered = true;
            game->screen().standout();
            game->screen().mvaddch(item->position(), FOOD);
            game->screen().standend();
        }
        //as a bonus this will detect amulets as well
        else if (item->m_type == AMULET)
        {
            discovered = true;
            game->screen().standout();
            game->screen().mvaddch(item->position(), AMULET);
            game->screen().standend();
        }
    }
    if (discovered) {
        discover();
        msg("your nose tingles as you sense food");
    }
    else
        ifterse("you hear a growling noise close by", "you hear a growling noise very close to you");
}

void Scroll::read_teleportation()
{
    //Scroll of teleportation: Make him disappear and reappear
    Room* original_room = game->hero().room();
    game->hero().teleport();
    if (original_room != game->hero().room())
        discover();
}

void Scroll::read_enchant_weapon()
{
    Item* item = game->hero().get_current_weapon();
    Weapon* weapon = dynamic_cast<Weapon*>(item);
    if (!weapon) {
        msg("you feel a strange sense of loss");
        return;
    }
    weapon->enchant_weapon();
}

void Scroll::read_create_monster()
{
    Agent* monster;
    Coord position;

    if (plop_monster(game->hero().position().y, game->hero().position().x, &position)) {
        monster = Monster::CreateMonster(randmonster(false, game->get_level()), &position, game->get_level());
    }
    else
        ifterse("you hear a faint cry of anguish", "you hear a faint cry of anguish in the distance");
}

void Scroll::read_remove_curse()
{
    if (game->hero().get_current_armor())
        game->hero().get_current_armor()->remove_curse();
    if (game->hero().get_current_weapon())
        game->hero().get_current_weapon()->remove_curse();
    if (game->hero().get_ring(LEFT))
        game->hero().get_ring(LEFT)->remove_curse();
    if (game->hero().get_ring(RIGHT))
        game->hero().get_ring(RIGHT)->remove_curse();

    ifterse("somebody is watching over you", "you feel as if somebody is watching over you");
}

void Scroll::read_aggravate_monsters()
{
    //This scroll aggravates all the monsters on the current level and sets them running towards the hero
    game->level().aggravate_monsters();
    ifterse("you hear a humming noise", "you hear a high pitched humming noise");
}

void Scroll::read_blank_paper()
{
    msg("this scroll seems to be blank");
}

void Scroll::read_vorpalize_weapon()
{
    //If he isn't wielding a weapon I get to chortle again!
    Item* item = game->hero().get_current_weapon();
    Weapon* weapon = dynamic_cast<Weapon*>(item);
    if (!weapon) {
        msg(laugh, short_msgs() ? "" : in_dist);
        return;
    }
    weapon->vorpalize();
}

void(Scroll::*scroll_functions[MAXSCROLLS])() =
{
  &Scroll::read_monster_confusion,
  &Scroll::read_magic_mapping,
  &Scroll::read_hold_monster,
  &Scroll::read_sleep,
  &Scroll::read_enchant_armor,
  &Scroll::read_identify,
  &Scroll::read_scare_monster,
  &Scroll::read_food_detection,
  &Scroll::read_teleportation,
  &Scroll::read_enchant_weapon,
  &Scroll::read_create_monster,
  &Scroll::read_remove_curse,
  &Scroll::read_aggravate_monsters,
  &Scroll::read_blank_paper,
  &Scroll::read_vorpalize_weapon
};

//do_read_scroll: Read a scroll from the pack and do the appropriate thing
bool do_read_scroll()
{
    Item *item = get_item("read", SCROLL);
    if (!item)
        return false;

    Scroll* scroll = dynamic_cast<Scroll*>(item);
    if (!scroll) {
        //mdk: reading non-scroll counts as turn
        msg("there is nothing on it to read"); 
        return true;
    }

    ifterse("the scroll vanishes", "as you read the scroll, it vanishes");
    if (scroll == game->hero().get_current_weapon())
        game->hero().set_current_weapon(NULL);

    //Call the function for this scroll
    if (scroll->m_which >= 0 && scroll->m_which < MAXSCROLLS)
        (scroll->*scroll_functions[scroll->m_which])();
    else {
        msg("what a puzzling scroll!");
        return true;
    }

    look(true); //put the result of the scroll on the screen
    update_status_bar();
    scroll->call_it();

    //Get rid of the thing
    if (scroll->m_count > 1)
        scroll->m_count--;
    else {
        game->hero().m_pack.remove(scroll);
        delete(scroll);
    }
    return true;
}

int is_scare_monster_scroll(Item* item)
{
    return item && item->m_type == SCROLL &&
        item->m_which == S_SCARE;
}

std::string ScrollInfo::get_inventory_name(int which, int count) const
{
    char *pb = prbuf;

    if (count == 1) {
        strcpy(pb, "A scroll ");
        pb = &prbuf[9];
    }
    else {
        sprintf(pb, "%d scrolls ", count);
        pb = &prbuf[strlen(prbuf)];
    }
    if (is_discovered(which) || game->wizard().reveal_items())
        sprintf(pb, "of %s", get_name(which).c_str());
    else if (!get_guess(which).empty())
        sprintf(pb, "called %s", get_guess(which).c_str());
    else
        chopmsg(pb, "titled '%.17s'", "titled '%s'", get_identifier(which).c_str());

    return prbuf;
}

std::string ScrollInfo::get_inventory_name(const Item * obj) const
{
    return get_inventory_name(obj->m_which, obj->m_count);
}

std::string ScrollInfo::get_inventory_name(int which) const
{
    return get_inventory_name(which, 1);
}

Scroll::Scroll(int which) :
    Item(SCROLL, which)
{
}

Item * Scroll::Clone() const
{
    return new Scroll(*this);
}

std::string Scroll::Name() const
{
    return "scroll";
}

std::string Scroll::InventoryName() const
{
    return item_class()->get_inventory_name(this);
}

bool Scroll::IsMagic() const
{
    return true;
}

bool Scroll::IsEvil() const
{
    return (m_which == S_SLEEP || m_which == S_CREATE || m_which == S_AGGR);

}

int Scroll::Worth() const
{
    int worth = item_class()->get_value(m_which);
    worth *= m_count;
    if (!item_class()->is_discovered(m_which))
        worth /= 2;
    return worth;
}
