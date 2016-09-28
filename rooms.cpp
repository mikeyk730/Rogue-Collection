//Create the layout for the new level
//rooms.c     1.4 (A.I. Design)       12/16/84

#include <ctype.h>

#include "rogue.h"
#include "main.h"
#include "rooms.h"
#include "monsters.h"
#include "list.h"
#include "output_interface.h"
#include "maze.h"
#include "move.h"
#include "chase.h"
#include "misc.h"
#include "io.h"
#include "level.h"
#include "pack.h"
#include "room.h"
#include "game_state.h"
#include "hero.h"
#include "monster.h"

Room rooms[MAXROOMS]; //One for each room -- A level

bool isfloor(byte c) {
    return ((c) == FLOOR || (c) == PASSAGE);
}

//do_rooms: Create rooms and corridors with a connectivity graph
void do_rooms()
{
    int i;
    struct Room *room;
    Monster* monster;
    int left_out;
    Coord top;
    Coord bsze;
    Coord mp;
    int old_lev;
    int endline;

    endline = maxrow + 1;
    old_lev = get_level();
    //bsze is the maximum room size
    const int COLS = game->screen().columns();
    bsze.x = COLS / 3;
    bsze.y = endline / 3;
    //Clear things for a new level
    for (i = 0; i < MAXROOMS; i++) {
        room = &rooms[i];
        room->index = i;
        room->reset();
    }
    //Put the gone rooms, if any, on the level
    left_out = rnd(4);
    for (i = 0; i < left_out; i++)
    {
        do {
            room = rnd_room();
        } while (room->is_maze());
        room->set_gone();
        if (room->index > 2 && get_level() > 10 && rnd(20) < get_level() - 9)
            room->set_maze();
    }
    //dig and populate all the rooms on the level
    for (i = 0, room = rooms; i < MAXROOMS; room++, i++)
    {
        //Find upper left corner of box that this room goes in
        top.x = (i % 3)*bsze.x + 1;
        top.y = i / 3 * bsze.y;
        if (room->is_gone())
        {
            //If the gone room is a maze room, draw the maze and set the size equal to the maximum possible.
            if (room->is_maze()) { room->pos.x = top.x; room->pos.y = top.y; draw_maze(room); }
            else
            {
                //Place a gone room.  Make certain that there is a blank line for passage drawing.
                do
                {
                    room->pos.x = top.x + rnd(bsze.x - 2) + 1;
                    room->pos.y = top.y + rnd(bsze.y - 2) + 1;
                    room->size.x = -COLS;
                    room->size.x = -endline;
                } while (!(room->pos.y > 0 && room->pos.y < endline - 1));
            }
            continue;
        }
        // dark rooms more common as we go down
        if (rnd(10) < (get_level() - 1) && !game->wizard().no_dark_rooms()) {
            room->set_dark(true);
        }
        //Find a place and size for a random room
        do
        {
            room->size.x = rnd(bsze.x - 4) + 4;
            room->size.y = rnd(bsze.y - 4) + 4;
            room->pos.x = top.x + rnd(bsze.x - room->size.x);
            room->pos.y = top.y + rnd(bsze.y - room->size.y);
        } while (room->pos.y == 0);
        draw_room(room);
        //Put the gold in
        if ((rnd(2) == 0) && (!game->hero().had_amulet() || (get_level() >= max_level())))
        {
            room->gold_val = rnd_gold();
            Item *gold = new Gold(room->gold_val);
            game->level().items.push_front(gold);

            while (1)
            {
                byte gch;
                rnd_pos(room, &room->gold);
                gch = game->level().get_tile(room->gold);
                if (isfloor(gch))
                    break;
            }
            game->level().set_tile(room->gold, GOLD);
            gold->set_position(room->gold);
        }
        //Put the monster in
        if (rnd(100) < (room->gold_val > 0 ? 80 : 25))
        {
            byte mch;
            do {
                rnd_pos(room, &mp);
                mch = game->level().get_tile_or_monster(mp);
            } while (!isfloor(mch));
            monster = Monster::CreateMonster(randmonster(false, get_level()), &mp, get_level());
            monster->give_pack();

        }
    }
}

//draw_room: Draw a box around a room and lay down the floor
void draw_room(struct Room *room)
{
    int y, x;

    //Here we draw normal rooms, one side at a time
    vert(room, room->pos.x); //Draw left side
    vert(room, room->pos.x + room->size.x - 1); //Draw right side
    horiz(room, room->pos.y); //Draw top
    horiz(room, room->pos.y + room->size.y - 1); //Draw bottom
    game->level().set_tile(room->pos, ULWALL);
    game->level().set_tile({ room->pos.x + room->size.x - 1,room->pos.y }, URWALL);
    game->level().set_tile({ room->pos.x,room->pos.y + room->size.y - 1 }, LLWALL);
    game->level().set_tile({ room->pos.x + room->size.x - 1,room->pos.y + room->size.y - 1 }, LRWALL);
    //Put the floor down
    for (y = room->pos.y + 1; y < room->pos.y + room->size.y - 1; y++)
        for (x = room->pos.x + 1; x < room->pos.x + room->size.x - 1; x++)
            game->level().set_tile({ x, y }, FLOOR);
}

