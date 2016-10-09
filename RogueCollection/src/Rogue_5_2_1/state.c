/*
    state.c - Portable Rogue Save State Code

    Copyright (C) 1999, 2000, 2005 Nicholas J. Kisseberth

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



#include <curses.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include "rogue.h"

#define READSTAT ((format_error == 0) && (read_error == 0))
#define WRITESTAT (write_error == 0)

int read_error   = FALSE;
int write_error  = FALSE;
int format_error = FALSE;
int end_of_file  = FALSE;
int big_endian   = 0;

void *
get_list_item(THING *l, int i)
{
    int count = 0;

    while(l != NULL)
    {   
        if (count == i)
            return(l);
                        
        l = l->l_next;
        
        count++;
    }
    
    return(NULL);
}

int
find_list_ptr(THING *l, void *ptr)
{
    int count = 0;

    while(l != NULL)
    {
        if (l == ptr)
            return(count);
            
        l = l->l_next;
        count++;
    }
    
    return(-1);
}

int
list_size(THING *l)
{
    int count = 0;
    
    while(l != NULL)
    {
        if (l == NULL)
            return(count);
            
        count++;
        
        l = l->l_next;
    }
    
    return(count);
}

int
rs_write(FILE *savef, void *ptr, int size)
{
    if (!write_error)
        encwrite(ptr,size,savef);

    if (0)
        write_error = TRUE;
        
    assert(write_error == 0);

    return(WRITESTAT);
}

int
rs_write_char(FILE *savef, char c)
{
    rs_write(savef, &c, 1);
    
    return(WRITESTAT);
}

int
rs_write_boolean(FILE *savef, bool c)
{
    unsigned char buf = (c == 0) ? 0 : 1;
    
    rs_write(savef, &buf, 1);

    return(WRITESTAT);
}

int
rs_write_booleans(FILE *savef, bool *c, int count)
{
    int n = 0;

    rs_write_int(savef,count);
    
    for(n = 0; n < count; n++)
        rs_write_boolean(savef,c[n]);

    return(WRITESTAT);
}

int
rs_write_shint(FILE *savef, shint c)
{
    unsigned char buf = c;

    rs_write(savef, &buf, 1);

    return(WRITESTAT);
}

int
rs_write_short(FILE *savef, short c)
{
    unsigned char bytes[2];
    unsigned char *buf = (unsigned char *) &c;

    if (big_endian)
    {
        bytes[1] = buf[0];
        bytes[0] = buf[1];
        buf = bytes;
    }

    rs_write(savef, buf, 2);

    return(WRITESTAT);
}

int
rs_write_shorts(FILE *savef, short *c, int count)
{
    int n = 0;

    rs_write_int(savef,count);
    
    for(n = 0; n < count; n++)
        rs_write_short(savef,c[n]);

    return(WRITESTAT);
}

int
rs_write_ushort(FILE *savef, unsigned short c)
{
    unsigned char bytes[2];
    unsigned char *buf = (unsigned char *) &c;

    if (big_endian)
    {
        bytes[1] = buf[0];
        bytes[0] = buf[1];
        buf = bytes;
    }

    rs_write(savef, buf, 2);

    return(WRITESTAT);
}

int
rs_write_int(FILE *savef, int c)
{
    unsigned char bytes[4];
    unsigned char *buf = (unsigned char *) &c;

    if (big_endian)
    {
        bytes[3] = buf[0];
        bytes[2] = buf[1];
        bytes[1] = buf[2];
        bytes[0] = buf[3];
        buf = bytes;
    }
    
    rs_write(savef, buf, 4);

    return(WRITESTAT);
}

int
rs_write_ints(FILE *savef, int *c, int count)
{
    int n = 0;

    rs_write_int(savef,count);
    
    for(n = 0; n < count; n++)
        rs_write_int(savef,c[n]);

    return(WRITESTAT);
}

int
rs_write_uint(FILE *savef, unsigned int c)
{
    unsigned char bytes[4];
    unsigned char *buf = (unsigned char *) &c;

    if (big_endian)
    {
        bytes[3] = buf[0];
        bytes[2] = buf[1];
        bytes[1] = buf[2];
        bytes[0] = buf[3];
        buf = bytes;
    }
    
    rs_write(savef, buf, 4);

    return(WRITESTAT);
}

int
rs_write_long(FILE *savef, long c)
{
    int c2;
    unsigned char bytes[4];
    unsigned char *buf = (unsigned char *)&c;

    if (sizeof(long) == 8)
    {
        c2 = c;
        buf = (unsigned char *) &c2;
    }

    if (big_endian)
    {
        bytes[3] = buf[0];
        bytes[2] = buf[1];
        bytes[1] = buf[2];
        bytes[0] = buf[3];
        buf = bytes;
    }
    
    rs_write(savef, buf, 4);

    return(WRITESTAT);
}

int
rs_write_longs(FILE *savef, long *c, int count)
{
    int n = 0;

    rs_write_int(savef,count);
    
    for(n = 0; n < count; n++)
        rs_write_long(savef,c[n]);

    return(WRITESTAT);
}

int
rs_write_ulong(FILE *savef, unsigned long c)
{
    unsigned int c2;
    unsigned char bytes[4];
    unsigned char *buf = (unsigned char *)&c;

    if ( (sizeof(long) == 8) && (sizeof(int) == 4) )
    {
        c2 = c;
        buf = (unsigned char *) &c2;
    }

    if (big_endian)
    {
        bytes[3] = buf[0];
        bytes[2] = buf[1];
        bytes[1] = buf[2];
        bytes[0] = buf[3];
        buf = bytes;
    }
    
    rs_write(savef, buf, 4);

    return(WRITESTAT);
}

int
rs_write_ulongs(FILE *savef, unsigned long *c, int count)
{
    int n = 0;

    rs_write_int(savef,count);
    
    for(n = 0; n < count; n++)
        rs_write_ulong(savef,c[n]);

    return(WRITESTAT);
}

int
rs_write_string(FILE *savef, char *s)
{
    int len = 0;

    len = (s == NULL) ? 0 : strlen(s) + 1;

    rs_write_int(savef, len);
    rs_write(savef, s, len);
            
    return(WRITESTAT);
}

int
rs_write_string_index(FILE *savef, const char *master[], int max, 
                      const char *str)
{
    int i;

    for(i = 0; i < max; i++)
    {
        if (str == master[i])
        {
            rs_write_int(savef,i);
            return(WRITESTAT);
        }
    }

    rs_write_int(savef,-1);

    return(WRITESTAT);
}

int
rs_write_strings(FILE *savef, char *s[], int count)
{
    int len = 0;
    int n = 0;

    rs_write_int(savef,count);

    for(n = 0; n < count; n++)
    {
        len = (s[n] == NULL) ? 0L : strlen(s[n]) + 1;
        rs_write_int(savef, len);
        rs_write(savef, s[n], len);
    }
    
    return(WRITESTAT);
}

int
rs_read(int inf, void *ptr, int size)
{
    int actual;

    end_of_file = FALSE;

    if (!read_error && !format_error)
    {
        actual = encread(ptr, size, inf);

        if ((actual == 0) && (size != 0))
           end_of_file = TRUE;
    }
       
    if (read_error)
    {
        printf("read error has occurred. restore short-circuited.\n");
        abort();
    }

    if (format_error)
    {
        printf("game format invalid. restore short-circuited.\n");
        abort();
    }

    return(READSTAT);
}

int
rs_read_char(int inf, char *c)
{
    rs_read(inf, c, 1);
    
    return(READSTAT);
}
int
rs_read_uchar(int inf, unsigned char *c)
{
    rs_read(inf, c, 1);
    
    return(READSTAT);
}

int
rs_read_boolean(int inf, bool *i)
{
    unsigned char buf;
    
    rs_read(inf, &buf, 1);
    
    *i = (bool) buf;
    
    return(READSTAT);
}

int
rs_read_booleans(int inf, bool *i, int count)
{
    int n = 0, value = 0;
    
    if (rs_read_int(inf,&value) != 0)
    {
        if (value != count)
        {
            printf("Invalid booleans block. %d != requested %d\n",value,count); 
            format_error = TRUE;
        }
        else
        {
            for(n = 0; n < value; n++)
                rs_read_boolean(inf, &i[n]);
        }
    }
    
    return(READSTAT);
}

int
rs_read_shint(int inf, shint *i)
{
    unsigned char buf;
    
    rs_read(inf, &buf, 1);
    
    *i = (shint) buf;
    
    return(READSTAT);
}

int
rs_read_short(int inf, short *i)
{
    unsigned char bytes[2];
    short  input;
    unsigned char *buf = (unsigned char *)&input;
    
    rs_read(inf, &input, 2);

    if (big_endian)
    {
        bytes[1] = buf[0];
        bytes[0] = buf[1];
        buf = bytes;
    }
    
    *i = *((short *) buf);

    return(READSTAT);
} 

int
rs_read_shorts(int inf, short *i, int count)
{
    int n = 0, value = 0;

    if (rs_read_int(inf,&value) != 0)
    {
        if (value != count)
            format_error = TRUE;
        else
        {
            for(n = 0; n < value; n++)
                rs_read_short(inf, &i[n]);
        }
    }
    
    return(READSTAT);
}

int
rs_read_ushort(int inf, unsigned short *i)
{
    unsigned char bytes[2];
    unsigned short  input;
    unsigned char *buf = (unsigned char *)&input;
    
    rs_read(inf, &input, 2);

    if (big_endian)
    {
        bytes[1] = buf[0];
        bytes[0] = buf[1];
        buf = bytes;
    }
    
    *i = *((unsigned short *) buf);

    return(READSTAT);
} 

int
rs_read_int(int inf, int *i)
{
    unsigned char bytes[4];
    int  input;
    unsigned char *buf = (unsigned char *)&input;
    
    rs_read(inf, &input, 4);

    if (big_endian)
    {
        bytes[3] = buf[0];
        bytes[2] = buf[1];
        bytes[1] = buf[2];
        bytes[0] = buf[3];
        buf = bytes;
    }
    
    *i = *((int *) buf);

    return(READSTAT);
}

int
rs_read_ints(int inf, int *i, int count)
{
    int n = 0, value = 0;
    
    if (rs_read_int(inf,&value) != 0)
    {
        if (value != count)
            format_error = TRUE;
        else
        {
            for(n = 0; n < value; n++)
                rs_read_int(inf, &i[n]);
        }
    }
    
    return(READSTAT);
}

int
rs_read_uint(int inf, unsigned int *i)
{
    unsigned char bytes[4];
    int  input;
    unsigned char *buf = (unsigned char *)&input;
    
    rs_read(inf, &input, 4);

    if (big_endian)
    {
        bytes[3] = buf[0];
        bytes[2] = buf[1];
        bytes[1] = buf[2];
        bytes[0] = buf[3];
        buf = bytes;
    }
    
    *i = *((unsigned int *) buf);

    return(READSTAT);
}

int
rs_read_long(int inf, long *i)
{
    unsigned char bytes[4];
    long input;
    unsigned char *buf = (unsigned char *) &input;
    
    rs_read(inf, &input, 4);

    if (big_endian)
    {
        bytes[3] = buf[0];
        bytes[2] = buf[1];
        bytes[1] = buf[2];
        bytes[0] = buf[3];
        buf = bytes;
    }
    
    *i = *((long *) buf);

    return(READSTAT);
}

int
rs_read_longs(int inf, long *i, int count)
{
    int n = 0, value = 0;
    
    if (rs_read_int(inf,&value) != 0)
    {
        if (value != count)
            format_error = TRUE;
        else
        {
            for(n = 0; n < value; n++)
                rs_read_long(inf, &i[n]);
        }
    }
    
    return(READSTAT);
}

int
rs_read_ulong(int inf, unsigned long *i)
{
    unsigned char bytes[4];
    unsigned long input;
    unsigned char *buf = (unsigned char *) &input;
    
    rs_read(inf, &input, 4);

    if (big_endian)
    {
        bytes[3] = buf[0];
        bytes[2] = buf[1];
        bytes[1] = buf[2];
        bytes[0] = buf[3];
        buf = bytes;
    }
    
    *i = *((unsigned long *) buf);

    return(READSTAT);
}

int
rs_read_ulongs(int inf, unsigned long *i, int count)
{
    int n = 0, value = 0;
    
    if (rs_read_int(inf,&value) != 0)
    {
        if (value != count)
            format_error = TRUE;
        else
        {
            for(n = 0; n < value; n++)
                rs_read_ulong(inf, &i[n]);
        }
    }
    
    return(READSTAT);
}

int
rs_read_string(int inf, char *s, int max)
{
    int len = 0;

    if (rs_read_int(inf, &len) != FALSE)
    {
        if (len > max)
        {
            printf("String too long to restore. %d > %d\n",len,max);
            printf("Sorry, invalid save game format\n");
            format_error = TRUE;
        }
    
        rs_read(inf, s, len);
    }
    
    return(READSTAT);
}

int
rs_read_new_string(int inf, char **s)
{
    int len=0;
    char *buf=0;

    if (rs_read_int(inf, &len) != 0)
    {
        if (len == 0)
            *s = NULL;
        else
        { 
            buf = malloc(len);

            if (buf == NULL)            
                read_error = TRUE;
            else
            {
                rs_read(inf, buf, len);
                *s = buf;
            }
        }
    }

    return(READSTAT);
}

int
rs_read_string_index(int inf, const char *master[], int maxindex, 
                     const char **str)
{
    int i;

    if (rs_read_int(inf,&i) != 0)
    {
        if (i > maxindex)
        {
            printf("String index is out of range. %d > %d\n", i, maxindex);
            printf("Sorry, invalid save game format\n");
            format_error = TRUE;
        }
        else if (i >= 0)
            *str = master[i];
        else
            *str = NULL;
    }

    return(READSTAT);
}

int
rs_read_strings(int inf, char **s, int count, int max)
{
    int n     = 0;
    int value = 0;
    
    if (rs_read_int(inf,&value) != 0)
    {
        if (value != count)
        {
            printf("Incorrect number of strings in block. %d > %d.", 
                value, count);
            printf("Sorry, invalid save game format");
            format_error = TRUE;
        }
        else
        {
            for(n = 0; n < value; n++)
            {
                rs_read_string(inf, s[n], max);
            }
        }
    }
    
    return(READSTAT);
}

int
rs_read_new_strings(int inf, char **s, int count)
{
    int len   = 0;
    int n     = 0;
    int value = 0;
    
    if (rs_read_int(inf,&value) != 0)
    {
        if (value != count)
        {
            printf("Incorrect number of new strings in block. %d > %d.",
                value,count);abort();
            printf("Sorry, invalid save game format");
            format_error = TRUE;
        }
        else
            for(n=0; n<value; n++)
            {
                rs_read_int(inf, &len);
            
                if (len == 0)
                    s[n]=0;
                else 
                {
                    s[n] = malloc(len);
                    rs_read(inf,s[n],len);
                }
            }
    }
    
    return(READSTAT);
}

/******************************************************************************/

