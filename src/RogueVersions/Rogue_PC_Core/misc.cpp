//All sorts of miscellaneous routines
//misc.c       1.4             (A.I. Design)   12/14/84
#include <sstream>
#include <stdio.h>
#include <cstring>
#include "random.h"
#include "game_state.h"
#include "io.h"
#include "daemons.h"
#include "misc.h"
#include "monsters.h"
#include "pack.h"
#include "output_shim.h"
#include "rip.h"
#include "list.h"
#include "mach_dep.h"
#include "main.h"
#include "fight.h"
#include "daemon.h"
#include "level.h"
#include "ring.h"
#include "scroll.h"
#include "potion.h"
#include "stick.h"
#include "armor.h"
#include "hero.h"
#include "room.h"
#include "monster.h"

const int MACROSZ = 41;

//tr_name: Print the name of a trap
const char *tr_name(byte type)
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

void darken_position(Coord pos, int hero_passage)
{
    //blot out floor if in a dark room
    byte ch = game->screen().mvinch(pos);
    if (ch == FLOOR)
    {
        Room* old_room = game->hero().previous_room();
        if (old_room->is_dark() && !old_room->is_gone())
            game->screen().add_tile(' ');
    }
    //darken passage
    //mdk:bugfix: originally items dropped beside a door and inside a passage would never be erased
    bool maze_or_passage = game->level().is_maze(pos) || game->level().is_passage(pos);
    if (maze_or_passage && ch != STAIRS && !game->wizard().see_all()) {
        //mdk:bugfix: this code originally blotted out detected monsters with a passage.
        //I added the check below to correct this.
        if (!(isupper(toascii(ch)) && game->hero().detects_others())) {
            game->screen().add_tile(PASSAGE);
        }
    }
}

void darken_area()
{
    if (game->hero().is_blind())
        return;

    Coord hero_pos = game->hero().position();
    int hero_passage = game->level().get_passage_num(hero_pos);

    Coord old_pos = game->hero().previous_position();
    for (int x = old_pos.x - 1; x <= (old_pos.x + 1); x++) {
        for (int y = old_pos.y - 1; y <= (old_pos.y + 1); y++)
        {
            Coord pos = { x, y };
            if (pos == hero_pos || offmap(pos))
                continue;

            darken_position(pos, hero_passage);
        }
    }
}

void reveal_position(const Coord pos, const bool wakeup, int* passcount)
{
    Coord hero_pos = game->hero().position();

    //we can reveal the 8 directions around the player if he has sight, but
    //only the player's position if he's blind
    if (!game->hero().is_blind() && pos == hero_pos) {
        return;
    }
    else if (game->hero().is_blind() && pos != hero_pos) {
        return;
    }

    bool is_passage = game->level().is_passage(pos);
    bool is_maze = game->level().is_maze(pos);
    int passage_number = game->level().get_passage_num(pos);

    byte tile = game->level().get_tile(pos);
    byte tile_under_hero = game->level().get_tile(hero_pos);

    //No Doors
    if (tile_under_hero != DOOR && tile != DOOR) {
        bool hero_in_passage = game->level().is_passage(hero_pos);
        bool hero_in_maze = game->level().is_maze(hero_pos);
        int hero_passage_number = game->level().get_passage_num(hero_pos);

        //Either hero or other in a passage
        if (hero_in_passage != is_passage)
        {
            //Neither is in a maze
            if (!hero_in_maze && !is_maze)
                return;
        }
        //Not in same passage
        else if (is_passage && passage_number != hero_passage_number) {
            return;
        }
    }

    Monster* monster = game->level().monster_at(pos);
    if (monster) {
        if (game->hero().detects_others() && monster->is_invisible())
        {
            if (game->in_smart_run_mode())
                game->stop_run_cmd();
            return;
        }
        else
        {
            if (wakeup)
                wake_monster(pos);
            if (monster->tile_beneath() != ' ' || (!game->hero().room()->is_dark() && !game->hero().is_blind()))
                monster->reload_tile_beneath();
            if (game->hero().can_see_monster(monster))
                tile = monster->m_disguise;
        }
    }

    //Draw the tile.
    if (game->level().use_standout(pos, tile))
        game->screen().standout();
    game->screen().add_tile(pos, tile);
    game->screen().standend();

    // determine whether we need to stop a running player
    if (game->in_smart_run_mode())
    {
        const int ey = hero_pos.y + 1;
        const int ex = hero_pos.x + 1;
        const int sx = hero_pos.x - 1;
        const int sy = hero_pos.y - 1;

        int sumhero = hero_pos.y + hero_pos.x;
        int diffhero = hero_pos.y - hero_pos.x;

        switch (game->run_character)
        {
        case 'h': if (pos.x == ex) return; break;
        case 'j': if (pos.y == sy) return; break;
        case 'k': if (pos.y == ey) return; break;
        case 'l': if (pos.x == sx) return; break;
        case 'y': if ((pos.y + pos.x) - sumhero >= 1) return; break;
        case 'u': if ((pos.y - pos.x) - diffhero >= 1) return; break;
        case 'n': if ((pos.y + pos.x) - sumhero <= -1) return; break;
        case 'b': if ((pos.y - pos.x) - diffhero <= -1) return; break;
        }
        switch (tile)
        {
        case DOOR:
            //stop at a door
            if (pos.x == hero_pos.x || pos.y == hero_pos.y)
                game->stop_run_cmd();
            break;
        case PASSAGE:
            //stop at a branching passage
            if (pos.x == hero_pos.x || pos.y == hero_pos.y) {
                ++(*passcount);
                if (*passcount > 1) {
                    game->stop_run_cmd();
                }
            }
            break;
        case FLOOR: case VWALL: case HWALL: case ULWALL: case URWALL: case LLWALL: case LRWALL: case ' ':
            //don't stop
            break;
        default:
            //stop at items, doors, monsters, etc.
            game->stop_run_cmd();
            break;
        }
    }
}

