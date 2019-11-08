//Routines to deal with the pack
//pack.c      1.4 (A.I. Design)       12/14/84
#include <stdio.h>
#include <sstream>
#include "random.h"
#include "game_state.h"
#include "pack.h"
#include "io.h"
#include "misc.h"
#include "output_shim.h"
#include "list.h"
#include "things.h"
#include "level.h"
#include "hero.h"
#include "room.h"
#include "monster.h"
#include "weapons.h"
#include "gold.h"
#include "item_category.h"
#include "text.h"

#define CALLABLE  -1

Item *pack_obj(byte ch, byte *chp)
{
    byte och = 'a';

    for (auto it = game->hero().m_pack.begin(); it != game->hero().m_pack.end(); ++it, och++) {
        if (ch == och)
            return *it;
    }
    *chp = och;
    return NULL;
}

//inventory: List what is in the pack
int inventory(std::list<Item *>& list, int type, const char *lstr)
{
    byte ch = 'a';
    int n_objs;
    char inv_temp[MAXSTR];

    n_objs = 0;
    for (auto it = game->hero().m_pack.begin(); it != game->hero().m_pack.end(); ++it, ch++)
    {
        Item* item = *it;
        Weapon* weapon = dynamic_cast<Weapon*>(item);
        //Don't print this one if: the type doesn't match the type we were passed AND it isn't a callable type AND it isn't a zappable weapon
        if (type && type != item->m_type &&
            !(type == CALLABLE && (item->m_type == SCROLL || item->m_type == POTION || item->m_type == RING || item->m_type == STICK)) &&
            !(type == WEAPON && item->m_type == POTION) && //show potions when wielding
            !(type == STICK && weapon && weapon->is_vorpalized() && item->charges())) //show vorpalized weapon when zapping
            continue;
        n_objs++;
        sprintf(inv_temp, "%c) %%s", ch);
        if (game->options.use_slow_inventory())
            msg(inv_temp, item->inventory_name(game->hero(), false).c_str());
        else
            add_line(lstr, inv_temp, item->inventory_name(game->hero(), false).c_str());
    }
    if (n_objs == 0)
    {
        msg(type == 0 ? "you are empty handed" : "you don't have anything appropriate");
        return false;
    }

    return game->options.use_slow_inventory() ? ' ' : end_line(lstr);
}

void single_inventory(std::list<Item*>& list)
{
    if (list.size() == 0) {
        msg("You aren't carrying anything");
        return;
    }

    char selection = 'a';

    if (list.size() > 1) {
        msg(short_msgs() ? "Item: " : "Which item do you wish to inventory: ");
        selection = readchar();
        clear_msg();

        if (selection == ESCAPE)
        {
            return;
        }
    }

    char ch = 'a';
    for (std::list<Item*>::iterator i = list.begin(); i != list.end(); ++i)
    {
        if (ch == selection)
        {
            msg("%c) %s", ch, (*i)->inventory_name(game->hero(), false).c_str());
            return;
        }

        ++ch;
    }

    if (!short_msgs())
        msg("'%s' not in pack", unctrl(selection));
    msg("Range is 'a' to '%c'", --ch);
}

//get_item: Pick something out of a pack for a purpose
Item* get_item(const std::string& purpose, int type)
{
    if (game->hero().m_pack.empty()) {
        //mdk:bugfix: Originally, trying to do something with an empty pack would count as a turn
        msg("you aren't carrying anything");
        return NULL;
    }

    //if we are doing something AGAIN, and the pack hasn't changed then don't ask just give him the same thing he got on the last command.
    if (game->repeat_last_action && purpose != "identify") {
        byte och = 0;
        Item* item = pack_obj(game->last_turn.item_letter, &och);
        if (item == game->last_turn.item_used) {
            return item;
        }
    }

    bool show_menu(game->options.show_inventory_menu());
    for (;;)
    {
        byte ch;
        if (show_menu) {
            ch = '*';
        }
        else {
            msg(get_text(text_use), purpose.c_str());
            ch = readchar();
        }

        reset_msg_position();
        show_menu = false;
        if (ch == '*')
        {
            //display the inventory and get a new selection
            ch = inventory(game->hero().m_pack, type, purpose.c_str());
            if (ch == 0) {
                return NULL;
            }
            if (ch == ' ') continue;
        }

        //Give the poor player a chance to abort the command
        if (ch == ESCAPE) {
            msg("");
            return NULL;
        }

        byte och = 0;
        Item *obj = pack_obj(ch, &och);
        if (!obj)
        {
            msg(get_text(text_inventory_prompt), och - 1);
            continue;
        }
        else
        {
            if (purpose != "identify") {
                game->last_turn.item_letter = ch;
                game->last_turn.item_used = obj;
            }
            return obj;
        }
    }
}

//pack_char: Return which character would address a pack object
int pack_char(Item *obj)
{
    byte c = 'a';
    for (auto it = game->hero().m_pack.begin(); it != game->hero().m_pack.end(); ++it) {
        if (*it == obj)
            return c;
        else
            c++;
    }
    return '?';
}

//do_call: Allow a user to call a potion, scroll, or ring something
bool do_call()
{
    Item *obj = get_item("call", CALLABLE);
    if (!obj)
        return false;

    if (!obj->Category()) {
        msg("you can't call that anything");
        return false;
    }

    if (obj->Category()->is_discovered()) {
        msg("that has already been identified");
        return false;
    }

    std::string called = obj->Category()->guess();
    if (called.empty())
        called = obj->Category()->identifier();
    msg(get_text(text_was_called), called.c_str());

    msg(get_text(text_call_it));
    getinfo(prbuf, MAXNAME);
    if (*prbuf && *prbuf != ESCAPE)
        obj->Category()->guess(prbuf);
    msg("");

    return false;
}


//pick_up: Add something to characters pack.
void Hero::pick_up(byte ch)
{
    //mdk:bugfix: this code used to be inside add_to_pack, so it wasn't run when picking up
    //gold. The result was a dangling m_destination pointer when you stole a monster's
    //gold.  This could cause a crash.
    for (auto it = game->level().monsters.begin(); it != game->level().monsters.end(); ++it) {
        Monster* monster = *it;

        //If this was the object of something's desire, that monster will get mad and run at the hero
        if (monster->is_going_to(position())) {
            std::ostringstream ss;
            ss << monster->get_name() << " " << monster->position() << " item may be taken";
            game->log("monster", ss.str());
            monster->set_destination(this);
        }
    }

    if (ch == GOLD)
    {
        Item* obj = find_obj(position(), true);
        Gold* gold = dynamic_cast<Gold*>(obj);
        if (gold == NULL)
            return;

        pick_up_gold(gold->get_gold_value());
        room()->m_gold_val = 0;
        game->level().items.remove(gold);
        delete gold;

        byte floor = (room()->is_gone()) ? PASSAGE : FLOOR;
        game->screen().add_tile(position(), floor);
        game->level().set_tile(position(), floor);
    }
    else {
        add_to_pack(NULL, false);
    }
}