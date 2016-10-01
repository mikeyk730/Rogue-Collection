//Read and execute the user commands
//command.c   1.44    (A.I. Design)   2/14/85

#include <ctype.h>
#include "rogue.h"
#include "game_state.h"
#include "daemons.h"
#include "daemon.h"
#include "command.h"
#include "main.h"
#include "io.h"
#include "wizard.h"
#include "misc.h"
#include "fakedos.h"
#include "save.h"
#include "armor.h"
#include "weapons.h"
#include "sticks.h"
#include "output_interface.h"
#include "passages.h"
#include "mach_dep.h"
#include "move.h"
#include "rings.h"
#include "things.h"
#include "potions.h"
#include "pack.h"
#include "scrolls.h"
#include "level.h"
#include "food.h"
#include "hero.h"

void command()
{
    int ntimes;

    if (game->hero().is_fast())
        ntimes = rnd(2) + 2;
    else ntimes = 1;
    while (ntimes--)
    {
        update_status_bar();
        SIG2();
        if (game->sleep_timer)
        {
            if (--game->sleep_timer <= 0) {
                msg("you can move again");
                game->sleep_timer = 0;
            }
        }
        else execcom();
        do_fuses();
        do_daemons();
        for (ntimes = LEFT; ntimes <= RIGHT; ntimes++)
            if (game->hero().get_ring(ntimes))
                switch (game->hero().get_ring(ntimes)->m_which)
                {
                case R_SEARCH:
                    search();
                    break;
                case R_TELEPORT:
                    if (rnd(50) == 17)
                        game->hero().teleport();
                    break;
                }
    }
}

//allow multiple keys to be mapped to the same command
int translate_command(int ch)
{
    switch (ch)
    {
    case '\b': 
        return 'h';
    case '+': 
        return 't';
    case '-':
        return 'z';
    }
    return ch;
}

int com_char()
{
    int ch;
    ch = readchar();
    clear_msg();
    return translate_command(ch);
}

int process_prefixes(int ch)
{
    int junk;
    int command = 0;

    switch (ch)
    {
    case '0': case '1': case '2': case '3': case '4': case '5': case '6': case '7': case '8': case '9':
        junk = game->repeat_cmd_count * 10;
        if ((junk += ch - '0') > 0 && junk < 10000)
            game->repeat_cmd_count = junk;
        show_count();
        break;

    case 'f': // f: toggle fast mode for this turn
        game->modifiers.m_fast_mode = !game->modifiers.m_fast_mode;
        break;
    case 'g': // g + direction: move onto an item without picking it up
        game->can_pickup_this_turn = false;
        break;
    case 'a': //a: repeat last command
        command = game->last_turn.command;
        game->repeat_cmd_count = game->last_turn.count;
        game->can_pickup_this_turn = game->last_turn.could_pickup;
        game->repeat_last_action = true;
        break;
    case ' ':
        break;
    case ESCAPE:
        game->modifiers.m_stop_at_door = false;
        game->repeat_cmd_count = 0;
        show_count();
        break;
    default:
        command = ch;
        break;
    }

    return command;
}

//Read a command, setting things up according to prefix like devices. Return the command character to be executed.
int read_command()
{
    int command, ch;

    bool was_fast_play_enabled = game->modifiers.scroll_lock();
    game->modifiers.m_fast_mode = game->modifiers.scroll_lock();
    look(true);
    if (!game->modifiers.is_running())
        game->modifiers.m_stop_at_door = false;
    game->can_pickup_this_turn = true;
    game->repeat_last_action = false;

    --game->repeat_cmd_count;
    if (game->repeat_cmd_count || game->last_turn.count)
        show_count();

    if (game->repeat_cmd_count > 0) {
        game->can_pickup_this_turn = game->last_turn.could_pickup;
        command = game->last_turn.command;
        game->modifiers.m_fast_mode = false;
    }
    else
    {
        game->repeat_cmd_count = 0;
        if (game->modifiers.is_running()) {
            command = game->run_character;
            game->can_pickup_this_turn = game->last_turn.could_pickup;
        }
        else
        {
            for (command = 0; command == 0;)
            {
                ch = com_char();
                if (was_fast_play_enabled != game->modifiers.scroll_lock())
                    game->modifiers.m_fast_mode = !game->modifiers.m_fast_mode;
                command = process_prefixes(ch);
            }
        }
    }
    if (game->repeat_cmd_count)
        game->modifiers.m_fast_mode = false;
    switch (command)
    {
    case 'h': case 'j': case 'k': case 'l': case 'y': case 'u': case 'b': case 'n':
        if (game->modifiers.fast_mode() && !game->modifiers.is_running())
        {
            if (!game->hero().is_blind()) {
                game->modifiers.m_stop_at_door = true;
                game->modifiers.m_first_move = true;
            }
            command = toupper(command);
        }

    case 'H': case 'J': case 'K': case 'L': case 'Y': case 'U': case 'B': case 'N': case 'q': case 'r': case 's': case 'z': case 't': case '.':

    case CTRL('D'): case 'C':

        break;

    default:
        game->repeat_cmd_count = 0;
    }

    game->last_turn.count = game->repeat_cmd_count;
    game->last_turn.command = command;
    game->last_turn.could_pickup = game->can_pickup_this_turn;
    return command;
}

