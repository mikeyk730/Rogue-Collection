#include <stdlib.h>
#include <sstream>
#include <algorithm>
#include <cstring>
#include "random.h"
#include "game_state.h"
#include "level.h"
#include "misc.h"
#include "io.h"
#include "main.h"
#include "room.h"
#include "rooms.h"
#include "list.h"
#include "agent.h"
#include "monsters.h"
#include "output_shim.h"
#include "game_state.h"
#include "hero.h"
#include "pack.h"
#include "things.h"
#include "potion.h"
#include "monster.h"
#include "amulet.h"
#include "text.h"

void Level::clear_level()
{
    memset(the_level, ' ', (MAXLINES - 3)*MAXCOLS);
    memset(the_flags, F_REAL, (MAXLINES - 3)*MAXCOLS);
}

int INDEX(Coord p)
{
    return ((p.x*(maxrow() - 1)) + p.y - 1);
}

bool Level::is_floor_or_passage(Coord p)
{
    byte ch = get_tile_or_monster(p, true);
    return (ch == FLOOR || ch == PASSAGE);
}

byte Level::get_tile(Coord p, bool consider_monsters, bool mimic_as_monster)
{
    if (consider_monsters)
    {
        Monster *monster = monster_at(p);
        if (monster) {
            if (mimic_as_monster)
                return monster->m_type;
            return monster->m_disguise;
        }
    }
    return the_level[INDEX(p)];
}

byte Level::get_tile_or_monster(Coord p, bool mimic_as_monster)
{
    return get_tile(p, true, mimic_as_monster);
}


void Level::set_tile(Coord p, byte c)
{
    the_level[INDEX(p)] = c;
}

byte Level::get_flags(Coord p)
{
    return the_flags[INDEX(p)];
}

void Level::set_flag(Coord p, byte f)
{
    the_flags[INDEX(p)] |= f;
}

void Level::unset_flag(Coord p, byte f)
{
    the_flags[INDEX(p)] &= ~f;
}

void Level::copy_flags(Coord p, byte f)
{
    the_flags[INDEX(p)] = f;
}

bool Level::is_passage(Coord p)
{
    return (get_flags(p) & F_PASS) != 0;
}

bool Level::is_maze(Coord p)
{
    return (get_flags(p) & F_MAZE) != 0;
}

bool Level::is_real(Coord p)
{
    return (get_flags(p) & F_REAL) != 0;
}

int Level::get_passage_num(Coord p)
{
    return get_flags(p) & F_PNUM;
}

int Level::get_trap_type(Coord p)
{
    return get_flags(p) & F_TMASK;
}

bool Level::use_standout(Coord p, unsigned char c)
{
    //The current character used for IBM ARMOR doesn't look right in inverse
    if (c == ARMOR)
        return false;

    return (is_maze(p) || is_passage(p)) && c != ' ' && c != PASSAGE;
}

void Level::search(Coord pos)
{
    if (!is_real(pos)) {
        switch (get_tile(pos))
        {
        case VWALL: case HWALL: case ULWALL: case URWALL: case LLWALL: case LRWALL:
            if (rnd(5) != 0) break;
            set_tile(pos, DOOR);
            set_flag(pos, F_REAL);
            game->cancel_repeating_cmd();
            game->stop_run_cmd();
            break;
        case FLOOR:
            if (rnd(2) != 0) break;
            set_tile(pos, TRAP);
            set_flag(pos, F_REAL);
            game->cancel_repeating_cmd();
            game->stop_run_cmd();
            msg(get_text(text_found_trap), tr_name(get_trap_type(pos)));
            break;
        }
    }
}

Room * Level::get_passage(Coord pos)
{
    return &passages[get_passage_num(pos)];
}

//monster_at: returns pointer to monster at coordinate. if no monster there return NULL
Monster* Level::monster_at(Coord p, bool include_disguised)
{
    Monster* monster;
    for (auto it = monsters.begin(); it != monsters.end(); ++it) {
        monster = *it;
        if (monster->position().x == p.x && monster->position().y == p.y)
        {
            if (monster->is_disguised() && !include_disguised)
                return nullptr;
            return monster;
        }
    }
    return nullptr;
}

void Level::draw_char(Coord p)
{
    game->screen().add_tile(p, get_tile(p));
}

