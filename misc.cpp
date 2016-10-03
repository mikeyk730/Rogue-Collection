//All sorts of miscellaneous routines
//misc.c       1.4             (A.I. Design)   12/14/84

#include <algorithm>
#include <sstream>
#include <stdio.h>

#include "rogue.h"
#include "game_state.h"
#include "io.h"
#include "daemons.h"
#include "misc.h"
#include "monsters.h"
#include "pack.h"
#include "output_interface.h"
#include "rip.h"
#include "list.h"
#include "mach_dep.h"
#include "main.h"
#include "fight.h"
#include "daemon.h"
#include "level.h"
#include "rings.h"
#include "scrolls.h"
#include "potions.h"
#include "sticks.h"
#include "armor.h"
#include "hero.h"
#include "room.h"
#include "monster.h"

const int MACROSZ = 41;

//tr_name: Print the name of a trap
char *tr_name(byte type)
{
    switch (type)
    {
    case T_DOOR: return "a trapdoor";
    case T_BEAR: return "a beartrap";
    case T_SLEEP: return "a sleeping gas trap";
    case T_ARROW: return "an arrow trap";
    case T_TELEP: return "a teleport trap";
    case T_DART: return "a poison dart trap";
    }
    msg("weird trap: %d", type);
    return NULL;
}

