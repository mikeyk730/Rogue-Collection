#include "commands.h"
#include "random.h"
#include "game_state.h"
#include "hero.h"
#include "pack.h"
#include "misc.h"
#include "io.h"
#include "level.h"
#include "daemons.h"

bool do_inventory()
{
    game->hero().inventory(0, "");
    return false;
}

bool do_eat()
{
    return game->hero().eat();
}

bool do_wield()
{
    return game->hero().wield();
}

bool do_wear_armor()
{
    return game->hero().wear_armor();
}

bool do_take_off_armor()
{
    return game->hero().take_off_armor();
}

bool do_put_on_ring()
{
    return game->hero().put_on_ring();
}

bool do_remove_ring()
{
    return game->hero().remove_ring();
}

bool do_object_help()
{
    help(helpobjs);
    return false;
}

bool do_command_help()
{
    help(helpcoms);
    return false;
}

bool do_toggle_terse()
{
    bool new_value = !in_brief_mode();
    set_brief_mode(new_value);
    msg(new_value ? "Ok, I'll be brief" : "Goodie, I can use big words again!");
    return false;
}

bool do_play_macro()
{
    std::string macro = game->macro;
    std::reverse(macro.begin(), macro.end());
    game->typeahead = macro;
    return false;
}

bool do_id_trap()
{
    Coord d;
    if (get_dir(&d))
    {
        Coord lookat = game->hero().position() + d;
        if (game->level().get_tile(lookat) != TRAP)
            msg("no trap there.");
        else 
            msg("you found %s", tr_name(game->level().get_trap_type(lookat)));
    }
    return false;
}

bool do_options()
{
    msg("i don't have any options, oh my!");
    return false;
}

bool do_clear_screen()
{
    msg("the screen looks fine to me");
    return false;
}

bool do_repeat_msg()
{
    msg(game->last_message);
    return false;
}

bool do_print_version()
{
    if (game->options.act_like_v1_1())
        msg("Rogue version %d.%d (acting like v1.1)", REV, VER);
    else
        msg("Rogue version %d.%d", REV, VER);

    return false;
}

bool do_rest()
{
    doctor();
    return true;
}
