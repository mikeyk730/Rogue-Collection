//Hero movement commands
//move.c      1.4 (A.I. Design)       12/22/84
#include <ctype.h>

#include "rogue.h"
#include "move.h"
#include "weapons.h"
#include "output_interface.h"
#include "io.h"
#include "rip.h"
#include "wizard.h"
#include "monsters.h"
#include "rooms.h"
#include "misc.h"
#include "main.h"
#include "fight.h"
#include "chase.h"
#include "mach_dep.h"
#include "level.h"
#include "rings.h"
#include "scrolls.h"
#include "pack.h"
#include "room.h"
#include "game_state.h"
#include "hero.h"
#include "monster.h"

//Used to hold the new hero position
Coord new_position;

//do_run: Start the hero running
void do_run(byte ch)
{
    game->modifiers.m_running = true;
    counts_as_turn = false;
    run_character = ch;
}

bool is_gone(Room* rp)
{
    return ((rp->is_gone()) && (rp->is_maze()) == 0);
}

//diag_ok: Check to see if the move is legal if it is diagonal
int diag_ok(const Coord sp, const Coord ep)
{
    if (ep.x == sp.x || ep.y == sp.y)
        return true;
    return (step_ok(game->level().get_tile({ sp.x,ep.y })) && step_ok(game->level().get_tile({ ep.x,sp.y })));
}


void finish_do_move(int fl)
{
    game->level().draw_char(game->hero().pos);
    if ((fl&F_PASS) && (game->level().get_tile(oldpos) == DOOR || (game->level().get_flags(oldpos)&F_MAZE)))
        leave_room(new_position);
    if ((fl&F_MAZE) && (game->level().get_flags(oldpos)&F_MAZE) == 0)
        enter_room(new_position);
    game->hero().pos = new_position;
}

bool continue_vertical() {
    int dy;

    bool up_is_door_or_psg = (game->hero().pos.y > 1 && ((game->level().get_flags({ game->hero().pos.x,game->hero().pos.y - 1 })&F_PASS) ||
        game->level().get_tile({ game->hero().pos.x,game->hero().pos.y - 1 }) == DOOR));
    bool down_is_door_or_psg = (game->hero().pos.y < maxrow - 1 && ((game->level().get_flags({ game->hero().pos.x,game->hero().pos.y + 1 })&F_PASS) ||
        game->level().get_tile({ game->hero().pos.x,game->hero().pos.y + 1 }) == DOOR));
    if (!(up_is_door_or_psg^down_is_door_or_psg))
        return false;
    if (up_is_door_or_psg) {
        run_character = 'k';
        dy = -1;
    }
    else {
        run_character = 'j';
        dy = 1;
    }
    new_position.y = game->hero().pos.y + dy;
    new_position.x = game->hero().pos.x;
    return true;
}

bool continue_horizontal()
{
    int dx;
    const int COLS = game->screen().columns();

    bool left_is_door_or_psg = (game->hero().pos.x > 1 && ((game->level().get_flags({ game->hero().pos.x - 1,game->hero().pos.y })&F_PASS) ||
        game->level().get_tile({ game->hero().pos.x - 1,game->hero().pos.y }) == DOOR));
    bool right_is_door_or_psg = (game->hero().pos.x < COLS - 2 && ((game->level().get_flags({ game->hero().pos.x + 1,game->hero().pos.y })&F_PASS) ||
        game->level().get_tile({ game->hero().pos.x + 1,game->hero().pos.y }) == DOOR));
    if (!(left_is_door_or_psg^right_is_door_or_psg))
        return false;
    if (left_is_door_or_psg) {
        run_character = 'h';
        dx = -1;
    }
    else {
        run_character = 'l';
        dx = 1;
    }
    new_position.y = game->hero().pos.y;
    new_position.x = game->hero().pos.x + dx;
    return true;
}

