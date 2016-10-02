//Contains functions for dealing with things like potions, scrolls, and other items.
//things.c     1.4 (AI Design) 12/14/84

#include <stdio.h>
#include <ctype.h>
#include <stdarg.h>

#include "rogue.h"
#include "game_state.h"
#include "things.h"
#include "pack.h"
#include "list.h"
#include "sticks.h"
#include "io.h"
#include "misc.h"
#include "daemons.h"
#include "rings.h"
#include "scrolls.h"
#include "potions.h"
#include "weapons.h"
#include "output_interface.h"
#include "main.h"
#include "armor.h"
#include "daemon.h"
#include "mach_dep.h"
#include "level.h"
#include "food.h"
#include "hero.h"

struct MagicItem things[NUMTHINGS] =
{
  {0, 27 }, //potion
  {0, 30 }, //scroll
  {0, 17 }, //food
  {0,  8 }, //weapon
  {0,  8 }, //armor
  {0,  5 }, //ring
  {0,  5 }  //stick
};

#define MAX(a,b,c,d) (a>b?(a>c?(a>d?a:d):(c>d?c:d)):(b>c?(b>d?b:d):(c>d?c:d)))
static short order[MAX(MAXSCROLLS, MAXPOTIONS, MAXRINGS, MAXSTICKS)]; //todo:eliminate static
static int line_cnt = 0; //todo:eliminate static

//init_things: Initialize the probabilities for types of things
void init_things()
{
    struct MagicItem *mp;
    for (mp = &things[1]; mp <= &things[NUMTHINGS - 1]; mp++)
        mp->prob += (mp - 1)->prob;
}

void Item::discover()
{
    set_known();
    if (item_class())
        item_class()->discover(m_which);
}

void Item::call_it()
{
    ItemClass* items = item_class();
    if (items) {
        items->call_it(m_which);
    }
}

ItemClass* Item::item_class() const
{
    //todo: change class layout, so we don't need to poke into game
    //this is problematic because we couldn't, for example, start
    //the hero off with a stick
    return game->item_class(m_type);
}

void chopmsg(char *s, char *shmsg, char *lnmsg, ...)
{
    va_list argptr;
    va_start(argptr, lnmsg);
    vsprintf(s, short_msgs() ? shmsg : lnmsg, argptr);
    va_end(argptr);
}

//do_drop: Put something down
bool do_drop()
{
    byte ch;
    Item *nobj, *op;

    ch = game->level().get_tile(game->hero().m_position);
    if (ch != FLOOR && ch != PASSAGE) {
        //mdk: trying to drop item into occupied space counts as turn
        msg("there is something there already");
        return true;
    }

    if ((op = get_item("drop", 0)) == NULL) 
        return false;

    if (!can_drop(op, true)) 
        return true;

    //Take it out of the pack
    if (op->m_count >= 2 && op->m_type != WEAPON)
    {
        op->m_count--;
        nobj = op->Clone();
        nobj->m_count = 1;
        op = nobj;
    }
    else
        game->hero().m_pack.remove(op);
    //Link it into the level object list
    game->level().items.push_front(op);
    op->m_position = game->hero().m_position;
    game->level().set_tile(op->m_position, op->m_type);
    msg("dropped %s", op->inventory_name(game->hero(), true).c_str());

    return true;
}

//can_drop: Do special checks for dropping or unweilding|unwearing|unringing
bool can_drop(Item *op, bool unequip)
{
    if (op == NULL)
        return true;
    if (op != game->hero().get_current_armor() && op != game->hero().get_current_weapon() &&
        op != game->hero().get_ring(LEFT) && op != game->hero().get_ring(RIGHT))
        return true;
    if (op->is_cursed()) {
        msg("you can't.  It appears to be cursed");
        return false;
    }
    if (!unequip)
        return true;

    if (op == game->hero().get_current_weapon()) {
        game->hero().set_current_weapon(NULL);
        return true;
    }

    else if (op == game->hero().get_current_armor()) {
        waste_time();  //mdk: taking off/dropping/throwing current armor takes two turns
        game->hero().set_current_armor(NULL);
        return true;
    }

    int hand;

    if (op != game->hero().get_ring(hand = LEFT)) {
        if (op != game->hero().get_ring(hand = RIGHT))
        {
            debug("Candrop called with funny thing");
            return true;
        }
    }
    game->hero().set_ring(hand, NULL);
    if (op->m_which == R_SEEINVIS) //todo: better place for this?  should be automatic
    {
        unsee();
        extinguish(unsee);
    }

    return true;
}

