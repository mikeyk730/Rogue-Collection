/*
    state.c - Portable Rogue Save State Code

    Copyright (C) 1999, 2000, 2005, 2007, 2008 Nicholas J. Kisseberth
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions
    are met:
    1. Redistributions of source code must retain the above copyright
       notice, this list of conditions and the following disclaimer.
    2. Redistributions in binary form must reproduce the above copyright
       notice, this list of conditions and the following disclaimer in the
       documentation and/or other materials provided with the distribution.
    3. Neither the name(s) of the author(s) nor the names of other contributors
       may be used to endorse or promote products derived from this software
       without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE AUTHOR(S) AND CONTRIBUTORS ``AS IS'' AND
    ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
    IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
    ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR(S) OR CONTRIBUTORS BE LIABLE
    FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
    DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
    OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
    HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
    LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
    OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
    SUCH DAMAGE.
*/

#include <stdlib.h>
#include <string.h>
#include <curses.h>
#include <errno.h>
#include "rogue.h"

/************************************************************************/
/* Save State Code                                                      */
/************************************************************************/

#define RSID_STATS        0xABCD0001
#define RSID_THING        0xABCD0002
#define RSID_THING_NULL   0xDEAD0002
#define RSID_OBJECT       0xABCD0003
#define RSID_MAGICITEMS   0xABCD0004
#define RSID_KNOWS        0xABCD0005
#define RSID_GUESSES      0xABCD0006
#define RSID_OBJECTLIST   0xABCD0007
#define RSID_BAGOBJECT    0xABCD0008
#define RSID_MONSTERLIST  0xABCD0009
#define RSID_MONSTERSTATS 0xABCD000A
#define RSID_MONSTERS     0xABCD000B
#define RSID_TRAP         0xABCD000C
#define RSID_WINDOW       0xABCD000D
#define RSID_DAEMONS      0xABCD000E
#define RSID_IWEAPS       0xABCD000F
#define RSID_IARMOR       0xABCD0010
#define RSID_SPELLS       0xABCD0011
#define RSID_ILIST        0xABCD0012
#define RSID_HLIST        0xABCD0013
#define RSID_DEATHTYPE    0xABCD0014
#define RSID_CTYPES       0XABCD0015
#define RSID_COORDLIST    0XABCD0016
#define RSID_ROOMS        0XABCD0017

#define READSTAT (format_error || read_error )
#define WRITESTAT (write_error)

static int read_error   = FALSE;
static int write_error  = FALSE;
static int format_error = FALSE;
static int endian = 0x01020304;
#define  big_endian ( *((char *)&endian) == 0x01 )

void
rs_write(FILE *savef, const void *ptr, size_t size)
{
    encwrite(ptr, size, savef);
}

void
rs_read(FILE *savef, void *ptr, size_t size)
{
    encread(ptr, size, savef);
}

void
rs_write_int(FILE *savef, int c)
{
    char bytes[4];
    char *buf = (char *) &c;

    if (big_endian)
    {
        bytes[3] = buf[0];
        bytes[2] = buf[1];
        bytes[1] = buf[2];
        bytes[0] = buf[3];
        buf = bytes;
    }
    
    rs_write(savef, buf, 4);
}

void
rs_read_int(FILE *savef, int *i)
{
    char bytes[4];
    int input = 0;
    char *buf = (char *)&input;
    
    rs_read(savef, &input, 4);

    if (encerror())
	return;

    if (big_endian)
    {
        bytes[3] = buf[0];
        bytes[2] = buf[1];
        bytes[1] = buf[2];
        bytes[0] = buf[3];
        buf = bytes;
    }
    
    *i = *((int *) buf);
}

void
rs_write_uint(FILE *savef, unsigned int c)
{
    char bytes[4];
    char *buf = (char *) &c;

    if (big_endian)
    {
        bytes[3] = buf[0];
        bytes[2] = buf[1];
        bytes[1] = buf[2];
        bytes[0] = buf[3];
        buf = bytes;
    }
    
    rs_write(savef, buf, 4);
}

void
rs_read_uint(FILE *savef, unsigned int *i)
{
    char bytes[4];
    int input = 0;
    char *buf = (char *)&input;
    
    rs_read(savef, &input, 4);

    if (encerror())
	return;

    if (big_endian)
    {
        bytes[3] = buf[0];
        bytes[2] = buf[1];
        bytes[1] = buf[2];
        bytes[0] = buf[3];
        buf = bytes;
    }
    
    *i = *((unsigned int *) buf);
}

void
rs_write_chars(FILE *savef, const char *c, int cnt)
{
    rs_write_int(savef, cnt);
    rs_write(savef, c, cnt);
}