void Level::show_map(bool reveal_interior)
{
    int x, y;
    byte ch;
    Monster* monster;

    const int COLS = game->screen().columns();
    for (y = 1; y < maxrow(); y++) {
        for (x = 0; x < COLS; x++)
        {
            Coord p = { x, y };
            switch (ch = get_tile(p))
            {
            case VWALL: case HWALL: case ULWALL: case URWALL: case LLWALL: case LRWALL:
                if (!(is_real(p))) {
                    ch = DOOR;
                    set_tile(p, DOOR);
                    unset_flag(p, F_REAL);
                }
                break;
            case FLOOR:
                if (!reveal_interior)
                    ch = ' ';
                else if (!(is_real(p))) {
                    ch = TRAP;
                    set_tile(p, TRAP);
                    unset_flag(p, F_REAL);
                }
                break;
            case DOOR: case PASSAGE: case STAIRS:
                break;
            default:
                if (!reveal_interior)
                    ch = ' ';
            }

            if (ch != ' ') {
                if ((monster = monster_at(p)) != NULL)
                    if (monster->tile_beneath() == ' ')
                        monster->set_tile_beneath(ch);
            }

            if (!reveal_interior && ch == DOOR)
            {
                game->screen().move(y, x);
                if (game->screen().curch() != DOOR)
                    game->screen().standout();
            }
            if (ch != ' ')
                game->screen().add_tile(p, ch);
            game->screen().standend();
        }
    }
}

#define TREAS_ROOM  20 //one chance in TREAS_ROOM for a treasure room
#define MAXTREAS  10 //maximum number of treasures in a treasure room
#define MINTREAS  2 //minimum number of treasures in a treasure room
#define MAXTRIES  10 //max number of tries to put down a monster

Level::Level()
{
}

void Level::new_level(int do_implode)
{
    int i, ntraps;
    Agent *monster;
    Coord pos;

    //Monsters only get displayed when you move so start a level by having the poor guy rest. God forbid he lands next to a monster!

    //Clean things off from last level
    clear_level();

    //Free up the monsters on the last level
    for (auto it = monsters.begin(); it != monsters.end(); ++it) {
        monster = *it;
        free_item_list(monster->m_pack);
    }
    free_agent_list(monsters);
    //Throw away stuff left on the previous level (if anything)
    free_item_list(items);

    std::ostringstream ss;
    ss << "Entering level " << game->get_level() << ", seed:" << std::hex << g_random->get_seed();
    game->log("level", ss.str());

    do_rooms(); //Draw rooms

    if (do_implode) {
        if (!game->options.act_like_v1_1() && game->options.interactive())
            game->screen().implode();
        else
            game->screen().blot_out(0, 0, maxrow() - 1, game->screen().columns() - 1);
    }

    update_status_bar();
    do_passages(); //Draw passages
    game->no_food++;
    put_things(); //Place objects (if any)

    //Place the staircase down.
    find_empty_location(&pos, false); //TODO: seed used to change after 100 failed attempts
    set_tile(pos, STAIRS);

    //Place the traps
    if (rnd(10) < game->get_level())
    {
        ntraps = rnd(game->get_level() / 4) + 1;
        if (ntraps > MAXTRAPS) ntraps = MAXTRAPS;
        i = ntraps;
        while (i--)
        {
            find_empty_location(&pos, false);
            byte type = rnd(NTRAPS);
            if (!game->wizard().no_traps()) {
                unset_flag(pos, F_REAL);
                if (game->options.trap_bugfix()) {
                    //mdk:bugfix:unset trap type to avoid ORing two different traps together.  (weird trap msg)
                    unset_flag(pos, F_TMASK);
                }
                else if (get_trap_type(pos)) {
                    std::ostringstream ss;
                    ss << "Trap (" << get_trap_type(pos) << ") exisits at " << pos.x << "," << pos.y << ".  Trying to set " << int(type);
                    game->log("warning", ss.str());
                }
                set_flag(pos, type);
            }
        }
    }

    Coord hero_position;
    do
    {
        find_empty_location(&hero_position, true);
    } while (!is_real(hero_position));  //don't place hero on a trap
    game->hero().set_position(hero_position);

    reset_msg_position();
    enter_room(game->hero().position());
    game->screen().add_tile(game->hero().position(), PLAYER);

    game->wizard().on_new_level();
    game->hero().on_new_level();
}