int
rs_write_str_t(FILE *savef, str_t st)
{
    rs_write_uint(savef,st);

    return(WRITESTAT);
}

int
rs_read_str_t(int inf, str_t *st)
{
    rs_read_uint(inf,st);
    
    return(READSTAT);
}

int
rs_write_coord(FILE *savef, coord c)
{
    rs_write_shint(savef, c.x);
    rs_write_shint(savef, c.y);
    
    return(WRITESTAT);
}

int
rs_read_coord(int inf, coord *c)
{
    rs_read_shint(inf,&c->x);
    rs_read_shint(inf,&c->y);
    
    return(READSTAT);
}

int
rs_write_window(FILE *savef, WINDOW *win)
{
    int row,col,height,width;
    width = getmaxx(win);
    height = getmaxy(win);

    rs_write_int(savef,RSID_WINDOW);
    rs_write_int(savef,height);
    rs_write_int(savef,width);
    
    for(row=0;row<height;row++)
        for(col=0;col<width;col++)
            rs_write_int(savef, mvwinch(win,row,col));
}

int
rs_read_window(int inf, WINDOW *win)
{
    int id,row,col,maxlines,maxcols,value,width,height;
    
    width = getmaxx(win);
    height = getmaxy(win);

    if (rs_read_int(inf, &id) != 0)
    {
        if (id != RSID_WINDOW)
        {
            printf("Invalid head id. %x != %x(RSID_WINDOW)\n", id, RSID_WINDOW);
            printf("Sorry, invalid save game format");
            format_error = TRUE;
        }   
        else
        {
            rs_read_int(inf,&maxlines);
            rs_read_int(inf,&maxcols);
            if (maxlines > height)
               abort();
            if (maxcols > width)
               abort();
               
            for(row=0;row<maxlines;row++)
                for(col=0;col<maxcols;col++)
                {
                    rs_read_int(inf, &value);
                    mvwaddch(win,row,col,value);
                }
        }
    }
        
    return(READSTAT);
}

