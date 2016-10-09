/*
 * Functions for dealing with linked lists of goodies
 *
 * @(#)list.c	4.7 (Berkeley) 12/19/81
 *
 * Rogue: Exploring the Dungeons of Doom
 * Copyright (C) 1980, 1981, 1982 Michael Toy, Ken Arnold and Glenn Wichman
 * All rights reserved.
 *
 * See the file LICENSE.TXT for full copyright and licensing information.
 */

#include <stdlib.h>
#include <curses.h>
#include "rogue.h"

/*
 * detach:
 *	Takes an item out of whatever linked list it might be in
 */
_detach(list, item)
register THING **list, *item;
{
    if (*list == item)
	*list = next(item);
    if (prev(item) != NULL) item->l_prev->l_next = next(item);
    if (next(item) != NULL) item->l_next->l_prev = prev(item);
    item->l_next = NULL;
    item->l_prev = NULL;
}

/*
 * _attach:
 *	add an item to the head of a list
 */
_attach(list, item)
register THING **list, *item;
{
    if (*list != NULL)
    {
	item->l_next = *list;
	(*list)->l_prev = item;
	item->l_prev = NULL;
    }
    else
    {
	item->l_next = NULL;
	item->l_prev = NULL;
    }
    *list = item;
}

/*
 * _free_list:
 *	Throw the whole blamed thing away
 */
_free_list(ptr)
register THING **ptr;
{
    register THING *item;

    while (*ptr != NULL)
    {
	item = *ptr;
	*ptr = next(item);
	discard(item);
    }
}

/*
 * discard:
 *	Free up an item
 */
discard(item)
register THING *item;
{
    total--;
    free((char *) item);
}

/*
 * new_item
 *	Get a new item with a specified size
 */
THING *
new_item()
{
    register THING *item;

    if ((item = calloc(1, sizeof *item)) == NULL)
	msg("ran out of memory after %d items", total);
    else
	total++;
    item->l_next = item->l_prev = NULL;
    return item;
}