bool do_hit_boundary()
{
    if (game->modifiers.is_running() && is_gone(game->hero().room) && !game->hero().is_blind())
    {
        switch (run_character)
        {
        case 'h': case 'l':
        {
            if (continue_vertical())
                return false;
            break;
        }
        case 'j': case 'k':
        {
            if (continue_horizontal())
                return false;
            break;
        }
        }
    }
    counts_as_turn = false;
    game->modifiers.m_running = false;
    return true;
}

bool do_move_impl()
{
    byte ch;
    int fl;

    //Check if he tried to move off the screen or make an illegal diagonal move, and stop him if he did. fudge it for 40/80 jll -- 2/7/84
    if (offmap(new_position))
        return !do_hit_boundary();
    if (!diag_ok(game->hero().pos, new_position)) {
        counts_as_turn = false;
        game->modifiers.m_running = false;
        return false;
    }
    //If you are running and the move does not get you anywhere stop running
    if (game->modifiers.is_running() && equal(game->hero().pos, new_position)) {
        counts_as_turn = false;
        game->modifiers.m_running = false;
    }
    fl = game->level().get_flags(new_position);
    ch = game->level().get_tile_or_monster(new_position);
    //When the hero is on the door do not allow him to run until he enters the room all the way
    if ((game->level().get_tile(game->hero().pos) == DOOR) && (ch == FLOOR))
        game->modifiers.m_running = false;
    if (!(fl&F_REAL) && ch == FLOOR) {
        ch = TRAP;
        game->level().set_tile(new_position, TRAP);
        game->level().set_flag(new_position, F_REAL);
    }
    else if (game->hero().is_held() && ch != 'F') { //TODO: remove direct check for F
        msg("you are being held");
        return false;
    }
    switch (ch)
    {
    case ' ': case VWALL: case HWALL: case ULWALL: case URWALL: case LLWALL: case LRWALL:
        return !do_hit_boundary();

    case DOOR:
        game->modifiers.m_running = false;
        if (game->level().get_flags(game->hero().pos)&F_PASS)
            enter_room(new_position);
        finish_do_move(fl);
        return false;

    case TRAP:
        ch = handle_trap(new_position);
        if (ch == T_DOOR || ch == T_TELEP)
            return false;

    case PASSAGE:
        finish_do_move(fl);
        return false;

    case FLOOR:
        if (!(fl&F_REAL))
            handle_trap(game->hero().pos);
        finish_do_move(fl);
        return false;

    default:
        game->modifiers.m_running = false;
        if (isupper(ch) || game->level().monster_at(new_position))
            game->hero().fight(&new_position, game->hero().get_current_weapon(), false);
        else
        {
            game->modifiers.m_running = false;
            if (ch != STAIRS)
                take = ch;

            finish_do_move(fl);
        }
    }
    return false;
}

//do_move: Check to see that a move is legal.  If it is handle the consequences (fighting, picking up, etc.)
void do_move(int dy, int dx)
{
    game->modifiers.m_first_move = false;

    //if something went wrong, bail out on this level
    if (invalid_position) {
        invalid_position = false;
        msg("the crack widens ... ");
        descend("");
        return;
    }

    //skip the turn if the hero is stuck in a bear trap
    if (game->bear_trap_turns) {
        game->bear_trap_turns--;
        msg("you are still stuck in the bear trap");
        return;
    }

    //Do a confused move (maybe)
    if (game->hero().is_confused() && rnd(5) != 0)
        rndmove(&game->hero(), &new_position);
    else
    {
        new_position.y = game->hero().pos.y + dy;
        new_position.x = game->hero().pos.x + dx;
    }

    bool more;
    do {
        more = do_move_impl();
    } while (more);
}


//door_open: Called to illuminate a room.  If it is dark, remove anything that might move.
void door_open(Room *room)
{
    int j, k;
    byte ch;
    Monster* item;

    if (!(room->is_gone()) && !game->hero().is_blind())
        for (j = room->pos.y; j < room->pos.y + room->size.y; j++)
            for (k = room->pos.x; k < room->pos.x + room->size.x; k++)
            {
                ch = game->level().get_tile_or_monster({ k,j });
                if (isupper(ch))
                {
                    item = wake_monster({ k,j });
                    if (item->oldch == ' ' && !(room->is_dark()) && !game->hero().is_blind())
                        item->oldch = game->level().get_tile({ k,j });
                }
            }
}