int
rs_write_daemons(FILE *savef, struct delayed_action *d_list, int count)
{
    int i = 0;
    int func = 0;
        
    rs_write_int(savef, RSID_DAEMONS);
    rs_write_int(savef, count);
        
    for(i = 0; i < count; i++)
    {
        if (d_list[i].d_func == rollwand)
            func = 1;
        else if (d_list[i].d_func == doctor)
            func = 2;
        else if (d_list[i].d_func == stomach)
            func = 3;
        else if (d_list[i].d_func == runners)
            func = 4;
        else if (d_list[i].d_func == swander)
            func = 5;
        else if (d_list[i].d_func == nohaste)
            func = 6;
        else if (d_list[i].d_func == unconfuse)
            func = 7;
        else if (d_list[i].d_func == unsee)
            func = 8;
        else if (d_list[i].d_func == sight)
            func = 9;
        else
            func = 0;

        rs_write_int(savef, d_list[i].d_type);
        rs_write_int(savef, func);
        rs_write_int(savef, d_list[i].d_arg);
        rs_write_int(savef, d_list[i].d_time);
    }
    
    return(WRITESTAT);
}       

int
rs_read_daemons(int inf, struct delayed_action *d_list, int count)
{
    int i = 0;
    int func = 0;
    int value = 0;
    int id = 0;
    
    if (d_list == NULL)
        printf("HELP THERE ARE NO DAEMONS\n");
    
    if (rs_read_int(inf, &id) != 0)
    {
        if (id != RSID_DAEMONS)
        {
            printf("Invalid id. %x != %x(RSID_DAEMONS)\n", id, RSID_DAEMONS);
            printf("Sorry, invalid save game format");
            format_error = TRUE;
        }
        else if (rs_read_int(inf, &value) != 0)
        {
            if (value > count)
            {
                printf("Incorrect number of daemons in block. %d > %d.",
                    value, count);
                printf("Sorry, invalid save game format");
                format_error = TRUE;
            }
            else
            {
                for(i=0; i < value; i++)
                {
                    func = 0;
                    rs_read_int(inf, &d_list[i].d_type);
                    rs_read_int(inf, &func);
                    rs_read_int(inf, &d_list[i].d_arg);
                    rs_read_int(inf, &d_list[i].d_time);
                    
                    switch(func)
                    {
                        case 1: d_list[i].d_func = rollwand;
                                break;
                        case 2: d_list[i].d_func = doctor;
                                break;
                        case 3: d_list[i].d_func = stomach;
                                break;
                        case 4: d_list[i].d_func = runners;
                                break;
                        case 5: d_list[i].d_func = swander;
                                break;
                        case 6: d_list[i].d_func = nohaste;
                                break;
                        case 7: d_list[i].d_func = unconfuse;
                                break;
                        case 8: d_list[i].d_func = unsee;
                                break;
                        case 9: d_list[i].d_func = sight;
                                break;
                        default:d_list[i].d_func = NULL;
                                break;
                    }   
                }
            }
        }
    }
    
    return(READSTAT);
}       
        
