//Create the layout for the new level
//rooms.c     1.4 (A.I. Design)       12/16/84

#include <ctype.h>

#include "rogue.h"
#include "main.h"
#include "rooms.h"
#include "monsters.h"
#include "list.h"
#include "curses.h"
#include "maze.h"
#include "move.h"
#include "misc.h"
#include "io.h"
#include "level.h"
#include "pack.h"
#include "room.h"
#include "game_state.h"
#include "hero.h"
#include "monster.h"
#include "gold.h"

bool isfloor(byte c) {
    return ((c) == FLOOR || (c) == PASSAGE);
}

//do_rooms: Create rooms and corridors with a connectivity graph
void Level::do_rooms()
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

    endline = maxrow() + 1;
    old_lev = game->get_level();
    //bsze is the maximum room size
    const int COLS = game->screen().columns();
    bsze.x = COLS / 3;
    bsze.y = endline / 3;
    //Clear things for a new level
    for (i = 0; i < MAXROOMS; i++) {
        room = &rooms[i];
        room->m_index = i;
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
        if (room->m_index > 2 && game->get_level() > 10 && rnd(20) < game->get_level() - 9)
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
            if (room->is_maze()) { room->m_ul_corner.x = top.x; room->m_ul_corner.y = top.y; draw_maze(room); }
            else
            {
                //Place a gone room.  Make certain that there is a blank line for passage drawing.
                do
                {
                    room->m_ul_corner.x = top.x + rnd(bsze.x - 2) + 1;
                    room->m_ul_corner.y = top.y + rnd(bsze.y - 2) + 1;
                    room->m_size.x = -COLS;
                    room->m_size.x = -endline;
                } while (!(room->m_ul_corner.y > 0 && room->m_ul_corner.y < endline - 1));
            }
            continue;
        }
        // dark rooms more common as we go down
        if (rnd(10) < (game->get_level() - 1) && !game->wizard().no_dark_rooms()) {
            room->set_dark(true);
        }
        //Find a place and size for a random room
        do
        {
            room->m_size.x = rnd(bsze.x - 4) + 4;
            room->m_size.y = rnd(bsze.y - 4) + 4;
            room->m_ul_corner.x = top.x + rnd(bsze.x - room->m_size.x);
            room->m_ul_corner.y = top.y + rnd(bsze.y - room->m_size.y);
        } while (room->m_ul_corner.y == 0);
        draw_room(room);
        //Put the gold in
        if ((rnd(2) == 0) && (!game->hero().had_amulet() || (game->get_level() >= game->max_level())))
        {
            room->m_gold_val = rnd_gold();
            Item *gold = new Gold(room->m_gold_val);
            game->level().items.push_front(gold);

            while (1)
            {
                byte gch;
                rnd_pos(room, &room->m_gold_position);
                gch = game->level().get_tile(room->m_gold_position);
                if (isfloor(gch))
                    break;
            }
            game->level().set_tile(room->m_gold_position, GOLD);
            gold->set_position(room->m_gold_position);
        }
        //Put the monster in
        if (rnd(100) < (room->m_gold_val > 0 ? 80 : 25))
        {
            byte mch;
            do {
                rnd_pos(room, &mp);
                mch = game->level().get_tile_or_monster(mp);
            } while (!isfloor(mch));
            monster = Monster::CreateMonster(randmonster(false, game->get_level()), &mp, game->get_level());
            monster->give_pack();

        }
    }
}

//draw_room: Draw a box around a room and lay down the floor
void draw_room(struct Room *room)
{
    int y, x;

    //Here we draw normal rooms, one side at a time
    vert(room, room->m_ul_corner.x); //Draw left side
    vert(room, room->m_ul_corner.x + room->m_size.x - 1); //Draw right side
    horiz(room, room->m_ul_corner.y); //Draw top
    horiz(room, room->m_ul_corner.y + room->m_size.y - 1); //Draw bottom
    game->level().set_tile(room->m_ul_corner, ULWALL);
    game->level().set_tile({ room->m_ul_corner.x + room->m_size.x - 1,room->m_ul_corner.y }, URWALL);
    game->level().set_tile({ room->m_ul_corner.x,room->m_ul_corner.y + room->m_size.y - 1 }, LLWALL);
    game->level().set_tile({ room->m_ul_corner.x + room->m_size.x - 1,room->m_ul_corner.y + room->m_size.y - 1 }, LRWALL);
    //Put the floor down
    for (y = room->m_ul_corner.y + 1; y < room->m_ul_corner.y + room->m_size.y - 1; y++)
        for (x = room->m_ul_corner.x + 1; x < room->m_ul_corner.x + room->m_size.x - 1; x++)
            game->level().set_tile({ x, y }, FLOOR);
}