void
rs_read_chars(FILE *savef, char *i, int cnt)
{
    int value = 0;  

    rs_read_int(savef, &value);
    
    if (!encerror() && (value != cnt))
	encseterr(EILSEQ);

    rs_read(savef, i, cnt);
}

void
rs_write_ints(FILE *savef, int *c, int cnt)
{
    int n = 0;

    rs_write_int(savef, cnt);

    for(n = 0; n < cnt; n++)
        rs_write_int(savef,c[n]);
}

void
rs_read_ints(FILE *savef, int *i, int cnt)
{
    int n, value;
    
    rs_read_int(savef,&value);

    if (!encerror() && (value != cnt))
	encseterr(EILSEQ);

    for(n = 0; n < cnt; n++)
        rs_read_int(savef, &i[n]);
}

void
rs_write_marker(FILE *savef, int id)
{
    rs_write_int(savef, id);
}

void
rs_read_marker(FILE *savef, int id)
{
    int nid;

    rs_read_int(savef, &nid);

    if (!encerror() && (id != nid))
	encseterr(EILSEQ);
}

/******************************************************************************/

void
rs_write_string(FILE *savef, const char *s)
{
    int len = 0;

    len = (s == NULL) ? 0 : (int) strlen(s) + 1;

    rs_write_int(savef, len);
    rs_write_chars(savef, s, len);
}

void
rs_read_string(FILE *savef, char *s, int max)
{
    int len = 0;

    rs_read_int(savef, &len);

    if (!encerror() && (len > max))
	encseterr(EILSEQ);

    rs_read_chars(savef, s, len);
}

void
rs_read_new_string(FILE *savef, char **s)
{
    int len=0;
    char *buf=0;

    rs_read_int(savef, &len);

    if (encerror())
	return;

    if (len == 0)
        buf = NULL;
    else
    { 
        buf = malloc(len);

        if (buf == NULL)            
            encseterr(ENOMEM);
    }

    rs_read_chars(savef, buf, len);

    *s = buf;
}

void
rs_write_string_index(FILE *savef, char *master[], int max, char *str)
{
    int i;

    for(i = 0; i < max; i++)
        if (str == master[i])
        {
            rs_write_int(savef, i);
            return;
        }

    rs_write_int(savef,-1);
}

void
rs_read_string_index(FILE *savef, char *master[], int maxindex, char **str)
{
    int i;

    rs_read_int(savef, &i);

    if (!encerror() && (i > maxindex))
        encseterr(EILSEQ);
    else if (i >= 0)
        *str = master[i];
    else
        *str = NULL;
}

void
rs_write_coord(FILE *savef, coord c)
{
    rs_write_int(savef, c.x);
    rs_write_int(savef, c.y);
}

void
rs_read_coord(FILE *savef, coord *c)
{
    coord in;

    rs_read_int(savef,&in.x);
    rs_read_int(savef,&in.y);

    if (!encerror()) 
    {
        c->x = in.x;
        c->y = in.y;
    }
}

void
rs_write_str_t(FILE *savef, str_t str)
{
    rs_write_int(savef, str.st_str);
    rs_write_int(savef, str.st_add);
}

void
rs_read_str_t(FILE *savef, str_t *str)
{
    str_t in;

    rs_read_int(savef,&in.st_str);
    rs_read_int(savef,&in.st_add);

    if (!encerror()) 
    {
        str->st_str = in.st_str;
	str->st_add = in.st_add;
    }
}

void
rs_write_window(FILE *savef, WINDOW *win)
{
    int row,col,height,width;

    width  = getmaxx(win);
    height = getmaxy(win);

    rs_write_marker(savef,RSID_WINDOW);
    rs_write_int(savef,height);
    rs_write_int(savef,width);

    for(row=0;row<height;row++)
        for(col=0;col<width;col++)
            rs_write_int(savef, mvwinch(win,row,col));
}

void
rs_read_window(FILE *savef, WINDOW *win)
{
    int row,col,maxlines,maxcols,value,width,height;
    
    width  = getmaxx(win);
    height = getmaxy(win);

    rs_read_marker(savef, RSID_WINDOW);

    rs_read_int(savef, &maxlines);
    rs_read_int(savef, &maxcols);

    if (encerror())
	return;

    for(row = 0; row < maxlines; row++)
        for(col = 0; col < maxcols; col++)
        {
            rs_read_int(savef, &value);

            if ((row < height) && (col < width))
                mvwaddch(win,row,col,value);
        }
}

/******************************************************************************/

void *
get_list_item(struct linked_list *l, int i)
{
    int cnt;

    for(cnt = 0; l != NULL; cnt++, l = l->l_next)
        if (cnt == i)
            return(l->l_data);
    
    return(NULL);
}