void show_count()
{
    const int COLS = game->screen().columns();
    const int LINES = game->screen().lines();

    game->screen().move(LINES - 2, COLS - 4);
    if (game->repeat_cmd_count > 0)
        game->screen().printw("%-4d", game->repeat_cmd_count);
    else
        game->screen().addstr("    ");
}

bool dispatch_command(int ch)
{
    bool counts_as_turn = true;

    //todo: replace switch with call into map<char,bool(*)()>
    switch (ch)
    {
    case 'h': case 'j': case 'k': case 'l': case 'y': case 'u': case 'b': case 'n':
    {
        Coord mv;
        find_dir(ch, &mv);
        counts_as_turn = do_move(mv, game->can_pickup_this_turn);
        break;
    }
    case 'H': case 'J': case 'K': case 'L': case 'Y': case 'U': case 'B': case 'N':
        counts_as_turn = do_run(tolower(ch));
        break;
    case 't':
        counts_as_turn = throw_projectile();
        break;
    case 'Q':
        counts_as_turn = false;
        quit();
        break;
    case 'i':
        counts_as_turn = false;
        inventory(game->hero().m_pack, 0, "");
        break;
    case 'd':
        counts_as_turn = drop();
        break;
    case 'q':
        counts_as_turn = quaff();
        break;
    case 'r':
        counts_as_turn = read_scroll();
        break;
    case 'e':
        counts_as_turn = game->hero().eat();
        break;
    case 'w':
        counts_as_turn = game->hero().wield();
        break;
    case 'W':
        counts_as_turn = game->hero().wear_armor();
        break;
    case 'T':
        counts_as_turn = game->hero().take_off_armor();
        break;
    case 'P':
        counts_as_turn = game->hero().put_on_ring();
        break;
    case 'R':
        counts_as_turn = game->hero().remove_ring();
        break;
    case 'c':
        counts_as_turn = false;
        call();
        break;
    case '>':
        counts_as_turn = false;
        go_down_stairs();
        break;
    case '<':
        counts_as_turn = false;
        go_up_stairs();
        break;
    case '/':
        counts_as_turn = false;
        help(helpobjs);
        break;
    case '?':
        counts_as_turn = false;
        help(helpcoms);
        break;
    case '!':
        counts_as_turn = false;
        fakedos();
        break;
    case 's':
        counts_as_turn = true;
        search();
        break;
    case 'z':
        counts_as_turn = do_zap();
        break;
    case 'D':
        counts_as_turn = false;
        discovered();
        break;
    case CTRL('T'):
    {
        bool new_value = !in_brief_mode();
        set_brief_mode(new_value);
        msg(new_value ? "Ok, I'll be brief" : "Goodie, I can use big words again!");
        counts_as_turn = false;
        break;
    }
    case 'F':
        counts_as_turn = false;
        record_macro();
        break;
    case CTRL('F'):
    {
        counts_as_turn = false;
        std::string macro = game->macro;
        std::reverse(macro.begin(), macro.end());
        game->typeahead = macro;
    }
        break;
    case CTRL('R'):
        counts_as_turn = false;
        msg(game->last_message);
        break;
    case 'v':
        counts_as_turn = false;
        msg("Rogue version %d.%d", REV, VER);
        break;
    case 'S':
        counts_as_turn = false;
        save_game();
        break;
    case '.':
        counts_as_turn = true;
        doctor(); 
        break;

    case '^':
    {
        counts_as_turn = false;
        Coord d;
        if (get_dir(&d))
        {
            Coord lookat;

            lookat.y = game->hero().m_position.y + d.y;
            lookat.x = game->hero().m_position.x + d.x;
            if (game->level().get_tile(lookat) != TRAP)
                msg("no trap there.");
            else msg("you found %s", tr_name(game->level().get_trap_type(lookat)));
        }
        break;
    }
    case 'o':
        counts_as_turn = false;
        msg("i don't have any options, oh my!");
        break;
    case CTRL('L'):
        counts_as_turn = false;
        msg("the screen looks fine to me");
        break;

    case CTRL('W'):
        counts_as_turn = false;
        game->wizard().toggle();
        break;

    default:
        if (game->wizard().enabled()) {
            switch (ch) {
                //Wizard commands
            case 'C':
                counts_as_turn = false;
                summon_object();
                break;
            case 'X':
                counts_as_turn = false; 
                show_map(true); 
                break;
            case 'Z':
                counts_as_turn = false; 
                show_map(false);
                break;
            case CTRL('P'):
            {
                counts_as_turn = false;
                char b[255];
                msg("Enter power: ");
                getinfo(b, 128);
                if (*b != ESCAPE)
                    game->wizard().toggle_powers(b);
                clear_msg();
                break;
            }
            default:
                counts_as_turn = false;
                msg("illegal command '%s'", unctrl(ch));
                game->repeat_cmd_count = 0;
            }
        }
        else {
            counts_as_turn = false;
            msg("illegal command '%s'", unctrl(ch));
            game->repeat_cmd_count = 0;
            break;
        }
    }

    return counts_as_turn;
}

void execcom()
{
    bool is_turn;
    do
    {
        int ch = read_command();
        is_turn = dispatch_command(ch);

        if (!game->modifiers.is_running())
            game->modifiers.m_stop_at_door = false;

    } while (!is_turn);
}