int
rs_write_magic_items(FILE *savef, struct magic_item *i, int count)
{
    int n;
    
    rs_write_int(savef, RSID_MAGICITEMS);
    rs_write_int(savef, count);

    for(n = 0; n < count; n++)
    {
        /* mi_name is constant, defined at compile time in all cases */
        rs_write_shint(savef,i[n].mi_prob);
        rs_write_short(savef,i[n].mi_worth);
    }
    
    return(WRITESTAT);
}

int
rs_read_magic_items(int inf, struct magic_item *mi, int count)
{
    int id;
    int n;
    int value;

    if (rs_read_int(inf, &id) != 0)
    {
        if (id != RSID_MAGICITEMS)
        {
            printf("Invalid id. %x != %x(RSID_MAGICITEMS)\n",
                id, RSID_MAGICITEMS);
            printf("Sorry, invalid save game format");
            format_error = TRUE;
        }   
        else if (rs_read_int(inf, &value) != 0)
        {
            if (value > count)
            {
                printf("Incorrect number of magic items in block. %d > %d.",
                    value, count);
                printf("Sorry, invalid save game format");
                format_error = TRUE;
            }
            else
            {
                for(n = 0; n < value; n++)
                {
                    /* mi_name is constant, defined at compile time in all cases */
                    rs_read_shint(inf,&mi[n].mi_prob);
                    rs_read_short(inf,&mi[n].mi_worth);
                }
            }
        }
    }
    
    return(READSTAT);
}

int
rs_write_room(FILE *savef, struct room *r)
{
    rs_write_coord(savef, r->r_pos);
    rs_write_coord(savef, r->r_max);
    rs_write_coord(savef, r->r_gold);
    rs_write_int(savef,   r->r_goldval);
    rs_write_short(savef, r->r_flags);
    rs_write_shint(savef, r->r_nexits);
    rs_write_coord(savef, r->r_exit[0]);
    rs_write_coord(savef, r->r_exit[1]);
    rs_write_coord(savef, r->r_exit[2]);
    rs_write_coord(savef, r->r_exit[3]);
    rs_write_coord(savef, r->r_exit[4]);
    rs_write_coord(savef, r->r_exit[5]);
    rs_write_coord(savef, r->r_exit[6]);
    rs_write_coord(savef, r->r_exit[7]);
    rs_write_coord(savef, r->r_exit[8]);
    rs_write_coord(savef, r->r_exit[9]);
    rs_write_coord(savef, r->r_exit[10]);
    rs_write_coord(savef, r->r_exit[11]);
    
    return(WRITESTAT);
}

int
rs_write_rooms(FILE *savef, struct room r[], int count)
{
    int n = 0;

    rs_write_int(savef, count);
    
    for(n=0; n<count; n++)
        rs_write_room(savef, &r[n]);
    
    return(WRITESTAT);
}

int
rs_read_room(int inf, struct room *r)
{
    rs_read_coord(inf,&r->r_pos);
    rs_read_coord(inf,&r->r_max);
    rs_read_coord(inf,&r->r_gold);
    rs_read_int(inf,&r->r_goldval);
    rs_read_short(inf,&r->r_flags);
    rs_read_shint(inf,&r->r_nexits);
    rs_read_coord(inf,&r->r_exit[0]);
    rs_read_coord(inf,&r->r_exit[1]);
    rs_read_coord(inf,&r->r_exit[2]);
    rs_read_coord(inf,&r->r_exit[3]);
    rs_read_coord(inf,&r->r_exit[4]);
    rs_read_coord(inf,&r->r_exit[5]);
    rs_read_coord(inf,&r->r_exit[6]);
    rs_read_coord(inf,&r->r_exit[7]);
    rs_read_coord(inf,&r->r_exit[8]);
    rs_read_coord(inf,&r->r_exit[9]);
    rs_read_coord(inf,&r->r_exit[10]);
    rs_read_coord(inf,&r->r_exit[11]);

    return(READSTAT);
}

int
rs_read_rooms(int inf, struct room *r, int count)
{
    int value = 0, n = 0;

    if (rs_read_int(inf,&value) != 0)
    {
        if (value > count)
        {
            printf("Incorrect number of rooms in block. %d > %d.",
                value,count);
            printf("Sorry, invalid save game format");
            format_error = TRUE;
        }
        else
            for(n = 0; n < value; n++)
                rs_read_room(inf,&r[n]);
    }

    return(READSTAT);
}

int
rs_write_room_reference(FILE *savef, struct room *rp)
{
    int i, room = -1;
    
    for (i = 0; i < MAXROOMS; i++)
        if (&rooms[i] == rp)
            room = i;

    rs_write_int(savef, room);

    return(WRITESTAT);
}

int
rs_read_room_reference(int inf, struct room **rp)
{
    int i;
    
    rs_read_int(inf, &i);

    *rp = &rooms[i];
            
    return(READSTAT);
}

