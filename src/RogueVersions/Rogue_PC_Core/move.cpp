//Hero movement commands
//move.c      1.4 (A.I. Design)       12/22/84
#include <ctype.h>

#include "random.h"
#include "move.h"
#include "weapons.h"
#include "output_shim.h"
#include "io.h"
#include "rip.h"
#include "wizard.h"
#include "monsters.h"
#include "rooms.h"
#include "misc.h"
#include "main.h"
#include "fight.h"
#include "level.h"
#include "ring.h"
#include "scrolls.h"
#include "pack.h"
#include "room.h"
#include "game_state.h"
#include "hero.h"
#include "monster.h"

#define BEAR_TIME    spread(3)

//Used to hold the new hero position
Coord new_position; //todo:remove static

//do_run: Start the hero running
bool do_run(Command c) //todo: understand running
{
    game->m_running = true;
    game->run_character = tolower(c.ch);
    return false;
}

bool is_gone(Room* rp)
{
    return ((rp->is_gone()) && (rp->is_maze()) == 0);
}

//diag_ok: Check to see if the move is legal if it is diagonal
int diag_ok(const Coord orig_pos, const Coord new_pos)
{
    if (new_pos.x == orig_pos.x || new_pos.y == orig_pos.y)
        return true;
    return (step_ok(game->level().get_tile({ orig_pos.x,new_pos.y })) && step_ok(game->level().get_tile({ new_pos.x,orig_pos.y })));
}


void finish_do_move(bool is_passage, bool is_maze)
{
    game->level().draw_char(game->hero().position());
    if (is_passage && (game->level().get_tile(game->hero().previous_position()) == DOOR || (game->level().is_maze(game->hero().previous_position()))))
        //mdk:bugfix: the tile where a passage meets a maze is flagged as both passage and maze.
        //This could cause the player to "leave" a maze even when he hadn't.  Monsters could
        //then incorrectly enter the maze.
        if(!is_maze || !game->options.room_bugfix())
            leave_room(new_position);
    if (is_maze && (game->level().is_maze(game->hero().previous_position())) == 0)
        enter_room(new_position);
    game->hero().set_position(new_position);
}

bool is_passage_or_door(Coord p)
{
    return (game->level().is_passage(p) || game->level().get_tile(p) == DOOR);
}

bool continue_vertical()
{
    int dy;

    Coord n = north(game->hero().position());
    Coord s = south(game->hero().position());

    bool up = (game->hero().position().y > 1 && is_passage_or_door(n));
    bool down = (game->hero().position().y < maxrow() - 1 && is_passage_or_door(s));

    if (!(up^down))
        return false;

    if (up) {
        game->run_character = 'k';
        dy = -1;
    }
    else {
        game->run_character = 'j';
        dy = 1;
    }

    new_position.y = game->hero().position().y + dy;
    new_position.x = game->hero().position().x;
    return true;
}

bool continue_horizontal()
{
    int dx;
    const int COLS = game->screen().columns();

    Coord w = west(game->hero().position());
    Coord e = east(game->hero().position());

    bool left = (game->hero().position().x > 1 && is_passage_or_door(w));
    bool right = (game->hero().position().x < COLS - 2 && is_passage_or_door(e));

    if (!(left^right))
        return false;

    if (left) {
        game->run_character = 'h';
        dx = -1;
    }
    else {
        game->run_character = 'l';
        dx = 1;
    }

    new_position.y = game->hero().position().y;
    new_position.x = game->hero().position().x + dx;
    return true;
}

bool this_move_counts; //todo: remove