int
find_list_ptr(struct linked_list *l, void *ptr)
{
    int cnt;

    for(cnt = 0; l != NULL; cnt++, l = l->l_next)
        if (l->l_data == ptr)
            return(cnt);
               
    return(-1);
}

int
list_size(struct linked_list *l)
{
    int cnt;
    
    for(cnt = 0; l != NULL; cnt++, l = l->l_next)
        if (l->l_data == NULL)
            return(cnt);
    
    return(cnt);
}

/******************************************************************************/

void
rs_write_stats(FILE *savef, struct stats *s)
{
    rs_write_marker(savef, RSID_STATS);
    rs_write_str_t(savef, s->s_str);
    rs_write_int(savef, s->s_exp);
    rs_write_int(savef, s->s_lvl);
    rs_write_int(savef, s->s_arm);
    rs_write_int(savef, s->s_hpt);
    rs_write_chars(savef, s->s_dmg, sizeof(s->s_dmg));
}


void
rs_read_stats(FILE *savef, struct stats *s)
{
    rs_read_marker(savef, RSID_STATS);
    rs_read_str_t(savef,&s->s_str);
    rs_read_int(savef,&s->s_exp);
    rs_read_int(savef,&s->s_lvl);
    rs_read_int(savef,&s->s_arm);
    rs_read_int(savef,&s->s_hpt);
    rs_read_chars(savef,s->s_dmg,sizeof(s->s_dmg));
}

        
void
rs_write_scrolls(FILE *savef)
{
    int i;

    for(i = 0; i < MAXSCROLLS; i++)
    {
        rs_write_string(savef,s_names[i]);
        rs_write_int(savef,s_know[i]);
        rs_write_string(savef,s_guess[i]);
    }
}

void
rs_read_scrolls(FILE *savef)
{
    int i;

    for(i = 0; i < MAXSCROLLS; i++)
    {
        rs_read_new_string(savef,&s_names[i]);
        rs_read_int(savef,&s_know[i]);
        rs_read_new_string(savef,&s_guess[i]);
    }
}

void
rs_write_potions(FILE *savef)
{
    int i;

    for(i = 0; i < MAXPOTIONS; i++)
    {
        rs_write_string_index(savef, rainbow, cNCOLORS, p_colors[i]);
        rs_write_int(savef,p_know[i]);
        rs_write_string(savef,p_guess[i]);
    }
}

void
rs_read_potions(FILE *savef)
{
    int i;

    for(i = 0; i < MAXPOTIONS; i++)
    {
        rs_read_string_index(savef, rainbow, cNCOLORS, &p_colors[i]);
        rs_read_int(savef,&p_know[i]);
        rs_read_new_string(savef,&p_guess[i]);
    }
}

void
rs_write_rings(FILE *savef)
{
    int i;

    for(i = 0; i < MAXRINGS; i++)
    {
        rs_write_string_index(savef, stones, cNSTONES, r_stones[i]);
        rs_write_int(savef,r_know[i]);
        rs_write_string(savef,r_guess[i]);
    }
}

void
rs_read_rings(FILE *savef)
{
    int i;

    for(i = 0; i < MAXRINGS; i++)
    {
        rs_read_string_index(savef, stones, cNSTONES, &r_stones[i]);
        rs_read_int(savef,&r_know[i]);
        rs_read_new_string(savef,&r_guess[i]);
    }
}

void
rs_write_sticks(FILE *savef)
{
    int i;

    for (i = 0; i < MAXSTICKS; i++)
    {
        if (strcmp(ws_type[i],"staff") == 0)
        {
            rs_write_int(savef,0);
            rs_write_string_index(savef, wood, cNWOOD, ws_made[i]);
        }
        else
        {
            rs_write_int(savef,1);
            rs_write_string_index(savef, metal, cNMETAL, ws_made[i]);
        }
        rs_write_int(savef, ws_know[i]);
        rs_write_string(savef, ws_guess[i]);
    }
}

void
rs_read_sticks(FILE *savef)
{
    int i = 0, list = 0;

    for(i = 0; i < MAXSTICKS; i++)
    { 
        rs_read_int(savef,&list);

	if (list == 0)
        {
            rs_read_string_index(savef, wood, cNWOOD, &ws_made[i]);
            ws_type[i] = "staff";
        }
        else 
        {
            rs_read_string_index(savef, metal, cNMETAL, &ws_made[i]);
            ws_type[i] = "wand";
        }
        rs_read_int(savef, &ws_know[i]);
        rs_read_new_string(savef, &ws_guess[i]);
    }
}