int
rs_write_stats(FILE *savef, struct stats *s)
{
    rs_write_int(savef, RSID_STATS);
    rs_write_str_t(savef, s->s_str);
    rs_write_long(savef, s->s_exp);
    rs_write_shint(savef, s->s_lvl);
    rs_write_shint(savef, s->s_arm);
    rs_write_short(savef, s->s_hpt);
    rs_write(savef, s->s_dmg, sizeof(s->s_dmg));
    rs_write_shint(savef,s->s_maxhp);

    return(WRITESTAT);
}

int
rs_read_stats(int inf, struct stats *s)
{
    int id;

    rs_read_int(inf, &id);

    rs_read_str_t(inf,&s->s_str);
    rs_read_long(inf,&s->s_exp);
    rs_read_shint(inf,&s->s_lvl);
    rs_read_shint(inf,&s->s_arm);
    rs_read_short(inf,&s->s_hpt);
    rs_read(inf,s->s_dmg,sizeof(s->s_dmg));
    rs_read_shint(inf,&s->s_maxhp);

    return(READSTAT);
}

int
rs_write_object(FILE *savef, THING *o)
{
    rs_write_int(savef, RSID_OBJECT);
    rs_write_shint(savef, o->_o._o_type); 
    rs_write_coord(savef, o->_o._o_pos); 
    rs_write_char(savef, o->_o._o_launch);
    rs_write(savef, o->_o._o_damage, sizeof(o->_o._o_damage));
    rs_write(savef, o->_o._o_hurldmg, sizeof(o->_o._o_hurldmg));
    rs_write_shint(savef, o->_o._o_count);
    rs_write_shint(savef, o->_o._o_which);
    rs_write_shint(savef, o->_o._o_hplus);
    rs_write_shint(savef, o->_o._o_dplus);
    rs_write_short(savef, o->_o._o_ac);
    rs_write_short(savef, o->_o._o_flags);
    rs_write_shint(savef, o->_o._o_group);

    return(WRITESTAT);
}

int
rs_read_object(int inf, THING *o)
{
    int id;

    if (rs_read_int(inf, &id) != 0)
    {
        if (id != RSID_OBJECT)
        {
            printf("Invalid id. %x != %x(RSID_OBJECT)\n",
                id,RSID_OBJECT);
            printf("Sorry, invalid save game format");
            format_error = TRUE;
        }
        else
        {
            rs_read_shint(inf, &o->_o._o_type);
            rs_read_coord(inf, &o->_o._o_pos);
            rs_read_char(inf, &o->_o._o_launch);
            rs_read(inf, &o->_o._o_damage, sizeof(o->_o._o_damage));
            rs_read(inf, &o->_o._o_hurldmg, sizeof(o->_o._o_hurldmg));
            rs_read_shint(inf, &o->_o._o_count);
            rs_read_shint(inf, &o->_o._o_which);
            rs_read_shint(inf, &o->_o._o_hplus);
            rs_read_shint(inf, &o->_o._o_dplus);
            rs_read_short(inf, &o->_o._o_ac);
            rs_read_short(inf, &o->_o._o_flags);
            rs_read_shint(inf, &o->_o._o_group);
        }
    }
    
    return(READSTAT);
}

int
rs_write_object_list(FILE *savef, THING *l)
{
    rs_write_int(savef, RSID_OBJECTLIST);
    rs_write_int(savef, list_size(l));

    while (l != NULL) 
    {
        rs_write_object(savef, l);
        l = l->l_next;
    }
    
    return(WRITESTAT);
}

int
rs_read_object_list(int inf, THING **list)
{
    int id;
    int i, cnt;
    THING *l = NULL, *previous = NULL, *head = NULL;

    if (rs_read_int(inf,&id) != 0)
    {
        if (rs_read_int(inf,&cnt) != 0)
        {
            for (i = 0; i < cnt; i++) 
            {
                l = new_item(sizeof(THING));
                memset(l,0,sizeof(THING));
                l->l_prev = previous;
                if (previous != NULL)
                    previous->l_next = l;
                rs_read_object(inf,l);
                if (previous == NULL)
                    head = l;
                previous = l;
            }
            
            if (l != NULL)
                l->l_next = NULL;
    
            *list = head;
        }
        else
            format_error = TRUE;
    }
    else
        format_error = TRUE;


    return(READSTAT);
}

int
rs_write_object_reference(FILE *savef, THING *list, THING *item)
{
    int i;
    
    i = find_list_ptr(list, item);

    rs_write_int(savef, i);

    return(WRITESTAT);
}

int
rs_read_object_reference(int inf, THING *list, THING **item)
{
    int i;
    
    rs_read_int(inf, &i);

    *item = get_list_item(list,i);
            
    return(READSTAT);
}

int
find_thing_coord(THING *monlist, coord *c)
{
    THING *mitem;
    THING *tp;
    int i = 0;

    for(mitem = monlist; mitem != NULL; mitem = mitem->l_next)
    {
        tp = mitem;

        if (c == &tp->t_pos)
            return(i);

        i++;
    }

    return(-1);
}

int
find_room_coord(struct room *rmlist, coord *c, int n)
{
    int i = 0;
    
    for(i = 0; i < n; i++)
        if(&rmlist[i].r_gold == c)
            return(i);
    
    return(-1);
}

int
find_object_coord(THING *objlist, coord *c)
{
    THING *oitem;
    THING *obj;
    int i = 0;

    for(oitem = objlist; oitem != NULL; oitem = oitem->l_next)
    {
        obj = oitem;

        if (c == &obj->o_pos)
            return(i);

        i++;
    }

    return(-1);
}

int
rs_write_thing(FILE *savef, THING *t)
{
    int i = -1;
    
    if (t == NULL)
    {
        rs_write_int(savef, RSID_THING_NULL);
        return(WRITESTAT);
    }
    
    rs_write_int(savef, RSID_THING);

    rs_write_coord(savef, t->_t._t_pos);
    rs_write_boolean(savef, t->_t._t_turn);
    rs_write_char(savef, t->_t._t_type);
    rs_write_char(savef, t->_t._t_disguise);
    rs_write_char(savef, t->_t._t_oldch);

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
    
    rs_write_short(savef, t->_t._t_flags);
    rs_write_stats(savef, &t->_t._t_stats);
    rs_write_room_reference(savef, t->_t._t_room);
    rs_write_object_list(savef, t->_t._t_pack);
    
    return(WRITESTAT);
}

