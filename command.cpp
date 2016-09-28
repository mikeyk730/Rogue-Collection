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
        status();
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
                switch (game->hero().get_ring(ntimes)->which)
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

void dispatch_command(int ch)
{
    switch (ch)
    {
    case 'h': case 'j': case 'k': case 'l': case 'y': case 'u': case 'b': case 'n':
    {
        Coord mv;
        find_dir(ch, &mv);
        do_move(mv, game->can_pickup_this_turn);
        break;
    }
    case 'H': case 'J': case 'K': case 'L': case 'Y': case 'U': case 'B': case 'N':
        do_run(tolower(ch));
        break;
    case 't':
    {
        Coord d;
        if (get_dir(&d))
            throw_projectile(d);
        else
            game->counts_as_turn = false;
        break;
    }
    case 'Q':
        game->counts_as_turn = false;
        quit();
        break;
    case 'i':
        game->counts_as_turn = false;
        inventory(game->hero().pack, 0, "");
        break;
    case 'd':
        drop();
        break;
    case 'q':
        quaff();
        break;
    case 'r':
        read_scroll();
        break;
    case 'e':
        eat();
        break;
    case 'w':
        game->hero().wield();
        break;
    case 'W':
        wear();
        break;
    case 'T':
        game->hero().take_off();
        break;
    case 'P':
        game->counts_as_turn = ring_on();
        break;
    case 'R':
        game->counts_as_turn = game->hero().take_off_ring();
        break;
    case 'c':
        game->counts_as_turn = false;
        call();
        break;
    case '>':
        game->counts_as_turn = false;
        go_down_stairs();
        break;
    case '<':
        game->counts_as_turn = false;
        go_up_stairs();
        break;
    case '/':
        game->counts_as_turn = false;
        help(helpobjs);
        break;
    case '?':
        game->counts_as_turn = false;
        help(helpcoms);
        break;
    case '!':
        game->counts_as_turn = false;
        fakedos();
        break;
    case 's':
        search();
        break;
    case 'z':
    {
        Coord d;
        if (get_dir(&d))
            do_zap(d);
        else
            game->counts_as_turn = false;
        break;
    }
    case 'D':
        game->counts_as_turn = false;
        discovered();
        break;
    case CTRL('T'):
    {
        bool new_value = !in_brief_mode();
        set_brief_mode(new_value);
        msg(new_value ? "Ok, I'll be brief" : "Goodie, I can use big words again!");
        game->counts_as_turn = false;
        break;
    }
    case 'F':
        game->counts_as_turn = false;
        record_macro();
        break;
    case CTRL('F'):
        game->counts_as_turn = false;
        //todo: revisit macro later, this definitely isn't safe
        //typeahead = game->get_environment("macro").c_str();
        break;
    case CTRL('R'):
        game->counts_as_turn = false;
        msg(game->last_message);
        break;
    case 'v':
        game->counts_as_turn = false;
        msg("Rogue version %d.%d", REV, VER);
        break;
    case 'S':
        game->counts_as_turn = false;
        save_game();
        break;
    case '.':
        doctor(); break;

    case '^':
    {
        game->counts_as_turn = false;
        Coord d;
        if (get_dir(&d))
        {
            Coord lookat;

            lookat.y = game->hero().pos.y + d.y;
            lookat.x = game->hero().pos.x + d.x;
            if (game->level().get_tile(lookat) != TRAP)
                msg("no trap there.");
            else msg("you found %s", tr_name(game->level().get_trap_type(lookat)));
        }
        break;
    }
    case 'o':
        game->counts_as_turn = false;
        msg("i don't have any options, oh my!");
        break;
    case CTRL('L'):
        game->counts_as_turn = false;
        msg("the screen looks fine to me");
        break;

    case CTRL('W'):
        game->counts_as_turn = false;
        game->hero().toggle_wizard();
        break;

    default:
        if (game->hero().is_wizard()) {
            switch (ch) {
                //Wizard commands
            case 'C':
                game->counts_as_turn = false;
                summon_object();
                break;
            case 'X':
                game->counts_as_turn = false; show_map(true); break;
            case 'Z':
                game->counts_as_turn = false;  show_map(false); break;
            default:
                game->counts_as_turn = false;
                msg("illegal command '%s'", unctrl(ch));
                game->repeat_cmd_count = 0;
            }
        }
        else {
            game->counts_as_turn = false;
            msg("illegal command '%s'", unctrl(ch));
            game->repeat_cmd_count = 0;
            break;
        }
    }
}

void execcom()
{
    do
    {
        game->counts_as_turn = true;

        int ch = read_command();
        dispatch_command(ch);

        if (!game->modifiers.is_running())
            game->modifiers.m_stop_at_door = false;

    } while (!game->counts_as_turn);
}