//vert: Draw a vertical line
void vert(struct Room *room, int startx)
{
    for (int y = room->m_ul_corner.y + 1; y <= room->m_size.y + room->m_ul_corner.y - 1; y++)
        game->level().set_tile({ startx,y }, VWALL);
}

//horiz: Draw a horizontal line
void horiz(struct Room *room, int starty)
{
    for (int x = room->m_ul_corner.x; x <= room->m_ul_corner.x + room->m_size.x - 1; x++)
        game->level().set_tile({ x,starty }, HWALL);
}

//rnd_pos: Pick a random spot in a room
void rnd_pos(struct Room *room, Coord *cp)
{
    cp->x = room->m_ul_corner.x + rnd(room->m_size.x - 2) + 1;
    cp->y = room->m_ul_corner.y + rnd(room->m_size.y - 2) + 1;
}

//enter_room: Code that is executed whenever you appear in a room
void enter_room(Coord cp)
{
    int y, x;
    Monster* monster;

    Room* room = game->level().get_room_from_position(cp);
    game->hero().set_room(room);

    if (game->invalid_position || (room->is_gone() && (room->is_maze()) == 0))
    {
        debug("in a gone room");
        return;
    }
    door_open(room);
    if (!(room->is_dark()) && !game->hero().is_blind() && !(room->is_maze())) {
        for (y = room->m_ul_corner.y; y < room->m_size.y + room->m_ul_corner.y; y++)
        {
            game->screen().move(y, room->m_ul_corner.x);
            for (x = room->m_ul_corner.x; x < room->m_size.x + room->m_ul_corner.x; x++)
            {
                Coord pos = { x, y };
                //Displaying monsters is all handled in the chase code now
                monster = game->level().monster_at(pos);
                if (monster == NULL || !game->hero().can_see_monster(monster))
                    game->screen().addch(game->level().get_tile(pos));
                else {
                    monster->reload_tile_beneath();
                    monster->render();
                }
            }
        }
    }
}

//leave_room: Code for when we exit a room
void leave_room(Coord cp)
{
    byte ch;

    Room* room = game->hero().room();
    game->hero().set_room(game->level().get_passage(cp));

    if(game->wizard().see_all())
        return;

    byte floor = (room->is_dark() && !game->hero().is_blind()) ? ' ' : FLOOR;
    if (room->is_maze())
        floor = PASSAGE;

    for (int y = room->m_ul_corner.y + 1; y < room->m_size.y + room->m_ul_corner.y - 1; y++) {
        for (int x = room->m_ul_corner.x + 1; x < room->m_size.x + room->m_ul_corner.x - 1; x++) {
            Coord pos = { x, y };
            switch (ch = game->screen().mvinch(pos))
            {
            case ' ': case PASSAGE: case TRAP: case STAIRS:
                break;

            default:
                //to check for monster, we have to strip out standout bit
                if (isupper(toascii(ch)) && game->hero().detects_others()) {
                    game->screen().standout();
                    game->screen().addch(ch);
                    game->screen().standend();
                    break;
                }
                else {
                    //mdk:bugfix: phantoms could leave a blank spot on the map because
                    //the code was detecting monsters by the character on the screen.
                    //I moved this code slightly so invisible monsters are considered.
                    Monster* m = game->level().monster_at(pos);
                    if (m) {
                        m->invalidate_tile_beneath();
                    }
                }

                game->screen().addch(floor);
            }
        }
    }
    door_open(room);
}

//get_room_from_position: Find what room some coordinates are in. NULL means they aren't in any room.
Room* Level::get_room_from_position(Coord pos)
{
    struct Room *room;

    for (room = rooms; room <= &rooms[MAXROOMS - 1]; room++)
        if (pos.x < room->m_ul_corner.x + room->m_size.x &&
            room->m_ul_corner.x <= pos.x &&
            pos.y < room->m_ul_corner.y + room->m_size.y
            && room->m_ul_corner.y <= pos.y)
            return room;

    if (game->level().is_passage(pos))
        return game->level().get_passage(pos);

    debug("in some bizarre place (%d, %d)", pos.y, pos.x);
    game->invalid_position = true;
    return NULL;
}

//rnd_room: Pick a room that is really there
Room* Level::rnd_room()
{
    int rm;
    do {
        rm = rnd(MAXROOMS);
    } while (!((rooms[rm].is_gone()) == 0 || (rooms[rm].is_maze())));

    return &rooms[rm];
}


void find_empty_location(Coord* c, bool consider_monsters)
{
    do
    {
        rnd_pos(game->level().rnd_room(), c);
    } while (!isfloor(game->level().get_tile(*c, consider_monsters)));
}