int
rs_fix_thing(THING *t)
{
    THING *item;
    THING *tp;

    if (t->t_reserved < 0)
        return;

    item = get_list_item(mlist,t->t_reserved);

    if (item != NULL)
    {
        tp = item;
        t->t_dest = &tp->t_pos;
    }
}

int
rs_fix_thing_list(THING *list)
{
    THING *item;

    for(item = list; item != NULL; item = item->l_next)
        rs_fix_thing(item);
}

int
rs_read_thing(int inf, THING *t)
{
    int id;
    int listid = 0, index = -1;
    THING *item;
        
    if (rs_read_int(inf, &id) != 0)
    {
        if (id != RSID_THING)
            format_error = TRUE;
        else
        {
            rs_read_coord(inf,&t->_t._t_pos);
            rs_read_boolean(inf,&t->_t._t_turn);
            rs_read_uchar(inf,&t->_t._t_type);
            rs_read_char(inf,&t->_t._t_disguise);
            rs_read_char(inf,&t->_t._t_oldch);
            
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
            
            rs_read_int(inf, &listid);
            rs_read_int(inf, &index);
            t->_t._t_reserved = -1;

            if (listid == 0) /* hero or NULL */
            {
                if (index == 1)
                    t->_t._t_dest = &hero;
                else
                    t->_t._t_dest = NULL;
            }
            else if (listid == 1) /* monster/thing */
            {
                t->_t._t_dest     = NULL;
                t->_t._t_reserved = index;
            }
            else if (listid == 2) /* object */
            {
                THING *obj;

                item = get_list_item(lvl_obj, index);

                if (item != NULL)
                {
                    obj = item;
                    t->_t._t_dest = &obj->o_pos;
                }
            }
            else if (listid == 3) /* gold */
            {
                t->_t._t_dest = &rooms[index].r_gold;
            }
            else
                t->_t._t_dest = NULL;
            
            rs_read_short(inf,&t->_t._t_flags);
            rs_read_stats(inf,&t->_t._t_stats);
            rs_read_room_reference(inf, &t->_t._t_room);
            rs_read_object_list(inf,&t->_t._t_pack);
        }
    }
    else 
        format_error = TRUE;
    
    return(READSTAT);
}

int
rs_write_thing_list(FILE *savef, THING *l)
{
    int cnt = 0;
    
    rs_write_int(savef, RSID_MONSTERLIST);

    cnt = list_size(l);

    rs_write_int(savef, cnt);

    if (cnt < 1)
        return(WRITESTAT);

    while (l != NULL) {
        rs_write_thing(savef, l);
        l = l->l_next;
    }
    
    return(WRITESTAT);
}

int
rs_read_thing_list(int inf, THING **list)
{
    int id;
    int i, cnt;
    THING *l = NULL, *previous = NULL, *head = NULL;

    if (rs_read_int(inf,&id) != 0)
    {
        if (id != RSID_MONSTERLIST)
        {
            printf("Invalid id. %x != %x(RSID_MONSTERLIST)\n",
                id,RSID_MONSTERLIST);
            printf("Sorry, invalid save game format");
            format_error = TRUE;
        }
        else if (rs_read_int(inf,&cnt) != 0)
        {
            for (i = 0; i < cnt; i++) 
            {
                l = new_item();
                l->l_prev = previous;
                if (previous != NULL)
                    previous->l_next = l;
                rs_read_thing(inf,l);
                if (previous == NULL)
                    head = l;
                previous = l;
            }
        

            if (l != NULL)
                l->l_next = NULL;

            *list = head;
        }
    }
    else 
        format_error = TRUE;
    
    return(READSTAT);
}

int
rs_write_monsters(FILE *savef, struct monster *m, int count)
{
    int n;
    
    rs_write_int(savef, RSID_MONSTERS);
    rs_write_int(savef, count);

    for(n=0;n<count;n++)
    {
        /*
        rs_write(savef, m[n].m_name, sizeof(m[n].m_name));
        rs_write_char(savef, m[n].m_carry);
        rs_write_short(savef, m[n].m_flags);
        */
        rs_write_stats(savef, &m[n].m_stats);
    }
    
    return(WRITESTAT);
}

int
rs_read_monsters(int inf, struct monster *m, int count)
{
    int id = 0, value = 0, n = 0;
    
    if (rs_read_int(inf, &id) != 0)
    {
        if (id != RSID_MONSTERS)
        {
            printf("Invalid id. %x != %x(RSID_MONSTERS)\n",
                id,RSID_MONSTERS);
            printf("Sorry, invalid save game format");
            format_error = TRUE;
        }
        else if (rs_read_int(inf, &value) != 0)
        {
            for(n=0;n<value;n++)
            {
                /*
                rs_read(inf,m[n].m_name,sizeof(m[n].m_name));
                rs_read_char(inf, &m[n].m_carry);
                rs_read_short(inf, &m[n].m_flags);
                */
                rs_read_stats(inf, &m[n].m_stats);
            }
        }
        else
            format_error = TRUE;
    }
    
    return(READSTAT);
}

int
rs_write_scrolls(FILE *savef)
{
    int i;

    for(i = 0; i < MAXSCROLLS; i++)
    {
        rs_write_string(savef,s_names[i]);
        rs_write_boolean(savef,s_know[i]);
        rs_write_string(savef,s_guess[i]);
    }

    return(READSTAT);
}

int
rs_read_scrolls(int inf)
{
    int i;

    for(i = 0; i < MAXSCROLLS; i++)
    {
        rs_read_new_string(inf,&s_names[i]);
        rs_read_boolean(inf,&s_know[i]);
        rs_read_new_string(inf,&s_guess[i]);
    }

    return(READSTAT);
}

int
rs_write_potions(FILE *savef)
{
    int i;

    for(i = 0; i < MAXPOTIONS; i++)
    {
        rs_write_string_index(savef,rainbow,NCOLORS,p_colors[i]);
        rs_write_boolean(savef,p_know[i]);
        rs_write_string(savef,p_guess[i]);
    }

    return(WRITESTAT);
}

int
rs_read_potions(int inf)
{
    int i;

    for(i = 0; i < MAXPOTIONS; i++)
    {
        rs_read_string_index(inf,rainbow,NCOLORS,&p_colors[i]);
        rs_read_boolean(inf,&p_know[i]);
        rs_read_new_string(inf,&p_guess[i]);
    }

    return(READSTAT);
}


