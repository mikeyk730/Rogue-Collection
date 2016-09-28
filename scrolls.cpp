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
#include "output_interface.h"
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
        m_identifier.push_back(prbuf);

        if (i > 0)
            m_magic_props[i].prob += m_magic_props[i - 1].prob;
    }
}

Item* create_scroll()
{
    int which = pick_one(game->scrolls().m_magic_props);
    return new Scroll(which);
}

void read_monster_confusion()
{
    //Scroll of monster confusion.  Give him that power.
    game->hero().set_can_confuse(true);
    msg("your hands begin to glow red");
}

void read_magic_mapping()
{
    //Scroll of magic mapping.
    int x, y;
    byte ch;
    Monster* monster;

    game->scrolls().discover(S_MAP);
    msg("oh, now this scroll has a map on it");
    //Take all the things we want to keep hidden out of the window
    const int COLS = game->screen().columns();
    for (y = 1; y < maxrow(); y++) for (x = 0; x < COLS; x++)
    {
        switch (ch = game->level().get_tile({ x, y }))
        {
        case VWALL: case HWALL: case ULWALL: case URWALL: case LLWALL: case LRWALL:
            if (!(game->level().get_flags({ x, y })&F_REAL)) {
                ch = DOOR;
                game->level().set_tile({ x, y }, DOOR);
                game->level().unset_flag({ x, y }, F_REAL);
            }
        case DOOR: case PASSAGE: case STAIRS:
            if ((monster = game->level().monster_at({ x, y })) != NULL)
                if (monster->oldch == ' ')
                    monster->oldch = ch;
            break;
        default: ch = ' ';
        }
        if (ch == DOOR)
        {
            game->screen().move(y, x);
            if (game->screen().curch() != DOOR)
                game->screen().standout();
        }
        if (ch != ' ')
            game->screen().mvaddch({ x, y }, ch);
        game->screen().standend();
    }
}

void read_hold_monster()
{
    //Hold monster scroll.  Stop all monsters within two spaces from chasing after the hero.
    int x, y;
    Agent* monster;

    const int COLS = game->screen().columns();
    for (x = game->hero().pos.x - 3; x <= game->hero().pos.x + 3; x++) {
        if (x >= 0 && x < COLS) {
            for (y = game->hero().pos.y - 3; y <= game->hero().pos.y + 3; y++) {
                if ((y > 0 && y < maxrow()) && ((monster = game->level().monster_at({ x, y })) != NULL))
                {
                    monster->set_running(false);
                    monster->set_is_held(true);
                }
            }
        }
    }
}

void read_sleep()
{
    //Scroll which makes you fall asleep
    game->scrolls().discover(S_SLEEP);
    game->sleep_timer += rnd(SLEEP_TIME) + 4;
    game->hero().set_running(false);
    msg("you fall asleep");
}

void read_enchant_armor()
{
    if (game->hero().get_current_armor() != NULL)
    {
        game->hero().get_current_armor()->enchant_armor();
        ifterse("your armor glows faintly", "your armor glows faintly for a moment");
    }
}

void read_identify()
{
    //Identify, let the rogue figure something out
    game->scrolls().discover(S_IDENT);
    msg("this scroll is an identify scroll");
    if ("on" == game->get_environment("menu"))
        more(" More ");
    whatis();
}

void read_scare_monster()
{
    //Reading it is a mistake and produces laughter at the poor rogue's boo boo.
    msg(laugh, short_msgs() ? "" : in_dist);
}

void read_food_detection()
{
    //Scroll of food detection
    byte discover = false;

    for (auto it = game->level().items.begin(); it != game->level().items.end(); ++it)
    {
        Item* item = *it;
        if (item->type == FOOD)
        {
            discover = true;
            game->screen().standout();
            game->screen().mvaddch(item->pos, FOOD);
            game->screen().standend();
        }
        //as a bonus this will detect amulets as well
        else if (item->type == AMULET)
        {
            discover = true;
            game->screen().standout();
            game->screen().mvaddch(item->pos, AMULET);
            game->screen().standend();
        }
    }
    if (discover) {
        game->scrolls().discover(S_GFIND);
        msg("your nose tingles as you sense food");
    }
    else
        ifterse("you hear a growling noise close by", "you hear a growling noise very close to you");
}