void
rs_write_daemons(FILE *savef, struct delayed_action *dlist, int cnt)
{
    int i = 0;
    int func = 0;
        
    rs_write_marker(savef, RSID_DAEMONS);
    rs_write_int(savef, cnt);
        
    for(i = 0; i < cnt; i++)
    {
        if (dlist[i].d_func == rollwand)
            func = 1;
        else if (dlist[i].d_func == doctor)
            func = 2;
        else if (dlist[i].d_func == stomach)
            func = 3;
        else if (dlist[i].d_func == runners)
            func = 4;
        else if (dlist[i].d_func == swander)
            func = 5;
        else if (dlist[i].d_func == nohaste)
            func = 6;
        else if (dlist[i].d_func == unconfuse)
            func = 7;
        else if (dlist[i].d_func == unsee)
            func = 8;
        else if (dlist[i].d_func == sight)
            func = 9;
        else if (dlist[i].d_func == NULL)
            func = 0;
        else
            func = -1;

        rs_write_int(savef, dlist[i].d_type);
        rs_write_int(savef, func);
        rs_write_int(savef, dlist[i].d_arg);
        rs_write_int(savef, dlist[i].d_time);
    }
}       

void
rs_read_daemons(FILE *savef, struct delayed_action *dlist, int cnt)
{
    int i = 0;
    int func = 0;
    int value = 0;

    rs_read_marker(savef, RSID_DAEMONS);
    rs_read_int(savef, &value);

    if (!encerror() && (value > cnt))
    {
	encseterr(EILSEQ);
	return;
    }

    for(i=0; i < cnt; i++)
    {
	func = 0;
        rs_read_int(savef, &dlist[i].d_type);
        rs_read_int(savef, &func);
        rs_read_int(savef, &dlist[i].d_arg);
        rs_read_int(savef, &dlist[i].d_time);

	if (encerror())
	    return;
                    
        switch(func)
        {
	    case 1: dlist[i].d_func = rollwand;
		    break;
	    case 2: dlist[i].d_func = doctor;
                    break;
            case 3: dlist[i].d_func = stomach;
                    break;
            case 4: dlist[i].d_func = runners;
                    break;
            case 5: dlist[i].d_func = swander;
                    break;
            case 6: dlist[i].d_func = nohaste;
                    break;
            case 7: dlist[i].d_func = unconfuse;
                    break;
            case 8: dlist[i].d_func = unsee;
                    break;
            case 9: dlist[i].d_func = sight;
                    break;
            default:dlist[i].d_func = NULL;
                    break;
	}   
    }

    if (dlist[i].d_func == NULL)
    {
        dlist[i].d_type = 0;
        dlist[i].d_arg = 0;
        dlist[i].d_time = 0;
    } 
}       

void
rs_write_room(FILE *savef, struct room *r)
{
    rs_write_coord(savef, r->r_pos);
    rs_write_coord(savef, r->r_max);
    rs_write_coord(savef, r->r_gold);
    rs_write_int(savef, r->r_goldval);
    rs_write_int(savef, r->r_flags);
    rs_write_int(savef, r->r_nexits);
    rs_write_coord(savef, r->r_exit[0]);
    rs_write_coord(savef, r->r_exit[1]);
    rs_write_coord(savef, r->r_exit[2]);
    rs_write_coord(savef, r->r_exit[3]);
}

void
rs_read_room(FILE *savef, struct room *r)
{
    rs_read_coord(savef,&r->r_pos);
    rs_read_coord(savef,&r->r_max);
    rs_read_coord(savef,&r->r_gold);
    rs_read_int(savef,&r->r_goldval);
    rs_read_int(savef,&r->r_flags);
    rs_read_int(savef,&r->r_nexits);
    rs_read_coord(savef,&r->r_exit[0]);
    rs_read_coord(savef,&r->r_exit[1]);
    rs_read_coord(savef,&r->r_exit[2]);
    rs_read_coord(savef,&r->r_exit[3]);
}

void
rs_write_rooms(FILE *savef, struct room r[], int cnt)
{
    int n = 0;

    rs_write_int(savef, cnt);
    
    for(n = 0; n < cnt; n++)
        rs_write_room(savef, &r[n]);
}

void
rs_read_rooms(FILE *savef, struct room *r, int cnt)
{
    int value = 0, n = 0;

    rs_read_int(savef,&value);

    if (!encerror() && (value > cnt))
        encseterr(EILSEQ);
    else
        for(n = 0; n < value; n++)
            rs_read_room(savef,&r[n]);
}

void
rs_write_room_reference(FILE *savef, struct room *rp)
{
    int i, room = -1;
    
    for (i = 0; i < MAXROOMS; i++)
        if (&rooms[i] == rp)
            room = i;

    rs_write_int(savef, room);
}