//handle_trap: The guy stepped on a trap.... Make him pay.
int handle_trap(Coord tc)
{
    byte tr;
    const int COLS = game->screen().columns();

    repeat_cmd_count = game->modifiers.m_running = false;
    game->level().set_tile(tc, TRAP);
    tr = game->level().get_trap_type(tc);
    was_trapped = 1;
    switch (tr)
    {
    case T_DOOR:
        descend("you fell into a trap!");
        break;

    case T_BEAR:
        game->bear_trap_turns += BEAR_TIME;
        msg("you are caught in a bear trap");
        break;

    case T_SLEEP:
        game->sleep_timer += SLEEP_TIME;
        game->hero().set_running(false);
        msg("a %smist envelops you and you fall asleep", noterse("strange white "));
        break;

    case T_ARROW:
        game->log("battle", "Arrow trap 1d6 attack on player");
        if (attempt_swing(game->hero().stats.level - 1, game->hero().stats.ac, 1))
        {
            if (!game->hero().decrease_hp(roll(1, 6), true)) {
                msg("an arrow killed you");
                death('a');
            }
            else
                msg("oh no! An arrow shot you");
        }
        else
        {
            Item *arrow;

            if ((arrow = new Weapon(ARROW, 0, 0)) != NULL)
            {
                arrow->count = 1;
                arrow->pos = game->hero().pos;
                fall(arrow, false);
            }
            msg("an arrow shoots past you");
        }
        break;

    case T_TELEP:
        game->hero().teleport();
        game->screen().mvaddch(tc, TRAP); //since the hero's leaving, look() won't put it on for us
        was_trapped++;//todo:look at this
        break;

    case T_DART:
        game->log("battle", "Dart trap 1d4 attack on player");
        if (attempt_swing(game->hero().stats.level + 1, game->hero().stats.ac, 1))
        {
            if (!game->hero().decrease_hp(roll(1, 4), true)) {
                msg("a poisoned dart killed you");
                death('d');
            }
            if (!game->hero().is_wearing_ring(R_SUSTSTR) && !save(VS_POISON))
                game->hero().adjust_strength(-1);
            msg("a dart just hit you in the shoulder");
        }
        else
            msg("a dart whizzes by your ear and vanishes");
        break;
    }
    clear_typeahead_buffer();
    return tr;
}

void descend(char *mesg)
{
    next_level();
    if (*mesg == 0) msg(" ");
    game->level().new_level(true);
    msg("");
    msg(mesg);
    if (!save(VS_LUCK))
    {
        msg("you are damaged by the fall");
        if (!game->hero().decrease_hp(roll(1, 8), true))
            death('f');
    }
}

//rndmove: Move in a random direction if the monster/person is confused
void rndmove(Agent *who, Coord *newmv)
{
    int x, y;
    byte ch;
    Item *obj;
    const int COLS = game->screen().columns();

    y = newmv->y = who->pos.y + rnd(3) - 1;
    x = newmv->x = who->pos.x + rnd(3) - 1;
    //Now check to see if that's a legal move.  If not, don't move. (I.e., bump into the wall or whatever)
    if (y == who->pos.y && x == who->pos.x) return;
    if ((y < 1 || y >= maxrow) || (x < 0 || x >= COLS))
        goto bad;
    else if (!diag_ok(who->pos, *newmv))
        goto bad;
    else
    {
        ch = game->level().get_tile_or_monster({ x, y });
        if (!step_ok(ch))
            goto bad;
        if (ch == SCROLL)
        {
            for (auto it = game->level().items.begin(); it != game->level().items.end(); ++it) {
                obj = *it;
                if (y == obj->pos.y && x == obj->pos.x)
                    break;
            }
            if (is_scare_monster_scroll(obj))
                goto bad;
        }
    }
    return;

bad:

    (*newmv) = who->pos;
    return;
}