void reveal_area(bool wakeup)
{
    int passcount = 0;

    Coord hero_pos = game->hero().position();
    int ey = hero_pos.y + 1;
    int ex = hero_pos.x + 1;
    int sx = hero_pos.x - 1;
    int sy = hero_pos.y - 1;
    const int COLS = game->screen().columns();
    for (int y = sy; y <= ey; y++) {
        if (!(y > 0 && y < maxrow()))
            continue;
        for (int x = sx; x <= ex; x++) {
            if (x <= 0 || x >= COLS)
                continue;
            Coord pos = { x, y };
            reveal_position(pos, wakeup, &passcount);
        }
    }
}

//look: A quick glance all around the player
void look(bool wakeup) //todo: learn this function
{
    //if the hero has moved
    if (game->hero().has_moved()) {
        //darken the area around his old position (except for current position)
        darken_area();
        game->hero().update_position();  //save the old room and position
    }

    //reveal the area around the player
    reveal_area(wakeup);

    // draw the player -- highlight him if he's in a maze/passage or hit a teleport trap
    Coord hero_pos = game->hero().position();
    if (game->level().use_standout(hero_pos, PLAYER) || game->hero().sprung_teleport_trap())
        game->screen().standout();
    game->screen().add_tile(hero_pos, PLAYER);
    game->screen().standend();

    // todo: why not when trap is sprung?
    if (game->hero().sprung_trap()) {
        game->screen().play_sound("trap");
        //alert();
        game->hero().reset_sprung();
    }
}

//find_obj: Find the unclaimed object at y, x
Item* find_obj(Coord p, bool expect_item)
{
    for (auto it = game->level().items.begin(); it != game->level().items.end(); ++it) {
        Item* op = *it;
        if (op->position().y == p.y && op->position().x == p.x)
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
    case 'h': case 'H':
        cp->y = 0; cp->x = -1;
        break;
    case 'j': case 'J':
        cp->y = 1; cp->x = 0;
        break;
    case 'k': case 'K':
        cp->y = -1; cp->x = 0;
        break;
    case 'l': case 'L':
        cp->y = 0; cp->x = 1;
        break;
    case 'y': case 'Y':
        cp->y = -1; cp->x = -1;
        break;
    case 'u': case 'U':
        cp->y = -1; cp->x = 1;
        break;
    case 'b': case 'B':
        cp->y = 1; cp->x = -1;
        break;
    case 'n': case 'N':
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
    if (game->options.prompt_for_help())
    {
        msg("what do you want identified? ");
        char ch = readchar();
        clear_msg();

        const char* str = "unknown character";
        if (ch >= 'A' && ch <= 'Z') {
            str = get_monster_name(ch);
        }

        msg("'%s': %s", unctrl(ch), str);
        return;
    }

    bool is_objs(helpscr == helpobjs);
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
        const char* str = *helpscr++;
        if (is_objs) {
            unsigned char ch = *str++;
            game->screen().add_tile(ch);
            if (ch == 'A' || ch == '$') {
                game->screen().add_text(*str++);
                game->screen().add_tile(*str++);
            }
        }
        game->screen().addstr(str);

        //decide if we need print a continue type message
        if ((*helpscr == 0) || isfull)
        {
            if (*helpscr == 0)
                game->screen().mvaddstr({ 0, 24 }, "--press space to continue--");
            else if (in_small_screen_mode())
                game->screen().mvaddstr({ 0, 24 }, "--Space for more, Esc to continue--");
            else
                game->screen().mvaddstr({ 0, 24 }, "--Press space for more, Esc to continue--");
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

    Coord hero_pos = game->hero().position();
    int ey = hero_pos.y + 1;
    int ex = hero_pos.x + 1;
    for (int y = hero_pos.y - 1; y <= ey; y++) {
        for (int x = hero_pos.x - 1; x <= ex; x++)
        {
            Coord pos = { x, y };
            if ((pos == hero_pos) || offmap(pos))
                continue;
            game->level().search(pos);
        }
    }

    return true;
}

//do_go_down_stairs: He wants to go down a level
bool do_go_down_stairs()
{
    if (game->level().get_tile(game->hero().position()) != STAIRS && !game->wizard().jump_levels())
        msg("I see no way down");
    else {
        game->screen().play_sound("stairs");
        game->next_level();
        game->level().new_level(true);
    }
    return false;
}

//do_go_up_stairs: He wants to go up a level
bool do_go_up_stairs()
{
    if (game->level().get_tile(game->hero().position()) == STAIRS || game->wizard().jump_levels()) {
        if (game->hero().has_amulet() || game->wizard().jump_levels()) {
            if (game->prev_level() == 0)
                total_winner();
            game->level().new_level(true);
            msg("you feel a wrenching sensation%s", noterse(" in your gut"));
            game->screen().play_sound("stairs");
        }
        else
            msg("your way is magically blocked");
    }
    else msg("I see no way up");

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
    if (getinfo(prbuf, MACROSZ - 1) != ESCAPE) {
        do {
            if (*cp != CTRL('F'))
                *buf++ = *cp;
        } while (*cp++);

        clear_typeahead_buffer();
        game->macro = buffer;
    }
    msg("");
    return false;
}