void
rs_read_room_reference(FILE *savef, struct room **rp)
{
    int i;
    
    rs_read_int(savef, &i);

    if (!encerror())
        *rp = &rooms[i];
}

void
rs_write_object(FILE *savef, struct object *o)
{
    rs_write_marker(savef, RSID_OBJECT);
    rs_write_int(savef, o->o_type);
    rs_write_coord(savef, o->o_pos);
    rs_write_int(savef, o->o_launch);
    rs_write_chars(savef, o->o_damage, sizeof(o->o_damage));
    rs_write_chars(savef, o->o_hurldmg, sizeof(o->o_hurldmg));
    rs_write_int(savef, o->o_count);
    rs_write_int(savef, o->o_which);
    rs_write_int(savef, o->o_hplus);
    rs_write_int(savef, o->o_dplus);
    rs_write_int(savef, o->o_ac);
    rs_write_int(savef, o->o_flags);
    rs_write_int(savef, o->o_group);
}

void
rs_read_object(FILE *savef, struct object *o)
{
    rs_read_marker(savef, RSID_OBJECT);
    rs_read_int(savef, &o->o_type);
    rs_read_coord(savef, &o->o_pos);
    rs_read_int(savef, &o->o_launch);
    rs_read_chars(savef, o->o_damage, sizeof(o->o_damage));
    rs_read_chars(savef, o->o_hurldmg, sizeof(o->o_hurldmg));
    rs_read_int(savef, &o->o_count);
    rs_read_int(savef, &o->o_which);
    rs_read_int(savef, &o->o_hplus);
    rs_read_int(savef, &o->o_dplus);
    rs_read_int(savef,&o->o_ac);
    rs_read_int(savef,&o->o_flags);
    rs_read_int(savef,&o->o_group);
}

void
rs_write_object_list(FILE *savef, struct linked_list *l)
{
    rs_write_marker(savef, RSID_OBJECTLIST);
    rs_write_int(savef, list_size(l));

    for( ;l != NULL; l = l->l_next) 
        rs_write_object(savef, (struct object *) l->l_data);  
}

void
rs_read_object_list(FILE *savef, struct linked_list **list)
{
    int i, cnt;
    struct linked_list *l = NULL, *previous = NULL, *head = NULL;

    rs_read_marker(savef, RSID_OBJECTLIST);
    rs_read_int(savef, &cnt);

    if (encerror())
	return;

    for (i = 0; i < cnt; i++) 
    {
	l = new_item(sizeof(struct object));

        memset(l->l_data,0,sizeof(struct object));

        l->l_prev = previous;

        if (previous != NULL)
	    previous->l_next = l;

        rs_read_object(savef,(struct object *) l->l_data);

        if (previous == NULL)
	    head = l;

        previous = l;
    }
            
    if (l != NULL)
	l->l_next = NULL;
    
    *list = head;
}

void
rs_write_object_reference(FILE *savef, struct linked_list *list, struct object *item)
{
    int i;
    
    i = find_list_ptr(list, item);

    rs_write_int(savef, i);
}

void
rs_read_object_reference(FILE *savef, struct linked_list *list, struct object **item)
{
    int i;
    
    rs_read_int(savef, &i);

    if (!encerror())
	*item = get_list_item(list,i);
    else
	*item = NULL;
}

int
find_room_coord(const struct room *rmlist, const coord *c, int n)
{
    int i = 0;
    
    for(i = 0; i < n; i++)
        if(&rmlist[i].r_gold == c)
            return(i);
    
    return(-1);
}

int
find_thing_coord(struct linked_list *monlist, coord *c)
{
    struct linked_list *mitem;
    struct thing *tp;
    int i = 0;

    for(mitem = monlist; mitem != NULL; mitem = mitem->l_next)
    {
        tp = THINGPTR(mitem);

        if (c == &tp->t_pos)
            return(i);

        i++;
    }

    return(-1);
}

int
find_object_coord(struct linked_list *objlist, coord *c)
{
    struct linked_list *oitem;
    struct object *obj;
    int i = 0;

    for(oitem = objlist; oitem != NULL; oitem = oitem->l_next)
    {
        obj = OBJPTR(oitem);

        if (c == &obj->o_pos)
            return(i);

        i++;
    }

    return(-1);
}