void read_teleportation()
{
    //Scroll of teleportation: Make him disappear and reappear
    Room* original_room = game->hero().room;
    game->hero().teleport();
    if (original_room != game->hero().room)
        game->scrolls().discover(S_TELEP);
}

void read_enchant_weapon()
{
    Item* weapon = game->hero().get_current_weapon();
    if (weapon == NULL || weapon->type != WEAPON) {
        msg("you feel a strange sense of loss");
        return;
    }
    weapon->enchant_weapon();
}

void read_create_monster()
{
    Agent* monster;
    Coord position;

    if (plop_monster(game->hero().pos.y, game->hero().pos.x, &position)) {
        monster = Monster::CreateMonster(randmonster(false, get_level()), &position, get_level());
    }
    else
        ifterse("you hear a faint cry of anguish", "you hear a faint cry of anguish in the distance");
}

void read_remove_curse()
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

void read_aggravate_monsters()
{
    //This scroll aggravates all the monsters on the current level and sets them running towards the hero
    aggravate();
    ifterse("you hear a humming noise", "you hear a high pitched humming noise");
}

void read_blank_paper()
{
    msg("this scroll seems to be blank");
}

void read_vorpalize_weapon()
{
    //If he isn't wielding a weapon I get to chortle again!
    Item* weapon = game->hero().get_current_weapon();
    if (!weapon || weapon->type != WEAPON) {
        msg(laugh, short_msgs() ? "" : in_dist);
        return;
    }
    weapon->vorpalize();
}

void(*scroll_functions[MAXSCROLLS])() =
{
  read_monster_confusion,
  read_magic_mapping,
  read_hold_monster,
  read_sleep,
  read_enchant_armor,
  read_identify,
  read_scare_monster,
  read_food_detection,
  read_teleportation,
  read_enchant_weapon,
  read_create_monster,
  read_remove_curse,
  read_aggravate_monsters,
  read_blank_paper,
  read_vorpalize_weapon
};

//read_scroll: Read a scroll from the pack and do the appropriate thing
void read_scroll()
{
    Item *scroll;

    scroll = get_item("read", SCROLL);
    if (scroll == NULL) return;
    if (scroll->type != SCROLL) { msg("there is nothing on it to read"); return; }
    ifterse("the scroll vanishes", "as you read the scroll, it vanishes");
    if (scroll == game->hero().get_current_weapon())
        game->hero().set_current_weapon(NULL);

    //Call the function for this scroll
    if (scroll->which >= 0 && scroll->which < MAXSCROLLS)
        scroll_functions[scroll->which]();
    else {
        msg("what a puzzling scroll!");
        return;
    }

    look(true); //put the result of the scroll on the screen
    status();
    game->scrolls().call_it(scroll->which);

    //Get rid of the thing
    if (scroll->count > 1)
        scroll->count--;
    else {
        game->hero().pack.remove(scroll);
        delete(scroll);
    }
}

int is_scare_monster_scroll(Item* item)
{
    return item && item->type == SCROLL &&
        item->which == S_SCARE;
}

int is_bad_scroll(Item* item)
{
    return item && item->type == SCROLL &&
        (item->which == S_SLEEP || item->which == S_CREATE || item->which == S_AGGR);
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
    if (is_discovered(which) || game->hero().is_wizard())
        sprintf(pb, "of %s", get_name(which).c_str());
    else if (!get_guess(which).empty())
        sprintf(pb, "called %s", get_guess(which).c_str());
    else
        chopmsg(pb, "titled '%.17s'", "titled '%s'", get_identifier(which).c_str());

    return prbuf;
}

std::string ScrollInfo::get_inventory_name(Item * obj) const
{
    return get_inventory_name(obj->which, obj->count);
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
