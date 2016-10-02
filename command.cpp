//Read and execute the user commands
//command.c   1.44    (A.I. Design)   2/14/85

#include <map>
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
#include "commands.h"

namespace
{
    std::map<int, bool(*)()> s_commands =
    {
        //proper actions
        { 'e', do_eat },
        { 'w', do_wield },
        { 'W', do_wear_armor },
        { 'T', do_take_off_armor },
        { 'P', do_put_on_ring },
        { 'R', do_remove_ring },
        { 't', do_throw_item },
        { 'z', do_zap },
        { 'q', do_quaff },
        { 'r', do_read_scroll },
        { 'd', do_drop },
        { 's', do_search },
        { '.', do_rest },
        { '>', do_go_down_stairs },
        { '<', do_go_up_stairs },

        //informational/utility actions
        { 'i', do_inventory },
        { 'c', do_call },
        { 'D', do_discovered },
        { '^', do_id_trap },
        { 'F', do_record_macro },
        { 'S', do_save_game },
        { 'Q', do_quit },
        { 'v', do_print_version },
        { '/', do_object_help },
        { '?', do_command_help },
        { '!', do_fakedos },
        { 'o', do_options },
        { CTRL('T'), do_toggle_terse },
        { CTRL('F'), do_play_macro },
        { CTRL('R'), do_repeat_msg },
        { CTRL('L'), do_clear_screen },
        { CTRL('W'), do_toggle_wizard },
    };

    //commands that are recognized only in wizard mode
    std::map<int, bool(*)()> s_wizard_commands = {
        { 'C', do_summon_object },
        { 'X', do_show_map },
        { CTRL('P'), do_toggle_powers },
    };

    bool is_move_command(int ch)
    {
        switch (ch) {
        case 'h': case 'j': case 'k': case 'l': case 'y': case 'u': case 'b': case 'n':
            return true;
        }
        return false;
    }

    bool is_run_command(int ch)
    {
        switch (ch) {
        case 'H': case 'J': case 'K': case 'L': case 'Y': case 'U': case 'B': case 'N':
            return true;
        }
        return false;
    }
}

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
                    do_search();
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

int process_prefixes(int ch, bool* fast_mode)
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
        *fast_mode = !*fast_mode;
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
        game->m_stop_at_door = false;
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

    bool fast_mode = false;
    look(true);
    if (!game->is_running())
        game->m_stop_at_door = false;
    game->can_pickup_this_turn = true;
    game->repeat_last_action = false;

    --game->repeat_cmd_count;
    if (game->repeat_cmd_count || game->last_turn.count)
        show_count();

    if (game->repeat_cmd_count > 0) {
        game->can_pickup_this_turn = game->last_turn.could_pickup;
        command = game->last_turn.command;
    }
    else
    {
        game->repeat_cmd_count = 0;
        if (game->is_running()) {
            command = game->run_character;
            game->can_pickup_this_turn = game->last_turn.could_pickup;
        }
        else
        {
            for (command = 0; command == 0;)
            {
                ch = com_char();
                command = process_prefixes(ch, &fast_mode);
            }
            fast_mode ^= game->fast_play();
        }
    }
    if (game->repeat_cmd_count)
        fast_mode = false;

    switch (command)
    {
    case 'h': case 'j': case 'k': case 'l': case 'y': case 'u': case 'b': case 'n':
        if (fast_mode && !game->is_running())
        {
            if (!game->hero().is_blind()) {
                game->m_stop_at_door = true;
                game->m_first_move = true;
            }
            command = toupper(command);
        }

    case 'H': case 'J': case 'K': case 'L': case 'Y': case 'U': case 'B': case 'N':
        if (game->options.stop_running_at_doors() && !game->is_running())
        {
            if (!game->hero().is_blind()) {
                game->m_stop_at_door = true;
                game->m_first_move = true;
            }
        }
    
    case 'q': case 'r': case 's': case 'z': case 't': case '.':

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
    if (is_move_command(ch))
        return do_move(ch, game->can_pickup_this_turn);
    else if (is_run_command(ch))
        return do_run(ch);

    //try executing the command
    auto i = s_commands.find(ch);
    if (i != s_commands.end()) {
        return (*i->second)();
    }

    //if we're a wizard look at wizard commands too
    if (game->wizard().enabled()) {
        auto i = s_wizard_commands.find(ch);
        if (i != s_wizard_commands.end()) {
            return (*i->second)();
        }
    }

    msg("illegal command '%s'", unctrl(ch));
    game->repeat_cmd_count = 0;
    return false;
}


void execcom()
{
    bool counts_as_turn;
    do
    {
        int ch = read_command();
        counts_as_turn = dispatch_command(ch);

        //todo: why is this here?
        if (!game->is_running())
            game->m_stop_at_door = false;

    } while (!counts_as_turn);
}