void
rs_write_thing(FILE *savef, struct thing *t)
{
    int i = -1;

    rs_write_marker(savef, RSID_THING);

    if (t == NULL)
    {
        rs_write_int(savef, 0);
        return;
    }

    rs_write_int(savef, 1);
    rs_write_coord(savef, t->t_pos);
    rs_write_int(savef, t->t_turn);
    rs_write_int(savef, t->t_type);
    rs_write_int(savef, t->t_disguise);
    rs_write_int(savef, t->t_oldch);

    /* 
        t_dest can be:
        0,0: NULL
        0,1: location of hero
        1,i: location of a thing (monster)
        2,i: location of an object
        3,i: location of gold in a room

        We need to remember what we are chasing rather than 
        the current location of what we are chasing.
    */

    if (t->t_dest == &hero)
    {
        rs_write_int(savef,0);
        rs_write_int(savef,1);
    }
    else if (t->t_dest != NULL)
    {
        i = find_thing_coord(mlist, t->t_dest);
            
        if (i >=0 )
        {
            rs_write_int(savef,1);
            rs_write_int(savef,i);
        }
        else
        {
            i = find_object_coord(lvl_obj, t->t_dest);
            
            if (i >= 0)
            {
                rs_write_int(savef,2);
                rs_write_int(savef,i);
            }
            else
            {
                i = find_room_coord(rooms, t->t_dest, MAXROOMS);
        
                if (i >= 0) 
                {
                    rs_write_int(savef,3);
                    rs_write_int(savef,i);
                }
                else 
                {
                    rs_write_int(savef, 0);
                    rs_write_int(savef,1); /* chase the hero anyway */
                }
            }
        }
    }
    else
    {
        rs_write_int(savef,0);
        rs_write_int(savef,0);
    }
    
    rs_write_int(savef, t->t_flags);
    rs_write_stats(savef, &t->t_stats);
    rs_write_object_list(savef, t->t_pack);
}

void
rs_read_thing(FILE *savef, struct thing *t)
{
    int listid = 0, index = -1;
    struct linked_list *item;
        
    rs_read_marker(savef, RSID_THING);
    rs_read_int(savef, &index);

    if (encerror())
	return;

    if (index == 0)
        return;

    rs_read_coord(savef,&t->t_pos);
    rs_read_int(savef,&t->t_turn);
    rs_read_int(savef,&t->t_type);
    rs_read_int(savef,&t->t_disguise);
    rs_read_int(savef,&t->t_oldch);

    /* 
        t_dest can be (listid,index):
        0,0: NULL
        0,1: location of hero
        1,i: location of a thing (monster)
        2,i: location of an object
        3,i: location of gold in a room

        We need to remember what we are chasing rather than 
        the current location of what we are chasing.
    */
            
    rs_read_int(savef, &listid);
    rs_read_int(savef, &index);
    t->t_reserved = -1;

    if (encerror())
	return;

    if (listid == 0) /* hero or NULL */
    {
	if (index == 1)
	    t->t_dest = &hero;
        else
	    t->t_dest = NULL;
    }
    else if (listid == 1) /* monster/thing */
    {
	t->t_dest     = NULL;
        t->t_reserved = index;
    }
    else if (listid == 2) /* object */
    {
	struct object *obj;

        item = get_list_item(lvl_obj,index);

        if (item != NULL)
	{
	    obj = OBJPTR(item);
            t->t_dest = &obj->o_pos;	
	}
    }
    else if (listid == 3) /* gold */
    {
	t->t_dest = &rooms[index].r_gold;
    }
    else
	t->t_dest = NULL;
            
    rs_read_int(savef,&t->t_flags);
    rs_read_stats(savef,&t->t_stats);
    rs_read_object_list(savef,&t->t_pack);
}

void
rs_fix_thing(struct thing *t)
{
    struct linked_list *item;
    struct thing *tp;

    if (t->t_reserved < 0)
        return;

    item = get_list_item(mlist,t->t_reserved);

    if (item != NULL)
    {
        tp = THINGPTR(item);
        t->t_dest = &tp->t_pos;
    }
}

void
rs_write_thing_list(FILE *savef, struct linked_list *l)
{
    int cnt = 0;
    
    rs_write_marker(savef, RSID_MONSTERLIST);

    cnt = list_size(l);

    rs_write_int(savef, cnt);

    if (cnt < 1)
        return;

    while (l != NULL) {
        rs_write_thing(savef, (struct thing *)l->l_data);
        l = l->l_next;
    }
}

void
rs_read_thing_list(FILE *savef, struct linked_list **list)
{
    int i, cnt;
    struct linked_list *l = NULL, *previous = NULL, *head = NULL;

    rs_read_marker(savef, RSID_MONSTERLIST);
    rs_read_int(savef, &cnt);

    if (encerror())
	return;

    for (i = 0; i < cnt; i++) 
    {
	l = new_item(sizeof(struct thing));

        l->l_prev = previous;
        
	if (previous != NULL)
	    previous->l_next = l;

	rs_read_thing(savef,(struct thing *)l->l_data);

	if (previous == NULL)
	    head = l;

	previous = l;
    }
      
    if (l != NULL)
	l->l_next = NULL;

    *list = head;
}

