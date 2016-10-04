//Routines to deal with the pack
//pack.c      1.4 (A.I. Design)       12/14/84

#include <stdio.h>
#include <sstream>

#include "rogue.h"
#include "game_state.h"
#include "pack.h"
#include "io.h"
#include "misc.h"
#include "output_interface.h"
#include "list.h"
#include "things.h"
#include "mach_dep.h"
#include "level.h"
#include "hero.h"
#include "room.h"
#include "monster.h"
#include "weapons.h"
#include "gold.h"

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
        add_line(lstr, inv_temp, item->inventory_name(game->hero(), false).c_str());
    }
    if (n_objs == 0)
    {
        msg(type == 0 ? "you are empty handed" : "you don't have anything appropriate");
        return false;
    }
    return (end_line(lstr));
}

//get_item: Pick something out of a pack for a purpose
Item* get_item(const std::string& purpose, int type)
{
    if (game->hero().m_pack.empty()) {
        //mdk:bugfix: previously, trying to do something with an empty pack would count as a turn
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
            if (!short_msgs())
                addmsg("which object do you want to ");
            msg("%s? (* for list): ", purpose.c_str());
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
            ifterse("range is 'a' to '%c'", "please specify a letter between 'a' and '%c'", och - 1);
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

//pick_up: Add something to characters pack.
void Hero::pick_up(byte ch)
{
    //If this was the object of something's desire, that monster will get mad and run at the hero
    for (auto it = game->level().monsters.begin(); it != game->level().monsters.end(); ++it) {
        Monster* monster = *it;
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
        game->screen().mvaddch(position(), floor);
        game->level().set_tile(position(), floor);
    }
    else {
        add_to_pack(NULL, false);
    }
}