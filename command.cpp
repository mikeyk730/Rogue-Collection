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

static byte can_pickup_this_turn; //todo:eliminate

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

int com_char()
{
    int ch;
    ch = readchar();
    switch (ch)
    {
    case '\b': ch = 'h'; break;
    case '+': ch = 't'; break;
    case '-': ch = 'z'; break;
    }
    if (msg_position && !game->modifiers.is_running())
        msg("");
    return ch;
}

int process_prefixes(int ch)
{
    int junk;
    int command = 0;

    switch (ch)
    {
    case '0': case '1': case '2': case '3': case '4': case '5': case '6': case '7': case '8': case '9':
        junk = repeat_cmd_count * 10;
        if ((junk += ch - '0') > 0 && junk < 10000)
            repeat_cmd_count = junk;
        show_count();
        break;

    case 'f': // f: toggle fast mode for this turn
        game->modifiers.m_fast_mode = !game->modifiers.m_fast_mode;
        break;
    case 'g': // g + direction: move onto an item without picking it up
        can_pickup_this_turn = false;
        break;
    case 'a': //a: repeat last command
        command = game->last_turn.command;
        repeat_cmd_count = game->last_turn.count;
        can_pickup_this_turn = game->last_turn.could_pickup;
        repeat_last_action = true;
        break;
    case ' ':
        break;
    case ESCAPE:
        game->modifiers.m_stop_at_door = false;
        repeat_cmd_count = 0;
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
    can_pickup_this_turn = true;
    repeat_last_action = false;

    --repeat_cmd_count;
    if (repeat_cmd_count || game->last_turn.count)
        show_count();

    if (repeat_cmd_count > 0) {
        can_pickup_this_turn = game->last_turn.could_pickup;
        command = game->last_turn.command;
        game->modifiers.m_fast_mode = false;
    }
    else
    {
        repeat_cmd_count = 0;
        if (game->modifiers.is_running()) {
            command = run_character;
            can_pickup_this_turn = game->last_turn.could_pickup;
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
    if (repeat_cmd_count)
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
        repeat_cmd_count = 0;
    }

    game->last_turn.count = repeat_cmd_count;
    game->last_turn.command = command;
    game->last_turn.could_pickup = can_pickup_this_turn;
    return command;
}

void show_count()
{
    const int COLS = game->screen().columns();
    const int LINES = game->screen().lines();

    game->screen().move(LINES - 2, COLS - 4);
    if (repeat_cmd_count > 0)
        game->screen().printw("%-4d", repeat_cmd_count);
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
        do_move(mv, can_pickup_this_turn);
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
            counts_as_turn = false;
        break;
    }
    case 'Q':
        counts_as_turn = false;
        quit();
        break;
    case 'i':
        counts_as_turn = false;
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
        ring_on();
        break;
    case 'R':
        game->hero().ring_off();
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
        search();
        break;
    case 'z':
    {
        Coord d;
        if (get_dir(&d))
            do_zap(d);
        else
            counts_as_turn = false;
        break;
    }
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
        counts_as_turn = false;
        //todo: revisit macro later, this definitely isn't safe
        //typeahead = game->get_environment("macro").c_str();
        break;
    case CTRL('R'):
        counts_as_turn = false;
        msg(last_message);
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
        doctor(); break;

    case '^':
    {
        counts_as_turn = false;
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
        counts_as_turn = false;
        msg("i don't have any options, oh my!");
        break;
    case CTRL('L'):
        counts_as_turn = false;
        msg("the screen looks fine to me");
        break;

    case CTRL('W'):
        counts_as_turn = false;
        game->hero().toggle_wizard();
        break;

    default:
        if (game->hero().is_wizard()) {
            switch (ch) {
                //Wizard commands
            case 'C':
                counts_as_turn = false;
                summon_object();
                break;
            case 'X':
                counts_as_turn = false; show_map(true); break;
            case 'Z':
                counts_as_turn = false;  show_map(false); break;
            default:
                counts_as_turn = false;
                msg("illegal command '%s'", unctrl(ch));
                repeat_cmd_count = 0;
            }
        }
        else {
            counts_as_turn = false;
            msg("illegal command '%s'", unctrl(ch));
            repeat_cmd_count = 0;
            break;
        }
    }
}

void execcom()
{
    do
    {
        counts_as_turn = true;

        int ch = read_command();
        dispatch_command(ch);

        if (!game->modifiers.is_running())
            game->modifiers.m_stop_at_door = false;

    } while (!counts_as_turn);
}
