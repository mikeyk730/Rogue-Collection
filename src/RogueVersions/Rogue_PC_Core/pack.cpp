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

//do_call: Allow a user to call a potion, scroll, or ring something
bool do_call()
{
    Item *obj = game->hero().get_item("call", CALLABLE);
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
    msg("Was called \"%s\"", called.c_str());

    msg("what do you want to call it? ");
    getinfo(prbuf, MAXNAME);
    if (*prbuf && *prbuf != ESCAPE)
        obj->Category()->guess(prbuf);
    msg("");

    return false;
}