//look: A quick glance all around the player
void look(bool wakeup) //todo: learn this function
{
    int x, y;
    byte ch, pch;
    Monster* monster;
    struct Room *room;
    int ey, ex;
    int passcount = 0;
    byte pfl, fp;
    int sy, sx, sumhero, diffhero;
    const int COLS = game->screen().columns();

    room = game->hero().m_room;
    pfl = game->level().get_flags(game->hero().m_position);
    pch = game->level().get_tile(game->hero().m_position);
    //if the hero has moved
    if (!equal(game->oldpos, game->hero().m_position))
    {
        if (!game->hero().is_blind())
        {
            for (x = game->oldpos.x - 1; x <= (game->oldpos.x + 1); x++)
                for (y = game->oldpos.y - 1; y <= (game->oldpos.y + 1); y++)
                {
                    Coord pos = { x, y };
                    if ((pos == game->hero().m_position) || offmap(pos)) continue;
                    game->screen().move(y, x);
                    ch = game->screen().curch();
                    if (ch == FLOOR)
                    {
                        if (game->oldrp->is_dark() && !game->oldrp->is_gone())
                            game->screen().addch(' ');
                    }
                    else
                    {
                        fp = game->level().get_flags(pos);
                        //if the maze or passage (that the hero is in!!) needs to be redrawn (passages once drawn always stay on) do it now.
                        if (((fp&F_MAZE) || (fp&F_PASS)) && (ch != PASSAGE) && (ch != STAIRS) && ((fp&F_PNUM) == (pfl & F_PNUM)))
                            game->screen().addch(PASSAGE);
                    }
                }
        }
        game->oldpos = game->hero().m_position;
        game->oldrp = room;
    }
    ey = game->hero().m_position.y + 1;
    ex = game->hero().m_position.x + 1;
    sx = game->hero().m_position.x - 1;
    sy = game->hero().m_position.y - 1;
    if (game->stop_at_door() && !game->first_move() && game->in_run_cmd()) {
        sumhero = game->hero().m_position.y + game->hero().m_position.x;
        diffhero = game->hero().m_position.y - game->hero().m_position.x;
    }
    for (y = sy; y <= ey; y++) if (y > 0 && y < maxrow()) {
        for (x = sx; x <= ex; x++)
        {
            if (x <= 0 || x >= COLS) continue;
            if (!game->hero().is_blind())
            {
                if (y == game->hero().m_position.y && x == game->hero().m_position.x) continue;
            }
            else if (y != game->hero().m_position.y || x != game->hero().m_position.x) continue;
            //THIS REPLICATES THE moat() MACRO.  IF MOAT IS CHANGED, THIS MUST BE CHANGED ALSO ?? What does this really mean ??
            fp = game->level().get_flags({ x, y });
            ch = game->level().get_tile({ x, y });
            //No Doors
            if (pch != DOOR && ch != DOOR)
                //Either hero or other in a passage
                if ((pfl&F_PASS) != (fp & F_PASS))
                {
                    //Neither is in a maze
                    if (!(pfl&F_MAZE) && !(fp&F_MAZE)) continue;
                }
            //Not in same passage
                else if ((fp&F_PASS) && (fp&F_PNUM) != (pfl & F_PNUM)) continue;
                if ((monster = game->level().monster_at({ x, y })) != NULL) if (game->hero().detects_others() && monster->is_invisible())
                {
                    if (game->stop_at_door() && !game->first_move())
                        game->stop_running();
                    continue;
                }
                else
                {
                    if (wakeup) wake_monster({ x,y });
                    if (monster->tile_beneath() != ' ' || (!(room->is_dark()) && !game->hero().is_blind()))
                        monster->reload_tile_beneath();
                    if (game->hero().can_see_monster(monster)) 
                        ch = monster->m_disguise;
                }
                //The current character used for IBM ARMOR doesn't look right in Inverse
                if ((ch != PASSAGE) && (fp&(F_PASS | F_MAZE))) if (ch != ARMOR) game->screen().standout();
                game->screen().move(y, x);
                game->screen().addch(ch);
                game->screen().standend();
                if (game->stop_at_door() && !game->first_move() && game->in_run_cmd())
                {
                    switch (game->run_character)
                    {
                    case 'h': if (x == ex) continue; break;
                    case 'j': if (y == sy) continue; break;
                    case 'k': if (y == ey) continue; break;
                    case 'l': if (x == sx) continue; break;
                    case 'y': if ((y + x) - sumhero >= 1) continue; break;
                    case 'u': if ((y - x) - diffhero >= 1) continue; break;
                    case 'n': if ((y + x) - sumhero <= -1) continue; break;
                    case 'b': if ((y - x) - diffhero <= -1) continue; break;
                    }
                    switch (ch)
                    {
                    case DOOR:
                        if (x == game->hero().m_position.x || y == game->hero().m_position.y)
                            game->stop_running();
                        break;
                    case PASSAGE:
                        if (x == game->hero().m_position.x || y == game->hero().m_position.y)
                            passcount++;
                        break;
                    case FLOOR: case VWALL: case HWALL: case ULWALL: case URWALL: case LLWALL: case LRWALL: case ' ':
                        break;
                    default:
                        game->stop_running();
                        break;
                    }
                }
        }
    }
    if (game->stop_at_door() && !game->first_move() && passcount > 1)
        game->stop_running();
    game->screen().move(game->hero().m_position.y, game->hero().m_position.x);
    //todo:check logic
    if ((game->level().is_passage(game->hero().m_position)) || (game->was_trapped > 1) || (game->level().is_maze(game->hero().m_position)))
        game->screen().standout();
    game->screen().addch(PLAYER);
    game->screen().standend();
    if (game->was_trapped) {
        beep();
        game->was_trapped = 0;
    }
}

//find_obj: Find the unclaimed object at y, x
Item* find_obj(Coord p, bool expect_item)
{
    for (auto it = game->level().items.begin(); it != game->level().items.end(); ++it) {
        Item* op = *it;
        if (op->m_position.y == p.y && op->m_position.x == p.x)
            return op;
    }
    if (expect_item) {
        std::ostringstream ss;
        ss << "Non-item " << game->level().get_tile(p) << " " << p.y << "," << p.x;
        game->log("error", ss.str());
        debug(ss.str().c_str());
    }
    return NULL;
}

//aggravate_monsters: Aggravate all the monsters on this level
void aggravate_monsters()
{
    std::for_each(game->level().monsters.begin(), game->level().monsters.end(), [](Monster *monster) {
        monster->start_run();
    });
}