int
rs_write_rings(FILE *savef)
{
    int i;
    const char *stones_list[NSTONES];

    for(i = 0; i < NSTONES; i++)
        stones_list[i] = stones[i].st_name;

    for(i = 0; i < MAXRINGS; i++)
    {
        rs_write_string_index(savef,stones_list,NSTONES,r_stones[i]);
        rs_write_boolean(savef,r_know[i]);
        rs_write_string(savef,r_guess[i]);
    }

    return(WRITESTAT);
}

int
rs_read_rings(int inf)
{
    int i;
    const char *stones_list[NSTONES];

    for(i = 0; i < NSTONES; i++)
        stones_list[i] = stones[i].st_name;

    for(i = 0; i < MAXRINGS; i++)
    {
        rs_read_string_index(inf,stones_list,NSTONES,&r_stones[i]);
        rs_read_boolean(inf,&r_know[i]);
        rs_read_new_string(inf,&r_guess[i]);
    }

    return(READSTAT);
}

int
rs_write_sticks(FILE *savef)
{
    int i;

    for (i = 0; i < MAXSTICKS; i++)
    {
        if (strcmp(ws_type[i],"staff") == 0)
        {
            rs_write_int(savef,0);
            rs_write_string_index(savef, wood, NWOOD, ws_made[i]);
        }
        else
        {
            rs_write_int(savef,1);
            rs_write_string_index(savef, metal, NMETAL, ws_made[i]);
        }
        rs_write_boolean(savef, ws_know[i]);
        rs_write_string(savef, ws_guess[i]);
    }
 
    return(WRITESTAT);
}
        
int
rs_read_sticks(int inf)
{
    int i = 0, list = 0;

    for(i = 0; i < MAXSTICKS; i++)
    { 
        rs_read_int(inf,&list);
        if (list == 0)
        {
            rs_read_string_index(inf,wood,NWOOD,&ws_made[i]);
            ws_type[i] = "staff";
        }
        else 
        {
            rs_read_string_index(inf,metal,NMETAL,&ws_made[i]);
            ws_type[i] = "wand";
        }
        rs_read_boolean(inf, &ws_know[i]);
        rs_read_new_string(inf, &ws_guess[i]);
    }

    return(READSTAT);
}

/******************************************************************************/
int
rs_write_thing_reference(FILE *savef, THING *list, THING *item)
{
    int i;

    if (item == NULL)
        rs_write_int(savef,-1);
    else
    {
        i = find_list_ptr(list, item);

        assert(i >= 0);

        rs_write_int(savef, i);
    }

    return(WRITESTAT);
}

int
rs_read_thing_reference(int inf, THING *list, THING **item)
{
    int i;
    
    rs_read_int(inf, &i);

    if (i == -1)
        *item = NULL;
    else
    {
        *item = get_list_item(list,i);

        assert(item != NULL);
    }        

    return(READSTAT);
}

int
rs_write_thing_references(FILE *savef, THING *list, THING *items[], int count)
{
    int i;

    for(i = 0; i < count; i++)
        rs_write_thing_reference(savef,list,items[i]);

    return(WRITESTAT);
}

int
rs_read_thing_references(int inf, THING *list, THING *items[], int count)
{
    int i;

    for(i = 0; i < count; i++)
        rs_read_thing_reference(inf,list,&items[i]);

    return(WRITESTAT);
}

int
rs_save_file(FILE *savef)
{
    int endian = 0x01020304;
    big_endian = ( *((char *)&endian) == 0x01 );

    rs_write_boolean(savef, after);
    rs_write_boolean(savef, noscore);
    rs_write_boolean(savef, amulet);
    rs_write_boolean(savef, askme);
    rs_write_boolean(savef, door_stop);
    rs_write_boolean(savef, fight_flush);
    rs_write_boolean(savef, firstmove);
    rs_write_boolean(savef, in_shell);
    rs_write_boolean(savef, jump);
    rs_write_boolean(savef, passgo);
    rs_write_boolean(savef, playing);
    rs_write_boolean(savef, running);
    rs_write_boolean(savef, save_msg);
    rs_write_boolean(savef, slow_invent);
    rs_write_boolean(savef, terse);
#ifdef WIZARD
    rs_write_boolean(savef, wizard);
#else
    rs_write_boolean(savef, 0);
#endif
    rs_write_char(savef, take);
    rs_write(savef, prbuf, MAXSTR);
    rs_write_char(savef, runch);

    rs_write_scrolls(savef);
    rs_write_potions(savef);
    rs_write_rings(savef);
    rs_write_sticks(savef);

    rs_write_string(savef, release);
    rs_write(savef, whoami, MAXSTR);
    rs_write(savef, fruit, MAXSTR);

    rs_write(savef, _level, MAXLINES*MAXCOLS);
    rs_write(savef, _flags, MAXLINES*MAXCOLS);

    rs_write_int(savef, max_level);
    rs_write_int(savef, ntraps);
    rs_write_int(savef, dnum);
    rs_write_int(savef, level);
    rs_write_int(savef, purse);
    rs_write_int(savef, no_move);
    rs_write_int(savef, no_command);
    rs_write_int(savef, inpack);
    rs_write_int(savef, lastscore);
    rs_write_int(savef, no_food);
    rs_write_int(savef, count);
    rs_write_int(savef, fung_hit);
    rs_write_int(savef, quiet);
    rs_write_int(savef, food_left);
    rs_write_int(savef, group);
    rs_write_int(savef, hungry_state);

    /* rs_write_ints(savef, a_chances, MAXARMORS); *//* constant */
    /* rs_write_ints(savef, a_class, MAXARMORS);   *//* constant */

    rs_write_long(savef, seed);
    rs_write_coord(savef, oldpos);
    rs_write_coord(savef, delta);

    rs_write_thing(savef, &player);                     
    rs_write_object_reference(savef, player.t_pack, cur_armor);
   	rs_write_object_reference(savef, player.t_pack, cur_weapon); 
    rs_write_object_reference(savef, player.t_pack, cur_ring[0]);
    rs_write_object_reference(savef, player.t_pack, cur_ring[1]); 

    rs_write_object_list(savef, lvl_obj);               
    rs_write_thing_list(savef, mlist);                
    rs_write_thing_references(savef, mlist, _monst, MAXLINES*MAXCOLS);

    rs_write_window(savef, stdscr);
    rs_write_stats(savef,&max_stats); 

    rs_write_rooms(savef, rooms, MAXROOMS);             
    rs_write_room_reference(savef, oldrp);              
    rs_write_rooms(savef, passages, MAXPASS);

    rs_write_monsters(savef,monsters,26);               
    rs_write_magic_items(savef, things,   NUMTHINGS);   
    rs_write_magic_items(savef, s_magic,  MAXSCROLLS);  
    rs_write_magic_items(savef, p_magic,  MAXPOTIONS);  
    rs_write_magic_items(savef, r_magic,  MAXRINGS);    
    rs_write_magic_items(savef, ws_magic, MAXSTICKS);      
    
    rs_write_coord(savef, ch_ret);                      /* 5.2-chase.c  */
    rs_write_char(savef,countch);                       /* 5.2-command.c*/
    rs_write_char(savef,direction);                     /* 5.2-command.c*/
    rs_write_char(savef,newcount);                      /* 5.2-command.c*/
    rs_write_daemons(savef, &d_list[0], 20);            /* 5.2-daemon.c */
    rs_write_int(savef,between);                        /* 5.2-daemons.c*/
    rs_write(savef,lvl_mons,sizeof(lvl_mons));          /* 5.2-monsters.c*/
    rs_write(savef,wand_mons,sizeof(wand_mons));        /* 5.2-monsters.c*/
    rs_write_coord(savef, nh);                          /* 5.2-move.c    */
    rs_write_boolean(savef, got_genocide);              /* 5.2-things.c  */

    return(WRITESTAT);
}