//new_thing: Return a new thing
Item* Item::CreateItem()
{
    //Decide what kind of object it will be. If we haven't had food for a while, let it be food.
    switch (game->no_food > 3 ? 2 : pick_one(things, NUMTHINGS))
    {
    case 0:
        return create_potion();
        break;

    case 1:
        return create_scroll();
        break;

    case 2:
        return Food::CreateFood();
        break;

    case 3:
        return create_weapon();
        break;

    case 4:
        return create_armor();
        break;

    case 5:
        return create_ring();
        break;

    case 6:
        return create_stick();
        break;

    default:
        debug("Picked a bad kind of object");
        wait_for(' ');
        break;
    }
    return 0;
}

//todo: remove this wrapper
int pick_one(std::vector<MagicItem> magic)
{
    MagicItem* a = new MagicItem[magic.size()];
    for (size_t i = 0; i < magic.size(); ++i)
        a[i] = magic[i];
    int r = pick_one(a, magic.size());
    delete[] a;
    return r;
}

//pick_one: Pick an item out of a list of nitems possible magic items
int pick_one(struct MagicItem *magic, int nitems)
{
    struct MagicItem *end;
    int i;
    struct MagicItem *start;

    start = magic;
    for (end = &magic[nitems], i = rnd(100); magic < end; magic++) if (i < magic->prob) break;
    if (magic == end)
    {
        debug("bad pick_one: %d from %d items", i, nitems);
        for (magic = start; magic < end; magic++)
            debug("%s: %d%%", magic->name, magic->prob);

        magic = start;
    }
    return magic - start;
}

//do_discovered: list what the player has discovered in this game of a certain type
bool do_discovered()
{
    if (game->wizard().enabled()) {
        debug_screen();
        return false;
    }

    print_disc(POTION);
    add_line("", " ", 0);
    print_disc(SCROLL);
    add_line("", " ", 0);
    print_disc(RING);
    add_line("", " ", 0);
    print_disc(STICK);
    end_line("");

    return false;
}

//print_disc: Print what we've discovered of type 'type'
void print_disc(byte type)
{
    ItemClass* items = game->item_class(type);
    int maxnum = items->get_max_items();

    set_order(order, maxnum);
    int num_found = 0;
    for (int i = 0; i < maxnum; i++) {
        if (items->is_discovered(order[i]) || !items->get_guess(order[i]).empty())
        {
            std::string line = items->get_inventory_name(order[i]);
            add_line("", "%s", line.c_str());
            num_found++;
        }
    }
    if (num_found == 0)
        add_line("", nothing(type), 0);
}

//set_order: Set up order for list
void set_order(short *order, int numthings)
{
    int i, r, t;

    for (i = 0; i < numthings; i++) order[i] = i;
    for (i = numthings; i > 0; i--)
    {
        r = rnd(i);
        t = order[i - 1];
        order[i - 1] = order[r];
        order[r] = t;
    }
}

//add_line: Add a line to the list of discoveries
int add_line(const char *use, const char *fmt, const char *arg)
{
    int x, y;
    int retchar = ' ';
    const int LINES = game->screen().lines();

    if (line_cnt == 0) {
        game->screen().wdump();
        game->screen().clear();
    }
    if (line_cnt >= LINES - 1 || fmt == NULL)
    {
        game->screen().move(LINES - 1, 0);
        if (*use)
            game->screen().printw("-Select item to %s. Esc to cancel-", use);
        else
            game->screen().addstr("-Press space to continue-");
        do retchar = readchar(); while (retchar != ESCAPE && retchar != ' ' && (!islower(retchar)));
        game->screen().clear();
        line_cnt = 0;
    }
    if (fmt != NULL && !(line_cnt == 0 && *fmt == '\0'))
    {
        game->screen().move(line_cnt, 0);
        game->screen().printw(fmt, arg);
        game->screen().getrc(&x, &y);
        //if the line wrapped but nothing was printed on this line you might as well use it for the next item
        if (y != 0) line_cnt = x + 1;
    }
    return (retchar);
}

//end_line: End the list of lines
int end_line(const char *use)
{
    int retchar;

    retchar = add_line(use, 0, 0);
    game->screen().wrestor();
    line_cnt = 0;
    return (retchar);
}

//nothing: Set up prbuf so that message for "nothing found" is there
char *nothing(byte type)
{
    char *sp, *tystr;

    sprintf(prbuf, "Haven't discovered anything");
    if (in_small_screen_mode()) sprintf(prbuf, "Nothing");
    sp = &prbuf[strlen(prbuf)];
    switch (type)
    {
    case POTION: tystr = "potion"; break;
    case SCROLL: tystr = "scroll"; break;
    case RING: tystr = "ring"; break;
    case STICK: tystr = "stick"; break;
    }
    sprintf(sp, " about any %ss", tystr);
    return prbuf;
}
