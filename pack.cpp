//Routines to deal with the pack
//pack.c      1.4 (A.I. Design)       12/14/84

#include <stdio.h>

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

Item *pack_obj(byte ch, byte *chp)
{
    byte och = 'a';

    for (auto it = game->hero().pack.begin(); it != game->hero().pack.end(); ++it, och++) {
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
    for (auto it = game->hero().pack.begin(); it != game->hero().pack.end(); ++it, ch++)
    {
        Item* item = *it;
        //Don't print this one if: the type doesn't match the type we were passed AND it isn't a callable type AND it isn't a zappable weapon
        if (type && type != item->type &&
            !(type == CALLABLE && (item->type == SCROLL || item->type == POTION || item->type == RING || item->type == STICK)) &&
            !(type == WEAPON && item->type == POTION) &&
            !(type == STICK && item->is_vorpalized() && item->get_charges())) //todo: does this work?
            continue;
        n_objs++;
        sprintf(inv_temp, "%c) %%s", ch);
        add_line(lstr, inv_temp, item->inv_name(false));
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
    if (game->hero().pack.empty()) {
        msg("you aren't carrying anything");
        counts_as_turn = false; //mdk: previously, trying to do something with an empty pack would count as a turn
        return NULL;
    }

    //if we are doing something AGAIN, and the pack hasn't changed then don't ask just give him the same thing he got on the last command.
    if (repeat_last_action && purpose != "identify") {
        byte och = 0;
        Item* item = pack_obj(game->again_state.last_item_letter, &och);
        if (item == game->again_state.last_item_used) {
            return item;
        }
    }

    bool show_menu(game->get_environment("menu") == "on");
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

        msg_position = 0;
        show_menu = false;
        if (ch == '*')
        {
            //display the inventory and get a new selection
            ch = inventory(game->hero().pack, type, purpose.c_str());
            if (ch == 0) {
                counts_as_turn = false;
                return NULL;
            }
            if (ch == ' ') continue;
        }

        //Give the poor player a chance to abort the command
        if (ch == ESCAPE) {
            counts_as_turn = false;
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
                game->again_state.last_item_letter = ch;
                game->again_state.last_item_used = obj;
            }
            return obj;
        }
    }
}

//pack_char: Return which character would address a pack object
int pack_char(Item *obj)
{
    byte c = 'a';
    for (auto it = game->hero().pack.begin(); it != game->hero().pack.end(); ++it) {
        if (*it == obj)
            return c;
        else
            c++;
    }
    return '?';
}

//pick_up: Add something to characters pack.
void pick_up(byte ch)
{
    if (ch == GOLD)
    {
        Room* room = game->hero().room;
        Coord pos = game->hero().position();
        Item* obj = find_obj(pos);
        if (obj == NULL)
            return;

        game->hero().pick_up_gold(obj->get_gold_value());
        room->gold_val = 0;
        game->level().items.remove(obj);
        delete obj;

        byte floor = (room->is_gone()) ? PASSAGE : FLOOR;
        game->screen().mvaddch(pos, floor);
        game->level().set_tile(pos, floor);
    }
    else {
        game->hero().add_to_pack(NULL, false);
    }
}