//vowelstr: For printfs: if string starts with a vowel, return "n" for an "an".
const char *vowelstr(const char *str)
{
    switch (*str)
    {
    case 'a': case 'A': case 'e': case 'E': case 'i': case 'I': case 'o': case 'O': case 'u': case 'U':
        return "n";
    default:
        return "";
    }
}

std::string vowelstr(const std::string & str)
{
    return vowelstr(str.c_str());
}

//is_in_use: See if the object is one of the currently used items
int is_in_use(Item *obj)
{
    if (obj == NULL)
        return false;
    if (obj == game->hero().get_current_armor() || obj == game->hero().get_current_weapon() || 
        obj == game->hero().get_ring(LEFT) || obj == game->hero().get_ring(RIGHT))
    {
        msg("That's already in use");
        return true;
    }
    return false;
}

int get_dir_impl(Coord* delta)
{
    int ch;

    msg("which direction? ");
    do {
        if ((ch = readchar()) == ESCAPE) {
            msg("");
            return false;
        }
    } while (find_dir(ch, delta) == 0);
    msg("");
    if (game->hero().is_confused() && rnd(5) == 0) do
    {
        delta->y = rnd(3) - 1;
        delta->x = rnd(3) - 1;
    } while (delta->y == 0 && delta->x == 0);
    return true;
}

//get_dir: Set up the direction co_ordinate for use in various "prefix" commands
int get_dir(Coord *delta)
{
    if (game->repeat_last_action) {
        *delta = game->last_turn.input_direction;
        return true;
    }
    if (get_dir_impl(delta))
    {
        game->last_turn.input_direction = *delta;
        return true;
    }
    return false;
}

bool find_dir(byte ch, Coord *cp)
{
    bool gotit;

    gotit = true;
    switch (ch)
    {
    case 'h': case'H':
        cp->y = 0; cp->x = -1;
        break;
    case 'j': case'J':
        cp->y = 1; cp->x = 0;
        break;
    case 'k': case'K':
        cp->y = -1; cp->x = 0;
        break;
    case 'l': case'L':
        cp->y = 0; cp->x = 1;
        break;
    case 'y': case'Y':
        cp->y = -1; cp->x = -1;
        break;
    case 'u': case'U':
        cp->y = -1; cp->x = 1;
        break;
    case 'b': case'B':
        cp->y = 1; cp->x = -1;
        break;
    case 'n': case'N':
        cp->y = 1; cp->x = 1;
        break;
    default:
        gotit = false;
        break;
    }
    return gotit;
}

//sign: Return the sign of the number
int sign(int n)
{
    if (n < 0)
        return -1;
    else
        return (n > 0);
}

//spread: Give a spread around a given number (+/- 10%)
int spread(int nm)
{
    int r = nm - nm / 10 + rnd(nm / 5);
    return r;
}

//step_ok: Returns true if it is ok to step on ch
int step_ok(int ch)
{
    //return false if wall or monster
    switch (ch)
    {
    case ' ': case VWALL: case HWALL: case ULWALL: case URWALL: case LLWALL: case LRWALL:
        return false;
    default:
        return ((ch < 'A') || (ch > 'Z'));
    }
}

//get_magic_char: Decide how good an object is and return the correct character for printing.
int get_magic_char(Item *obj)
{
    if (obj->is_cursed() || obj->is_evil())
        return BMAGIC;
    return MAGIC;
}