//put_things: Put potions and scrolls on this level
void Level::put_things()
{
    int i = 0;
    Coord tp;

    //Once you have found the amulet, the only way to get new stuff is to go down into the dungeon.
    //This is real unfair - I'm going to allow one thing, that way the poor guy will get some food.
    if (game->hero().had_amulet() && game->get_level() < game->max_level())
        i = MAXOBJ - 1;
    else
    {
        //If he is really deep in the dungeon and he hasn't found the amulet yet, put it somewhere on the ground
        //Check this first so if we are out of memory the guy has a hope of getting the amulet
        if (game->get_level() >= AMULETLEVEL && !game->hero().had_amulet()) //mdk: amulet doesn't appear again if you lose it
        {
            Item* amulet = new Amulet();
            items.push_front(amulet);

            //Put it somewhere
            find_empty_location(&tp, true);
            set_tile(tp, AMULET);
            amulet->set_position(tp);
        }
        //check for treasure rooms, and if so, put it in.
        if (rnd(TREAS_ROOM) == 0)
            treas_room();
    }

    //Do MAXOBJ attempts to put things on a level
    for (; i < MAXOBJ; i++) {
        if (rnd(100) < 35)
        {
            //Pick a new object and link it in the list
            Item* cur = Item::CreateItem();
            items.push_front(cur);
            //Put it somewhere
            find_empty_location(&tp, false);
            set_tile(tp, cur->m_type);
            cur->set_position(tp);
        }
    }
}

//treas_room: Add a treasure room
void Level::treas_room()
{
    int nm;
    Item *item;
    Monster* monster;
    struct Room *room;
    int spots, num_monst;
    Coord pos;

    room = rnd_room();
    spots = (room->m_size.y - 2)*(room->m_size.x - 2) - MINTREAS;
    if (spots > (MAXTREAS - MINTREAS)) spots = (MAXTREAS - MINTREAS);
    num_monst = nm = rnd(spots) + MINTREAS;
    while (nm--)
    {
        do {
            rnd_pos(room, &pos);
        } while (!isfloor(get_tile(pos)));
        item = Item::CreateItem();
        item->set_position(pos);
        items.push_front(item);
        set_tile(pos, item->m_type);
    }
    //fill up room with monsters from the next level down
    if ((nm = rnd(spots) + MINTREAS) < num_monst + 2) nm = num_monst + 2;
    spots = (room->m_size.y - 2)*(room->m_size.x - 2);
    if (nm > spots) nm = spots;
    while (nm--)
    {
        for (spots = 0; spots < MAXTRIES; spots++)
        {
            rnd_pos(room, &pos);
            if (isfloor(get_tile(pos)) && monster_at(pos) == NULL) break;
        }
        if (spots != MAXTRIES)
        {
            monster = Monster::CreateMonster(randmonster(false, game->get_level() + 1), &pos, game->get_level() + 1);
            if (game->invalid_position)
                debug("treasure roomm bailout");
            monster->set_is_mean(true); //no sloughers in THIS room
            monster->give_pack();
        }
    }
}

void Level::illuminate_rooms()
{
    for (Room* room = rooms; room < &rooms[MAXROOMS]; room++)
        room->set_dark(false);
}

bool Level::reveal_magic()
{
    bool discovered = false;
    for (auto i = items.begin(); i != items.end(); ++i)
    {
        Item* item = *i;
        if (item->is_magic())
        {
            discovered = true;
            game->screen().add_tile(item->position(), get_magic_char(item));
        }
    }
    for (auto m = monsters.begin(); m != monsters.end(); ++m) {
        Agent* monster = *m;
        for (auto i = monster->m_pack.begin(); i != monster->m_pack.end(); ++i)
        {
            Item* item = *i;
            if (item->is_magic())
            {
                discovered = true;
                game->screen().add_tile(monster->position(), MAGIC);
            }
        }
    }
    return discovered;
}

bool Level::detect_monsters(bool enable)
{
    bool revealed = false;

    for (auto i = monsters.begin(); i != monsters.end(); ++i) {
        Monster* monster(*i);
        byte screen_tile = game->screen().mvinch(monster->position());

        if (enable)
        {
            if (!game->hero().can_see_monster(monster)) {
                if (screen_tile != monster->m_type) {
                    revealed = true;
                    monster->set_tile_beneath(screen_tile);
                }
                game->screen().standout();
            }
            game->screen().add_tile(monster->m_type);
            game->screen().standend();
        }
        else {
            //if we can't see the monster, replace it with whatever is beneath it
            if (!game->hero().can_see_monster(monster) && monster->has_tile_beneath())
                game->screen().add_tile(monster->tile_beneath());
        }
    }
    return revealed;
}

bool Level::has_monsters() const
{
    return !monsters.empty();
}

int rnd_gold()
{
    return (rnd(50 + 10 * game->get_level()) + 2);
}

int maxrow()
{
    int lines = game->screen().lines();
    if (in_small_screen_mode())
        return lines - 3;
    return lines - 2;
}

//aggravate_monsters: Aggravate all the monsters on this level
void Level::aggravate_monsters()
{
    std::for_each(monsters.begin(), monsters.end(), [](Monster *monster) {
        monster->start_run();
    });
}