void
rs_fix_thing_list(struct linked_list *list)
{
    struct linked_list *item;

    for(item = list; item != NULL; item = item->l_next)
        rs_fix_thing(THINGPTR(item));
}

void
rs_fix_magic_items(struct magic_item *mi, int cnt)
{
    int i;

    for (i = 0; i < cnt; i++)
	if (i > 0)
	    mi[i].mi_prob += mi[i-1].mi_prob;
}

void
rs_fix_monsters(struct monster mons[26])
{
    sprintf(mons['F'-'A'].m_stats.s_dmg,"%dd1",fung_hit);
}

void
rs_write_trap(FILE *savef, struct trap *trap)
{
    rs_write_coord(savef, trap->tr_pos);
    rs_write_int(savef, trap->tr_type);
    rs_write_int(savef, trap->tr_flags);
}

void
rs_read_trap(FILE *savef, struct trap *trap)
{
    rs_read_coord(savef,&trap->tr_pos);
    rs_read_int(savef,&trap->tr_type);
    rs_read_int(savef,&trap->tr_flags);
}

void
rs_write_traps(FILE *savef, struct trap t[], int cnt)
{
    int n = 0;

    rs_write_marker(savef, RSID_MONSTERS);
    rs_write_int(savef, cnt);
    
    for(n = 0; n < cnt; n++)
        rs_write_trap(savef, &t[n]);
}

void
rs_read_traps(FILE *savef, struct trap *t, int cnt)
{
    int value = 0, n = 0;

    rs_read_marker(savef, RSID_MONSTERS);

    rs_read_int(savef,&value);

    if (!encerror() && (value > cnt))
        encseterr(EILSEQ);

    for(n = 0; n < value; n++)
        rs_read_trap(savef,&t[n]);
}

int
rs_save_file(FILE *savef)
{
    encclearerr();

    rs_write_thing(savef, &player);                     
    rs_write_object_list(savef, lvl_obj);               
    rs_write_thing_list(savef, mlist);                
    rs_write_traps(savef, traps, MAXTRAPS);             
    rs_write_rooms(savef, rooms, MAXROOMS);             
    rs_write_room_reference(savef, oldrp);              
    rs_write_stats(savef,&max_stats);                   
    rs_write_object_reference(savef, player.t_pack, cur_weapon); 
    rs_write_object_reference(savef, player.t_pack, cur_armor);
    rs_write_object_reference(savef, player.t_pack, cur_ring[0]);
    rs_write_object_reference(savef, player.t_pack, cur_ring[1]);
    rs_write_int(savef, level);                         
    rs_write_int(savef, purse);                         
    rs_write_int(savef, mpos);                          
    rs_write_int(savef, ntraps);                        
    rs_write_int(savef, no_move);                       
    rs_write_int(savef, no_command);                    
    rs_write_int(savef, inpack);                        
    rs_write_int(savef, max_hp);                        
    rs_write_int(savef, total);                         
    rs_write_int(savef, lastscore);                     
    rs_write_int(savef, no_food);                       
    rs_write_int(savef, seed);                          
    rs_write_int(savef, count);                         
    rs_write_int(savef, dnum);                          
    rs_write_int(savef, fung_hit);                      
    rs_write_int(savef, quiet);                         
    rs_write_int(savef, max_level);                     
    rs_write_int(savef, food_left);                     
    rs_write_int(savef, group);                         
    rs_write_int(savef, hungry_state);                  
    rs_write_int(savef, take);
    rs_write_int(savef, runch);
    rs_write_scrolls(savef);
    rs_write_potions(savef);
    rs_write_rings(savef);
    rs_write_sticks(savef);
    rs_write_chars(savef,whoami,80);
    rs_write_chars(savef,fruit,80);
    rs_write_window(savef, cw);
    rs_write_window(savef, mw);
    rs_write_window(savef, stdscr);
    rs_write_int(savef, running);                   
    rs_write_int(savef, playing);                   
    rs_write_int(savef, wizard);                    
    rs_write_int(savef, after);
    rs_write_int(savef, notify);                    
    rs_write_int(savef, fight_flush);               
    rs_write_int(savef, terse);                     
    rs_write_int(savef, door_stop);                 
    rs_write_int(savef, jump);                      
    rs_write_int(savef, slow_invent);               
    rs_write_int(savef, firstmove);                 
    rs_write_int(savef, waswizard);                 
    rs_write_int(savef, askme);                     
    rs_write_int(savef, amulet);                    
    rs_write_int(savef, in_shell);                  
    rs_write_coord(savef, oldpos);                      
    rs_write_coord(savef, delta);                       
    rs_write_coord(savef, ch_ret);                      /* chase.c      */
    rs_write_daemons(savef, &d_list[0], 20);            /* daemon.c     */
    rs_write_int(savef,between);                        /* daemons.c    */
    rs_write_int(savef,num_checks);                     /* main.c       */
    rs_write_chars(savef,lvl_mons,sizeof(lvl_mons));    /* monsters.c   */
    rs_write_chars(savef,wand_mons,sizeof(wand_mons));	/* monsters.c   */

    return( encclearerr() );
}