//vert: Draw a vertical line
void vert(struct Room *room, int startx)
{
    int y;

    for (y = room->pos.y + 1; y <= room->size.y + room->pos.y - 1; y++)
        game->level().set_tile({ startx,y }, VWALL);
}

//horiz: Draw a horizontal line
void horiz(struct Room *room, int starty)
{
    int x;

    for (x = room->pos.x; x <= room->pos.x + room->size.x - 1; x++)
        game->level().set_tile({ x,starty }, HWALL);
}

//rnd_pos: Pick a random spot in a room
void rnd_pos(struct Room *room, Coord *cp)
{
    cp->x = room->pos.x + rnd(room->size.x - 2) + 1;
    cp->y = room->pos.y + rnd(room->size.y - 2) + 1;
}

//enter_room: Code that is executed whenever you appear in a room
void enter_room(Coord *cp)
{
    struct Room *room;
    int y, x;
    Monster* monster;

    room = game->hero().room = get_room_from_position(cp);
    if (invalid_position || (room->is_gone() && (room->is_maze()) == 0))
    {
        debug("in a gone room");
        return;
    }
    door_open(room);
    if (!(room->is_dark()) && !game->hero().is_blind() && !(room->is_maze()))
        for (y = room->pos.y; y < room->size.y + room->pos.y; y++)
        {
            game->screen().move(y, room->pos.x);
            for (x = room->pos.x; x < room->size.x + room->pos.x; x++)
            {
                //Displaying monsters is all handled in the chase code now
                monster = game->level().monster_at({ x, y });
                if (monster == NULL || !game->hero().can_see_monster(monster))
                    game->screen().addch(game->level().get_tile({ x, y }));
                else {
                    monster->oldch = game->level().get_tile({ x,y });
                    game->screen().addch(monster->disguise);
                }
            }
        }
}

//leave_room: Code for when we exit a room
void leave_room(Coord *cp)
{
    int y, x;
    struct Room *room;
    byte ch;

    room = game->hero().room;
    game->hero().room = game->level().get_passage(*cp);

    byte floor = ((room->is_dark()) && !game->hero().is_blind()) ? ' ' : FLOOR;
    if (room->is_maze())
        floor = PASSAGE;

    for (y = room->pos.y + 1; y < room->size.y + room->pos.y - 1; y++) {
        for (x = room->pos.x + 1; x < room->size.x + room->pos.x - 1; x++) {
            switch (ch = game->screen().mvinch(y, x))
            {
            case ' ': case PASSAGE: case TRAP: case STAIRS:
                break;

            case FLOOR:
                if (floor == ' ')
                    game->screen().addch(' ');
                break;

            default:
                //to check for monster, we have to strip out standout bit
                if (isupper(toascii(ch))) {
                    if (game->hero().detects_others()) {
                        game->screen().standout();
                        game->screen().addch(ch);
                        game->screen().standend();
                        break;
                    }
                    else {
                        Monster* m = game->level().monster_at({ x, y });
                        if (m) {
                            m->oldch = MDK;
                        }
                    }
                }
                game->screen().addch(floor);
            }
        }
    }
    door_open(room);
}

//get_room_from_position: Find what room some coordinates are in. NULL means they aren't in any room.
Room* get_room_from_position(Coord *pos)
{
    struct Room *room;

    for (room = rooms; room <= &rooms[MAXROOMS - 1]; room++)
        if (pos->x < room->pos.x + room->size.x &&
            room->pos.x <= pos->x &&
            pos->y < room->pos.y + room->size.y
            && room->pos.y <= pos->y)
            return room;

    if (game->level().is_passage(*pos))
        return game->level().get_passage(*pos);

    debug("in some bizarre place (%d, %d)", pos->y, pos->x);
    invalid_position = true;
    return NULL;
}

//rnd_room: Pick a room that is really there
Room* rnd_room()
{
    int rm;
    do {
        rm = rnd(MAXROOMS);
    } while (!((rooms[rm].is_gone()) == 0 || (rooms[rm].is_maze())));

    return &rooms[rm];
}

void find_empty_location(Coord* c, int consider_monsters)
{
    auto tile_getter = consider_monsters ? &Level::get_tile_or_monster : &Level::get_tile;

    do
    {
        rnd_pos(rnd_room(), c);
    } while (!isfloor((game->level().*tile_getter)(*c)));
}
