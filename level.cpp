#include <stdlib.h>

#include "rogue.h"
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
#include "output_interface.h"
#include "game_state.h"
#include "hero.h"
#include "passages.h"
#include "pack.h"
#include "things.h"
#include "potions.h"
#include "monster.h"
#include "amulet.h"

//One for each passage
Room passages[MAXPASS] =
{
  { 0, {0, 0}, {0, 0}, {0, 0}, 0, IS_GONE | IS_DARK, 0, 0 },
  { 1, {0, 0}, {0, 0}, {0, 0}, 0, IS_GONE | IS_DARK, 0, 0 },
  { 2, {0, 0}, {0, 0}, {0, 0}, 0, IS_GONE | IS_DARK, 0, 0 },
  { 3, {0, 0}, {0, 0}, {0, 0}, 0, IS_GONE | IS_DARK, 0, 0 },
  { 4, {0, 0}, {0, 0}, {0, 0}, 0, IS_GONE | IS_DARK, 0, 0 },
  { 5, {0, 0}, {0, 0}, {0, 0}, 0, IS_GONE | IS_DARK, 0, 0 },
  { 6, {0, 0}, {0, 0}, {0, 0}, 0, IS_GONE | IS_DARK, 0, 0 },
  { 7, {0, 0}, {0, 0}, {0, 0}, 0, IS_GONE | IS_DARK, 0, 0 },
  { 8, {0, 0}, {0, 0}, {0, 0}, 0, IS_GONE | IS_DARK, 0, 0 },
  { 9, {0, 0}, {0, 0}, {0, 0}, 0, IS_GONE | IS_DARK, 0, 0 },
  { 10, {0, 0}, {0, 0}, {0, 0}, 0, IS_GONE | IS_DARK, 0, 0 },
  { 11, {0, 0}, {0, 0}, {0, 0}, 0, IS_GONE | IS_DARK, 0, 0 }
};

int s_level = 1;
int s_max_level = 1;

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
                return monster->type;
            return monster->disguise;
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

void Level::search(Coord pos)
{
    if (!is_real(pos)) {
        switch (get_tile(pos))
        {
        case VWALL: case HWALL: case ULWALL: case URWALL: case LLWALL: case LRWALL:
            if (rnd(5) != 0) break;
            set_tile(pos, DOOR);
            set_flag(pos, F_REAL);
            game->repeat_cmd_count = game->modifiers.m_running = false;
            break;
        case FLOOR:
            if (rnd(2) != 0) break;
            set_tile(pos, TRAP);
            set_flag(pos, F_REAL);
            game->repeat_cmd_count = game->modifiers.m_running = false;
            msg("you found %s", tr_name(get_trap_type(pos)));
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
        if (monster->pos.x == p.x && monster->pos.y == p.y)
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
    game->screen().mvaddch(p, get_tile(p));
}


#define TREAS_ROOM  20 //one chance in TREAS_ROOM for a treasure room
#define MAXTREAS  10 //maximum number of treasures in a treasure room
#define MINTREAS  2 //minimum number of treasures in a treasure room
#define MAXTRIES  10 //max number of tries to put down a monster

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
        free_item_list(monster->pack);
    }
    free_agent_list(monsters);
    //Throw away stuff left on the previous level (if anything)
    free_item_list(items);

    do_rooms(); //Draw rooms
    if (max_level() == 1)
    {
        game->screen().clear();
    }
    if (do_implode)
        game->screen().implode();
    update_status_bar();
    do_passages(); //Draw passages
    game->no_food++;
    put_things(); //Place objects (if any)

    //Place the staircase down.
    find_empty_location(&pos, false); //TODO: seed used to change after 100 failed attempts
    set_tile(pos, STAIRS);

    //Place the traps
    if (rnd(10) < get_level())
    {
        ntraps = rnd(get_level() / 4) + 1;
        if (ntraps > MAXTRAPS) ntraps = MAXTRAPS;
        i = ntraps;
        while (i--)
        {
            find_empty_location(&pos, false);
            byte type = rnd(NTRAPS);
            if (!game->wizard().no_traps()) {
                unset_flag(pos, F_REAL);
                set_flag(pos, type);
            }
        }
    }
    do
    {
        find_empty_location(&game->hero().pos, true);
    } while (!(get_flags(game->hero().pos) & F_REAL));  //don't place hero on a trap

    reset_msg_position();  //todo: rest probably belongs somewhere else
    //unhold when you go down just in case
    game->hero().set_is_held(false);
    enter_room(game->hero().pos);
    game->screen().mvaddch(game->hero().pos, PLAYER);
    game->oldpos = game->hero().pos;
    game->oldrp = game->hero().room;
    if (game->hero().detects_others())
        turn_see(false);
}

//put_things: Put potions and scrolls on this level
void Level::put_things()
{
    int i = 0;
    Coord tp;

    //Once you have found the amulet, the only way to get new stuff is to go down into the dungeon.
    //This is real unfair - I'm going to allow one thing, that way the poor guy will get some food.
    if (game->hero().had_amulet() && get_level() < max_level())
        i = MAXOBJ - 1;
    else
    {
        //If he is really deep in the dungeon and he hasn't found the amulet yet, put it somewhere on the ground
        //Check this first so if we are out of memory the guy has a hope of getting the amulet
        if (get_level() >= AMULETLEVEL && !game->hero().had_amulet()) //mdk: amulet doesn't appear again if you lose it
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
            set_tile(tp, cur->type);
            cur->pos = tp;
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
    spots = (room->size.y - 2)*(room->size.x - 2) - MINTREAS;
    if (spots > (MAXTREAS - MINTREAS)) spots = (MAXTREAS - MINTREAS);
    num_monst = nm = rnd(spots) + MINTREAS;
    while (nm--)
    {
        do {
            rnd_pos(room, &pos);
        } while (!isfloor(get_tile(pos)));
        item = Item::CreateItem();
        item->pos = pos;
        items.push_front(item);
        set_tile(pos, item->type);
    }
    //fill up room with monsters from the next level down
    if ((nm = rnd(spots) + MINTREAS) < num_monst + 2) nm = num_monst + 2;
    spots = (room->size.y - 2)*(room->size.x - 2);
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
            monster = Monster::CreateMonster(randmonster(false, get_level() + 1), &pos, get_level() + 1);
            if (game->invalid_position)
                debug("treasure roomm bailout");
            monster->set_is_mean(true); //no sloughers in THIS room
            monster->give_pack();

        }
    }
}



int get_level()
{
    return s_level;
}

int next_level()
{
    ++s_level;
    if (s_level > s_max_level)
        s_max_level = s_level;
    return s_level;
}

int prev_level()
{
    return --s_level;
}

int max_level()
{
    return s_max_level;
}

int rnd_gold()
{
    return (rnd(50 + 10 * s_level) + 2);
}

int maxrow()
{
    int lines = game->screen().lines();
    if (game->screen().small_screen_mode())
        return lines - 3;
    return lines - 2;
}