int
rs_restore_file(int inf)
{
    bool junk;
    int endian = 0x01020304;
    big_endian = ( *((char *)&endian) == 0x01 );
    
    rs_read_boolean(inf, &after);
    rs_read_boolean(inf, &noscore);
    rs_read_boolean(inf, &amulet);
    rs_read_boolean(inf, &askme);
    rs_read_boolean(inf, &door_stop);
    rs_read_boolean(inf, &fight_flush);
    rs_read_boolean(inf, &firstmove);
    rs_read_boolean(inf, &in_shell);
    rs_read_boolean(inf, &jump);
    rs_read_boolean(inf, &passgo);
    rs_read_boolean(inf, &playing);
    rs_read_boolean(inf, &running);
    rs_read_boolean(inf, &save_msg);
    rs_read_boolean(inf, &slow_invent);
    rs_read_boolean(inf, &terse);
#ifdef WIZARD
    rs_read_boolean(inf, &wizard);
#else
    rs_read_boolean(inf, &junk);
#endif
    rs_read_char(inf, &take);
    rs_read(inf, prbuf, MAXSTR);
    rs_read_char(inf, &runch);

    rs_read_scrolls(inf);
    rs_read_potions(inf);
    rs_read_rings(inf);
    rs_read_sticks(inf);

    rs_read_new_string(inf, &release);
    rs_read(inf, whoami, MAXSTR);
    rs_read(inf, fruit, MAXSTR);

    rs_read(inf, _level, MAXLINES*MAXCOLS);
    rs_read(inf, _flags, MAXLINES*MAXCOLS);

    rs_read_int(inf, &max_level);
    rs_read_int(inf, &ntraps);
    rs_read_int(inf, &dnum);
    rs_read_int(inf, &level);
    rs_read_int(inf, &purse);
    rs_read_int(inf, &no_move);
    rs_read_int(inf, &no_command);
    rs_read_int(inf, &inpack);
    rs_read_int(inf, &lastscore);
    rs_read_int(inf, &no_food);
    rs_read_int(inf, &count);
    rs_read_int(inf, &fung_hit);
    rs_read_int(inf, &quiet);
    rs_read_int(inf, &food_left);
    rs_read_int(inf, &group);
    rs_read_int(inf, &hungry_state);

    rs_read_long(inf, &seed);
    rs_read_coord(inf, &oldpos);
    rs_read_coord(inf, &delta);

    rs_read_thing(inf, &player); 
    rs_read_object_reference(inf, player.t_pack, &cur_armor);
    rs_read_object_reference(inf, player.t_pack, &cur_weapon);
    rs_read_object_reference(inf, player.t_pack, &cur_ring[0]);
    rs_read_object_reference(inf, player.t_pack, &cur_ring[1]);

    rs_read_object_list(inf, &lvl_obj);                 
    rs_read_thing_list(inf, &mlist);                  
    rs_fix_thing(&player);
    rs_fix_thing_list(mlist);
    rs_read_thing_references(inf,mlist,_monst,MAXLINES*MAXCOLS);

    rs_read_window(inf, stdscr);
    rs_read_stats(inf, &max_stats);

    rs_read_rooms(inf, rooms, MAXROOMS);
    rs_read_room_reference(inf, &oldrp);
    rs_read_rooms(inf, passages, MAXPASS);

    rs_read_monsters(inf,monsters,26);                  
    rs_read_magic_items(inf, things,   NUMTHINGS);         
    rs_read_magic_items(inf, s_magic,  MAXSCROLLS);       
    rs_read_magic_items(inf, p_magic,  MAXPOTIONS);       
    rs_read_magic_items(inf, r_magic,  MAXRINGS);         
    rs_read_magic_items(inf, ws_magic, MAXSTICKS);       

    rs_read_coord(inf, &ch_ret);                        /* 5.2-chase.c      */
    rs_read_char(inf,&countch);                         /* 5.2-command.c    */
    rs_read_char(inf,&direction);                       /* 5.2-command.c    */
    rs_read_char(inf,&newcount);                        /* 5.2-command.c    */
    rs_read_daemons(inf, d_list, 20);                   /* 5.2-daemon.c     */
    rs_read_int(inf,&between);                          /* 5.2-daemons.c    */
    rs_read(inf, lvl_mons, sizeof(lvl_mons));           /* 5.2-monsters.c   */
    rs_read(inf, wand_mons, sizeof(wand_mons));         /* 5.2-monsters.c   */
    rs_read_coord(inf, &nh);                            /* 5.2-move.c       */
    rs_read_boolean(inf, &got_genocide);                /* 5.2-things.c     */
    
    return(READSTAT);
}
