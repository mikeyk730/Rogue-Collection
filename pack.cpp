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
int inventory(std::list<Item *>& list, int type, char *lstr)
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
Item* get_item(char *purpose, int type)
{
    Item *obj;
    byte ch;
    byte och;
    static byte lch;
    static Item *wasthing = NULL;
    byte gi_state; //get item sub state
    int once_only = false;

    if ("on" == game->get_environment("menu"))
        once_only = true;
    gi_state = again;
    if (game->hero().pack.empty())
        msg("you aren't carrying anything");
    else
    {
        ch = lch;
        for (;;)
        {
            //if we are doing something AGAIN, and the pack hasn't changed then don't ask just give him the same thing he got on the last command.
            if (gi_state && wasthing == pack_obj(ch, &och))
                goto skip;
            if (once_only) {
                ch = '*';
                goto skip;
            }
            if (!short_msgs())
                addmsg("which object do you want to ");
            msg("%s? (* for list): ", purpose);
            //ignore any alt characters that may be typed
            ch = readchar();
        skip:
            msg_position = 0;
            gi_state = false;
            once_only = false;
            if (ch == '*')
            {
                if ((ch = inventory(game->hero().pack, type, purpose)) == 0) {
                    counts_as_turn = false;
                    return NULL;
                }
                if (ch == ' ') continue;
                lch = ch;
            }
            //Give the poor player a chance to abort the command
            if (ch == ESCAPE) {
                counts_as_turn = false;
                msg("");
                return NULL;
            }
            if ((obj = pack_obj(ch, &och)) == NULL)
            {
                ifterse("range is 'a' to '%c'", "please specify a letter between 'a' and '%c'", och - 1);
                continue;
            }
            else
            {
                //If you find an object reset flag because you really don't know if the object he is getting is going to change the pack.  If he detaches the thing from the pack later this flag will get set.
                if (strcmp(purpose, "identify")) {
                    lch = ch;
                    wasthing = obj;
                }
                return obj;
            }
        }
    }
    return NULL;
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