//help: prints out help screens
void help(const char*const* helpscr)
{
    int hcount = 0;
    int hrow, hcol;
    int isfull;
    byte answer = 0;

    game->screen().wdump();
    while (*helpscr && answer != ESCAPE)
    {
        isfull = false;
        if ((hcount % (in_small_screen_mode() ? 23 : 46)) == 0) game->screen().clear();
        //determine row and column
        hcol = 0;
        if (in_small_screen_mode())
        {
            hrow = hcount % 23;
            if (hrow == 22) isfull = true;
        }
        else
        {
            hrow = (hcount % 46) / 2;
            if (hcount % 2) hcol = 40;
            if (hrow == 22 && hcol == 40) isfull = true;
        }
        game->screen().move(hrow, hcol);
        game->screen().addstr(*helpscr++);
        //decide if we need print a continue type message
        if ((*helpscr == 0) || isfull)
        {
            if (*helpscr == 0)
                game->screen().mvaddstr(24, 0, "--press space to continue--");
            else if (in_small_screen_mode())
                game->screen().mvaddstr(24, 0, "--Space for more, Esc to continue--");
            else
                game->screen().mvaddstr(24, 0, "--Press space for more, Esc to continue--");
            do {
                answer = readchar();
            } while (answer != ' ' && answer != ESCAPE);
        }
        hcount++;
    }
    game->screen().wrestor();
}

int distance(Coord a, Coord b)
{
    int dx, dy;
    dx = (a.x - b.x);
    dy = (a.y - b.y);
    return dx*dx + dy*dy;
}

int equal(Coord a, Coord b)
{
    return a == b;
}

int offmap(Coord p)
{
    const int COLS = game->screen().columns();
    return (p.y < 1 || p.y >= maxrow() || p.x < 0 || p.x >= COLS);
}

//do_search: Player gropes about him to find hidden things.
bool do_search()
{
    if (game->hero().is_blind())
        return true;

    int ey = game->hero().m_position.y + 1;
    int ex = game->hero().m_position.x + 1;
    for (int y = game->hero().m_position.y - 1; y <= ey; y++) {
        for (int x = game->hero().m_position.x - 1; x <= ex; x++)
        {
            Coord pos = { x, y };
            if ((pos == game->hero().m_position) || offmap(pos))
                continue;
            game->level().search(pos);
        }
    }

    return true;
}

//do_go_down_stairs: He wants to go down a level
bool do_go_down_stairs()
{
    if (game->level().get_tile(game->hero().m_position) != STAIRS && !game->wizard().jump_levels())
        msg("I see no way down");
    else {
        next_level();
        game->level().new_level(true);
    }
    return false;
}

//do_go_up_stairs: He wants to go up a level
bool do_go_up_stairs()
{
    if (game->level().get_tile(game->hero().m_position) == STAIRS || game->wizard().jump_levels()) {
        if (game->hero().has_amulet() || game->wizard().jump_levels()) {
            if (prev_level() == 0)
                total_winner();
            game->level().new_level(true);
            msg("you feel a wrenching sensation in your gut");
        }
        else
            msg("your way is magically blocked");
    }
    else msg("I see no way up");

    return false;
}

//do_call: Allow a user to call a potion, scroll, or ring something
bool do_call()
{
    Item *obj = get_item("call", CALLABLE);
    if (!obj)
        return false;

    ItemClass* item_class = obj->item_class();
    if (!item_class){
        msg("you can't call that anything");
        return false;
    }

    if (item_class->is_discovered(obj->m_which)) {
        msg("that has already been identified");
        return false;
    }

    std::string called = item_class->get_guess(obj->m_which);
    if (called.empty())
        called = item_class->get_identifier(obj->m_which);
    msg("Was called \"%s\"", called.c_str());

    msg("what do you want to call it? ");
    getinfo(prbuf, MAXNAME);
    if (*prbuf && *prbuf != ESCAPE)
        item_class->set_guess(obj->m_which, prbuf);
    msg("");

    return false;
}

//prompt player for definition of macro
bool do_record_macro()
{
    char buffer[MACROSZ];
    char* buf = buffer;

    memset(buf, 0, MACROSZ);
    char *cp = prbuf;

    msg("F9 was %s, enter new macro: ", game->macro.c_str());
    if (getinfo(prbuf, MACROSZ - 1) != ESCAPE)
        do {
            if (*cp != CTRL('F'))
                *buf++ = *cp;
        } while (*cp++);

        msg("");
        clear_typeahead_buffer();
        game->macro = buffer;

        return false;
}