bool do_hit_boundary()
{
    if (game->in_run_cmd() && is_gone(game->hero().room()) && !game->hero().is_blind())
    {
        switch (game->run_character)
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
    this_move_counts = false;
    game->stop_run_cmd();
    return true;
}

bool do_move_impl(bool can_pickup, bool confused_move)
{
    byte ch;

    //Check if he tried to move off the screen or make an illegal diagonal move, and stop him if he did. fudge it for 40/80 jll -- 2/7/84
    if (offmap(new_position))
        return !do_hit_boundary();

    if (!diag_ok(game->hero().position(), new_position)) {
        this_move_counts = false;
        game->stop_run_cmd();
        return false;
    }
    //If you are running and the move does not get you anywhere stop running
    if (game->in_run_cmd() && equal(game->hero().position(), new_position)) {
        //mdk:bugfix: in the original code you could avoid taking a random step while confused
        //by using the run command.
        if (!confused_move || !game->options.confused_bugfix()) {
            this_move_counts = false;
            game->stop_run_cmd();
        }
    }

    bool is_real = game->level().is_real(new_position);
    bool is_passage = game->level().is_passage(new_position);
    bool is_maze = game->level().is_maze(new_position);

    ch = game->level().get_tile_or_monster(new_position);
    //When the hero is on the door do not allow him to run until he enters the room all the way
    if ((game->level().get_tile(game->hero().position()) == DOOR) && (ch == FLOOR))
        game->stop_run_cmd();
    if (!(is_real) && ch == FLOOR) {
        ch = TRAP;
        game->level().set_tile(new_position, TRAP);
        game->level().set_flag(new_position, F_REAL);
    }
    else if (game->hero().is_held()) {
        //mdk: When you're held, you can only attack your captor
        Monster* monster = game->level().monster_at(new_position);
        if (!monster || !game->hero().is_held_by(monster)) {
            msg("you are being held");
            return false;
        }
    }
    switch (ch)
    {
    case ' ': case VWALL: case HWALL: case ULWALL: case URWALL: case LLWALL: case LRWALL:
        return !do_hit_boundary();

    case DOOR:
        game->stop_run_cmd();
        if (game->level().is_passage(game->hero().position()))
            enter_room(new_position);
        finish_do_move(is_passage, is_maze);
        return false;

    case TRAP:
        ch = handle_trap(new_position);
        if (ch == T_DOOR || ch == T_TELEP)
            return false;

    case PASSAGE:
        finish_do_move(is_passage, is_maze);
        return false;

    case FLOOR:
        if (!(is_real))
            handle_trap(game->hero().position());
        finish_do_move(is_passage, is_maze);
        return false;

    default:
        game->stop_run_cmd();
        if (isupper(ch) || game->level().monster_at(new_position))
            game->hero().fight(new_position, game->hero().get_current_weapon(), false);
        else
        {
            game->stop_run_cmd();
            finish_do_move(is_passage, is_maze);

            if (ch != STAIRS && can_pickup)
                game->hero().pick_up(ch);
        }
    }
    return false;
}

//do_move: Check to see that a move is legal.  If it is handle the consequences (fighting, picking up, etc.)
bool do_move(Command c) //todo:understand
{
    Coord delta;
    find_dir(c.ch, &delta);

    this_move_counts = true;

    game->m_first_move = false;

    //if something went wrong, bail out on this level
    if (game->invalid_position) {
        game->invalid_position = false;
        msg("the crack widens ... ");
        game->save_game("crack_widens.sav");
        descend("");
        return this_move_counts;
    }

    //skip the turn if the hero is stuck in a bear trap
    if (game->hero().decrement_trap_turns()) {
        msg("you are still stuck in the bear trap");
        return this_move_counts;
    }

    //Do a confused move (maybe)
    bool confused_move = false;
    if (game->hero().is_confused() && rnd(5) != 0) {
        rndmove(&game->hero(), &new_position);
        confused_move = true;
    }
    else {
        new_position = game->hero().position() + delta;
    }

    bool more;
    do {
        more = do_move_impl(c.can_pick_up, confused_move);
    } while (more);

    return this_move_counts;
}


//door_open: Called to illuminate a room.  If it is dark, remove anything that might move.
void door_open(Room *room)
{
    if (room->is_gone() || game->hero().is_blind())
        return;

    for (int j = room->m_ul_corner.y; j < room->m_ul_corner.y + room->m_size.y; j++)
    {
        for (int k = room->m_ul_corner.x; k < room->m_ul_corner.x + room->m_size.x; k++)
        {
            Coord pos = { k, j };
            Monster* monster = wake_monster(pos);
            if (monster && monster->tile_beneath() == ' ' && !room->is_dark())
                monster->reload_tile_beneath();
        }
    }
}

//handle_trap: The guy stepped on a trap.... Make him pay.
int handle_trap(Coord tc)
{
    byte tr;

    game->cancel_repeating_cmd();
    game->stop_run_cmd();
    game->level().set_tile(tc, TRAP);
    tr = game->level().get_trap_type(tc);
    game->hero().set_sprung_trap(tr);
    switch (tr)
    {
    case T_DOOR:
        descend("you fell into a trap!");
        break;

    case T_BEAR:
        game->hero().set_trap_turns(BEAR_TIME);
        msg("you are caught in a bear trap");
        break;

    case T_SLEEP:
        game->hero().increase_sleep_turns(SLEEP_TIME);
        msg("a %smist envelops you and you fall asleep", noterse("strange white "));
        break;

    case T_ARROW:
        game->log("battle", "Arrow trap 1d6 attack on player");
        if (attempt_swing(game->hero().level() - 1, game->hero().ac(), 1))
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
                arrow->m_count = 1;
                arrow->set_position(game->hero().position());
                game->level().drop_item(arrow, false);
            }
            msg("an arrow shoots past you");
        }
        break;

    case T_TELEP:
        game->hero().teleport();
        game->screen().add_tile(tc, TRAP); //since the hero's leaving, look() won't put it on for us
        break;

    case T_DART:
        game->log("battle", "Dart trap 1d4 attack on player");
        if (attempt_swing(game->hero().level() + 1, game->hero().ac(), 1))
        {
            if (!game->hero().decrease_hp(roll(1, 4), true)) {
                msg("a poisoned dart killed you");
                death('d');
            }
            if (!save(VS_POISON))
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

void descend(const char *mesg)
{
    game->next_level();
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

    y = newmv->y = who->position().y + rnd(3) - 1;
    x = newmv->x = who->position().x + rnd(3) - 1;
    //Now check to see if that's a legal move.  If not, don't move. (I.e., bump into the wall or whatever)
    if (y == who->position().y && x == who->position().x) return;
    if ((y < 1 || y >= maxrow()) || (x < 0 || x >= COLS)) {
        (*newmv) = who->position();
        return;
    }
    else if (!diag_ok(who->position(), *newmv)) {
        (*newmv) = who->position();
        return;
    }
    else
    {
        ch = game->level().get_tile_or_monster({ x, y });
        if (!step_ok(ch)) {
            (*newmv) = who->position();
            return;
        }
        if (ch == SCROLL)
        {
            for (auto it = game->level().items.begin(); it != game->level().items.end(); ++it) {
                obj = *it;
                if (y == obj->position().y && x == obj->position().x)
                    break;
            }
            if (is_scare_monster_scroll(obj)) {
                (*newmv) = who->position();
                return;
            }
        }
    }
}