int
rs_restore_file(FILE *savef)
{
    encclearerr();

    rs_read_thing(savef, &player);                        
    rs_read_object_list(savef, &lvl_obj);                 
    rs_read_thing_list(savef, &mlist);                  
    rs_fix_thing(&player);
    rs_fix_thing_list(mlist);
    rs_read_traps(savef, traps, MAXTRAPS);
    rs_read_rooms(savef, rooms, MAXROOMS);
    rs_read_room_reference(savef, &oldrp);
    rs_read_stats(savef,&max_stats);                      
    rs_read_object_reference(savef, player.t_pack, &cur_weapon);
    rs_read_object_reference(savef, player.t_pack, &cur_armor);
    rs_read_object_reference(savef, player.t_pack, &cur_ring[0]);
    rs_read_object_reference(savef, player.t_pack, &cur_ring[1]);
    rs_fix_magic_items(things,NUMTHINGS);         
    rs_fix_magic_items(s_magic,MAXSCROLLS);       
    rs_fix_magic_items(p_magic,MAXPOTIONS);       
    rs_fix_magic_items(r_magic,MAXRINGS);         
    rs_fix_magic_items(ws_magic,MAXSTICKS);       
    rs_read_int(savef, &level);                           
    rs_read_int(savef, &purse);                           
    rs_read_int(savef, &mpos);                            
    rs_read_int(savef, &ntraps);                          
    rs_read_int(savef, &no_move);                         
    rs_read_int(savef, &no_command);                      
    rs_read_int(savef, &inpack);                          
    rs_read_int(savef, &max_hp);                          
    rs_read_int(savef, &total);                           
    rs_read_int(savef, &lastscore);                       
    rs_read_int(savef, &no_food);                         
    rs_read_int(savef, &seed);                            
    rs_read_int(savef, &count);                           
    rs_read_int(savef, &dnum);                            
    rs_read_int(savef, &fung_hit);                        
    rs_read_int(savef, &quiet);                           
    rs_read_int(savef, &max_level);                       
    rs_read_int(savef, &food_left);                       
    rs_read_int(savef, &group);                           
    rs_read_int(savef, &hungry_state);                    
    rs_read_int(savef, &take);
    rs_read_int(savef, &runch);
    rs_read_scrolls(savef);
    rs_read_potions(savef);
    rs_read_rings(savef);
    rs_read_sticks(savef);
    rs_read_chars(savef,whoami,80);
    rs_read_chars(savef,fruit,80);
    rs_read_window(savef, cw);
    rs_read_window(savef, mw);
    rs_read_window(savef, stdscr);
    rs_read_int(savef, &running);                     
    rs_read_int(savef, &playing);                     
    rs_read_int(savef, &wizard);                      
    rs_read_int(savef, &after);
    rs_read_int(savef, &notify);                      
    rs_read_int(savef, &fight_flush);                 
    rs_read_int(savef, &terse);                       
    rs_read_int(savef, &door_stop);                   
    rs_read_int(savef, &jump);                        
    rs_read_int(savef, &slow_invent);                 
    rs_read_int(savef, &firstmove);                   
    rs_read_int(savef, &waswizard);                   
    rs_read_int(savef, &askme);                       
    rs_read_int(savef, &amulet);                      
    rs_read_int(savef, &in_shell);                    
    rs_read_coord(savef,&oldpos);                         
    rs_read_coord(savef,&delta);                          
    rs_read_coord(savef, &ch_ret);                        /* chase.c      */
    rs_read_daemons(savef, d_list, 20);                   /* daemon.c     */
    rs_read_int(savef,&between);                          /* daemons.c    */
    rs_read_int(savef,&num_checks);                       /* main.c       */
    rs_read_chars(savef, lvl_mons, sizeof(lvl_mons));     /* monsters.c   */
    rs_read_chars(savef, wand_mons, sizeof(wand_mons));	/* monsters.c   */
    rs_fix_monsters(monsters);    

    return( encclearerr() );
}
