/************************************************************************/
/* Save State Code                                                      */
/************************************************************************/

#define RSID_STATS        0xABCD0001
#define RSID_THING        0xABCD0002
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
#include "rogue.h"

#define READSTAT ((format_error == 0) && (read_error == 0))
#define WRITESTAT (write_error == 0)

int read_error   = FALSE;
int write_error  = FALSE;
int format_error = FALSE;
int end_of_file  = FALSE;
int big_endian   = 0;

void *
get_list_item(struct linked_list *l, int i)
{
    int count = 0;

    while(l != NULL)
    {   
        if (count == i)
            return(l->l_data);
                        
        l = l->l_next;
        
        count++;
    }
    
    return(NULL);
}

int
find_list_ptr(struct linked_list *l, void *ptr)
{
    int count = 0;

    while(l != NULL)
    {
        if (l->l_data == ptr)
            return(count);
            
        l = l->l_next;
        count++;
    }
    
    return(-1);
}

int
list_size(struct linked_list *l)
{
    int count = 0;
    
    while(l != NULL)
    {
        if (l->l_data == NULL)
            return(count);
            
        count++;
        
        l = l->l_next;
    }
    
    return(count);
}

rs_write(FILE *savef, void *ptr, int size)
{
    if (!write_error)
        encwrite(ptr,size,savef);
    if (0)
        write_error = TRUE;
        
    assert(write_error == 0);
    return(WRITESTAT);
}

rs_read(int inf, void *ptr, int size)
{
    int actual;
    end_of_file =FALSE;
    if (!read_error && !format_error)
    {
        actual = encread(ptr, size, inf);

        if ((actual == 0) && (size != 0))
           end_of_file = TRUE;
    }
       
    if (read_error){
        printf("read error has occurred. restore short-circuited.\n");abort();}
    if (format_error)
        {printf("game format invalid. restore short-circuited.\n");abort();}

    return(READSTAT);
}

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

    return(WRITESTAT);
}

rs_write_ulong(FILE *savef, unsigned long c)
{
    char bytes[4];
    char *buf = (char *)&c;
    
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

rs_write_long(FILE *savef, long c)
{
    char bytes[4];
    char *buf = (char *)&c;
    
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

rs_write_boolean(FILE *savef, bool c)
{
    char buf;
    
    if (c == 0)
        buf = 0;
    else
        buf = 1;
    
    rs_write(savef, &buf, 1);

    return(WRITESTAT);
}

rs_read_int(int inf, int *i)
{
    char bytes[4];
    int  input;
    char *buf = (char *)&input;
    
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

rs_read_ulong(int inf, unsigned long *i)
{
    char bytes[4];
    unsigned long input;
    char *buf = (char *) &input;
    
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

rs_read_long(int inf, long *i)
{
    char bytes[4];
    long input;
    char *buf = (char *) &input;
    
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

rs_read_boolean(int inf, bool *i)
{
    char buf;
    
    rs_read(inf, &buf, 1);
    
    *i = buf;
    
    return(READSTAT);
}

rs_write_ints(FILE *savef, int *c, int count)
{
    int n=0;

    rs_write_int(savef,count);
    
    for(n=0;n<count;n++)
        rs_write_int(savef,c[n]);

    return(WRITESTAT);
}

rs_write_short(FILE *savef, short c)
{
    char bytes[2];
    char *buf = (char *) &c;
    
    if (big_endian)
    {
        bytes[1] = buf[0];
        bytes[0] = buf[1];
        buf = bytes;
    }
    
    rs_write(savef, buf, 2);

    return(WRITESTAT);
}

rs_read_short(int inf, short *s)
{
    char  bytes[2];
    short input;
    char *buf = (char *)&input;
    
    rs_read(inf, &input, 2);

    if (big_endian)
    {
        bytes[1] = buf[0];
        bytes[0] = buf[1];
        buf = bytes;
    }
    
    *s = *((short *) buf);
    return(READSTAT);
}

rs_write_shorts(FILE *savef, short *c, int count)
{
    int n=0;

    rs_write_int(savef,count);
    
    for(n=0;n<count;n++)
        rs_write_short(savef,c[n]);

    return(WRITESTAT);
}

rs_write_longs(FILE *savef, long *c, int count)
{
    int n=0;

    rs_write_int(savef,count);
    
    for(n=0;n<count;n++)
        rs_write_long(savef,c[n]);

    return(WRITESTAT);
}

rs_write_ulongs(FILE *savef, unsigned long *c, int count)
{
    int n=0;

    rs_write_int(savef,count);
    
    for(n=0;n<count;n++)
        rs_write_ulong(savef,c[n]);

    return(WRITESTAT);
}

rs_write_booleans(FILE *savef, bool *c, int count)
{
    int n=0;

    rs_write_int(savef,count);
    
    for(n=0;n<count;n++)
        rs_write_boolean(savef,c[n]);

    return(WRITESTAT);
}

rs_read_ints(int inf, int *i, int count)
{
    int n=0,value=0;
    
    if (rs_read_int(inf,&value) != 0)
    {
        if (value != count)
            format_error = TRUE;
        else
        {
            for(n=0;n<value;n++)
                rs_read_int(inf, &i[n]);
        }
    }
    
    return(READSTAT);
}

rs_read_shorts(int inf, short *i, int count)
{
    int n=0,value=0;
    
    if (rs_read_int(inf,&value) != 0)
    {
        if (value != count)
            format_error = TRUE;
        else
        {
            for(n=0;n<value;n++)
                rs_read_short(inf, &i[n]);
        }
    }
    
    return(READSTAT);
}

rs_read_longs(int inf, long *i, int count)
{
    int n=0,value=0;
    
    if (rs_read_int(inf,&value) != 0)
    {
        if (value != count)
            format_error = TRUE;
        else
        {
            for(n=0;n<value;n++)
                rs_read_long(inf, &i[n]);
        }
    }
    
    return(READSTAT);
}

rs_read_ulongs(int inf, unsigned long *i, int count)
{
    int n=0,value=0;
    
    if (rs_read_int(inf,&value) != 0)
    {
        if (value != count)
            format_error = TRUE;
        else
        {
            for(n=0;n<value;n++)
                rs_read_ulong(inf, &i[n]);
        }
    }
    
    return(READSTAT);
}

rs_read_booleans(int inf, bool *i, int count)
{
    int n=0,value=0;
    
    if (rs_read_int(inf,&value) != 0)
    {
        if (value != count)
        {
            printf("Invalid booleans block. %d != requested %d\n",value,count); 
            format_error = TRUE;
        }
        else
        {
            for(n=0;n<value;n++)
                rs_read_boolean(inf, &i[n]);
        }
    }
    
    return(READSTAT);
}

rs_write_char(FILE *savef, char c)
{
    rs_write(savef, &c, 1);
    
    return(WRITESTAT);
}

rs_read_char(int inf, char *c)
{
    rs_read(inf, c, 1);
    
    return(READSTAT);
}

rs_write_string(FILE *savef, char *s)
{
    int len = 0;

    len = (s == NULL) ? 0 : strlen(s) + 1;

    rs_write_int(savef, len);
    rs_write(savef, s, len);
            
    return(WRITESTAT);
}

rs_read_string_index(int inf, struct words master[], int maxindex, char **str)
{
    int i;

    if (rs_read_int(inf,&i) != 0)
    {
        if (i > maxindex)
        {
            printf("String index is out of range. %d > %d\n",      
                i, maxindex);
            printf("Sorry, invalid save game format\n");
            format_error = TRUE;
        }
        else if (i >= 0)
            *str = master[i].w_string;
        else
            *str = NULL;
    }
    return(READSTAT);
}

rs_write_string_index(FILE *savef, struct words master[], int max, char *str)
{
    int i;

    for(i = 0; i < max; i++)
    {
        if (str == master[i].w_string)
        {
            rs_write_int(savef,i);
            return(WRITESTAT);
        }
    }

    rs_write_int(savef,-1);
    return(WRITESTAT);
}

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

rs_write_words(FILE *savef, struct words *w, int count)
{
    int n = 0;

    rs_write_int(savef,count);

    for(n = 0; n < count; n++)
    {
        rs_write(savef, w[n].w_string, sizeof(w[n].w_string));
    }
    
    return(WRITESTAT);
}

rs_read_words(int inf, struct words *w, int count)
{
    int n = 0;
    int value = 0;

    rs_read_int(inf,&value);

    if (value != count)
    {
        printf("Incorrect number of words in block. %d != %d.",
                value,count);
        printf("Sorry, invalid save game format");
        format_error = TRUE;
    }
    else for(n = 0; n < count; n++)
    {
        rs_read(inf, w[n].w_string, sizeof(w[n].w_string));
    }
    
    return(READSTAT);
}

rs_read_new_strings(int inf, char **s, int count)
{
    int len   = 0;
    int n     = 0;
    int value = 0;
    
    if (rs_read_int(inf,&value) != 0)
    {
        if (value != count)
        {
            printf("Incorrect number of strings in block. %d > %d.",
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

rs_write_coord(FILE *savef, coord *c)
{
    rs_write_int(savef, c->x);
    rs_write_int(savef, c->y);
    
    return(WRITESTAT);
}

rs_read_coord(int inf, coord *c)
{
    rs_read_int(inf,&c->x);
    rs_read_int(inf,&c->y);
    
    return(READSTAT);
}

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

rs_read_window(int inf, WINDOW *win)
{
    int id,row,col,maxlines,maxcols,value,width,height;
    
    width = getmaxx(win);
    height = getmaxy(win);

    if (rs_read_int(inf, &id) != 0)
    {
        if (id != RSID_WINDOW)
        {
            printf("Invalid head id. %x != %x(RSID_WINDOW)\n",
                id,RSID_WINDOW);
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

struct delayed_action {
    int d_type;
    int (*d_func)();
    int d_arg;
    int d_time;
};

rs_write_daemons(FILE *savef, struct delayed_action *d_list,int count)
{
    int i = 0;
    int func = 0;
        
    rs_write_int(savef, RSID_DAEMONS);
    rs_write_int(savef, count);
        
    for(i=0; i < count; i++)
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
            printf("Invalid id. %x != %x(RSID_DAEMONS)\n",
                id,RSID_DAEMONS);
            printf("Sorry, invalid save game format");
            format_error = TRUE;
        }
        else if (rs_read_int(inf, &value) != 0)
        {
            if (value > count)
            {
                printf("Incorrect number of daemons in block. %d > %d.",
                    value,count);
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

rs_read_rings(int inf)
{
    int i;

    for(i = 0; i < MAXRINGS; i++)
    {
        rs_read_string_index(inf,stones,NSTONES,&r_stones[i]);
        rs_read_boolean(inf,&r_know[i]);
        rs_read_new_string(inf,&r_guess[i]);
    }

    return(READSTAT);
}

rs_write_rings(FILE *savef)
{
    int i;

    for(i = 0; i < MAXRINGS; i++)
    {
        rs_write_string_index(savef,stones,NSTONES,r_stones[i]);
        rs_write_boolean(savef,r_know[i]);
        rs_write_string(savef,r_guess[i]);
    }

    return(WRITESTAT);
}

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

int
find_room_coord(rmlist, c, n)
struct room *rmlist;
coord *c;
int n;
{
    int i = 0;
    
    for(i=0; i < n; i++)
        if(&rmlist[i].r_gold == c)
            return(i);
    
    return(-1);
}

rs_write_rooms(FILE *savef, struct room r[], int count)
{
    int n = 0;

    rs_write_int(savef, count);
    
    for(n=0; n<count; n++)
    {
        rs_write_coord(savef, &r[n].r_pos);
        rs_write_coord(savef, &r[n].r_max);
        rs_write_coord(savef, &r[n].r_gold);
        rs_write_int(savef, r[n].r_goldval);
        rs_write_int(savef, r[n].r_flags);
        rs_write_int(savef, r[n].r_nexits);
        rs_write_coord(savef, &r[n].r_exit[0]);
        rs_write_coord(savef, &r[n].r_exit[1]);
        rs_write_coord(savef, &r[n].r_exit[2]);
        rs_write_coord(savef, &r[n].r_exit[3]);
    }
    
    return(WRITESTAT);
}

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
            for(n=0; n<value; n++)
            {   
                rs_read_coord(inf,&r[n].r_pos);
                rs_read_coord(inf,&r[n].r_max);
                rs_read_coord(inf,&r[n].r_gold);
                rs_read_int(inf,&r[n].r_goldval);
                rs_read_int(inf,&r[n].r_flags);
                rs_read_int(inf,&r[n].r_nexits);
                rs_read_coord(inf,&r[n].r_exit[0]);
                rs_read_coord(inf,&r[n].r_exit[1]);
                rs_read_coord(inf,&r[n].r_exit[2]);
                rs_read_coord(inf,&r[n].r_exit[3]);
            }
    }

    return(READSTAT);
}

int
find_object_coord(objlist, c)
struct linked_list *objlist;
coord *c;
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

rs_write_object(FILE *savef, struct object *o)
{
    rs_write_int(savef, RSID_OBJECT);
    rs_write_int(savef, o->o_type);
    rs_write_coord(savef, &o->o_pos);
    rs_write_char(savef, o->o_launch);
    rs_write(savef, o->o_damage, sizeof(o->o_damage));
    rs_write(savef, o->o_hurldmg, sizeof(o->o_damage));
    rs_write_int(savef, o->o_count);
    rs_write_int(savef, o->o_which);
    rs_write_int(savef, o->o_hplus);
    rs_write_int(savef, o->o_dplus);
    rs_write_int(savef, o->o_ac);
    rs_write_int(savef, o->o_flags);
    rs_write_int(savef, o->o_group);
    return(WRITESTAT);
}

rs_read_object(int inf, struct object *o)
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
            rs_read_int(inf, &o->o_type);
            rs_read_coord(inf, &o->o_pos);
            rs_read_char(inf, &o->o_launch);
            rs_read(inf, o->o_damage, sizeof(o->o_damage));
            rs_read(inf, o->o_hurldmg, sizeof(o->o_hurldmg));
            rs_read_int(inf, &o->o_count);
            rs_read_int(inf, &o->o_which);
            rs_read_int(inf, &o->o_hplus);
            rs_read_int(inf, &o->o_hplus);
            rs_read_int(inf,&o->o_ac);
            rs_read_int(inf,&o->o_flags);
            rs_read_int(inf,&o->o_group);
        }
    }
    
    return(READSTAT);
}

rs_read_object_list(int inf, struct linked_list **list)
{
    int id;
    int i, cnt;
    struct linked_list *l = NULL, *previous = NULL, *head = NULL;

    if (rs_read_int(inf,&id) != 0)
    {
        if (rs_read_int(inf,&cnt) != 0)
        {
            for (i = 0; i < cnt; i++) 
            {
                l = new_item(sizeof(struct object));
                memset(l->l_data,0,sizeof(struct object));
                l->l_prev = previous;
                if (previous != NULL)
                    previous->l_next = l;
                rs_read_object(inf,(struct object *) l->l_data);
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

rs_write_object_list(FILE *savef, struct linked_list *l)
{
    rs_write_int(savef, RSID_OBJECTLIST);
    rs_write_int(savef, list_size(l));

    while (l != NULL) 
    {
        rs_write_object(savef, (struct object *) l->l_data);
        l = l->l_next;
    }
    
    return(WRITESTAT);
}

rs_write_stats(FILE *savef, struct stats *s)
{
    rs_write_int(savef, RSID_STATS);
    rs_write_str_t(savef, &s->s_str);
    rs_write_long(savef, s->s_exp);
    rs_write_int(savef, s->s_lvl);
    rs_write_int(savef, s->s_arm);
    rs_write_int(savef, s->s_hpt);
    rs_write(savef, s->s_dmg, sizeof(s->s_dmg));

    return(WRITESTAT);
}

rs_read_stats(int inf, struct stats *s)
{
    int id;

    rs_read_int(inf, &id);

    rs_read_str_t(inf,&s->s_str);
    rs_read_long(inf,&s->s_exp);
    rs_read_int(inf,&s->s_lvl);
    rs_read_int(inf,&s->s_arm);
    rs_read_int(inf,&s->s_hpt);
    rs_read(inf,s->s_dmg,sizeof(s->s_dmg));
    
    return(READSTAT);
}

rs_write_traps(FILE *savef, struct trap *trap,int count)
{
    int n;

    rs_write_int(savef, RSID_TRAP);
    rs_write_int(savef, count);
    
    for(n=0; n<count; n++)
    {
        rs_write_coord(savef, &trap[n].tr_pos);
        rs_write_char(savef, trap[n].tr_type);
        rs_write_int(savef, trap[n].tr_flags);
    }
}

rs_read_traps(int inf, struct trap *trap, int count)
{
    int id = 0, value = 0, n = 0;

    if (rs_read_int(inf,&id) != 0)
    {
        if (id != RSID_TRAP)
        {
            printf("Invalid id. %x != %x(RSID_TRAP)\n",
                id,RSID_TRAP);
            printf("Sorry, invalid save game format");
            format_error = TRUE;
        }
        else if (rs_read_int(inf,&value) != 0)
        {
            if (value > count)
            {
                printf("Incorrect number of traps in block. %d > %d.",
                    value,count);
                printf("Sorry, invalid save game format\n");
                format_error = TRUE;
            }
            else
            {
                for(n=0;n<value;n++)
                {   
                    rs_read_coord(inf,&trap[n].tr_pos);
                    rs_read_char(inf,&trap[n].tr_type);
                    rs_read_int(inf,&trap[n].tr_flags);
                }
            }
        }
        else
            format_error = TRUE;
    }
    
    return(READSTAT);
}

rs_write_monsters(FILE * savef, struct monster * m, int count)
{
    int n;
    
    rs_write_int(savef, RSID_MONSTERS);
    rs_write_int(savef, count);

    for(n=0;n<count;n++)
    {
        rs_write(savef, m[n].m_name, sizeof(m[n].m_name));
        rs_write_short(savef, m[n].m_carry);
        rs_write_short(savef, m[n].m_flags);
        rs_write_stats(savef, &m[n].m_stats);
    }
    
    return(WRITESTAT);
}

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
                rs_read(inf,m[n].m_name,sizeof(m[n].m_name));
                rs_read_short(inf, &m[n].m_carry);
                rs_read_short(inf, &m[n].m_flags);
                rs_read_stats(inf, &m[n].m_stats);
            }
        }
        else
            format_error = TRUE;
    }
    
    return(READSTAT);
}

int
find_thing_coord(monlist, c)
struct linked_list *monlist;
coord *c;
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

rs_write_thing(FILE *savef, struct thing *t)
{
    int i = -1;
    
    rs_write_int(savef, RSID_THING);
    rs_write_coord(savef, &t->t_pos);
    rs_write_boolean(savef, t->t_turn);
    rs_write_char(savef, t->t_type);
    rs_write_char(savef, t->t_disguise);
    rs_write_char(savef, t->t_oldch);

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
    
    rs_write_short(savef, t->t_flags);
    rs_write_stats(savef, &t->t_stats);
    rs_write_object_list(savef, t->t_pack);
    
    return(WRITESTAT);
}

rs_read_thing(int inf, struct thing *t)
{
    int id;
    int listid = 0, index = -1;
    struct linked_list *item;
        
    if (rs_read_int(inf, &id) != 0)
    {
        if (id != RSID_THING)
            format_error = TRUE;
        else
        {
            rs_read_coord(inf,&t->t_pos);
            rs_read_boolean(inf,&t->t_turn);
            rs_read_char(inf,&t->t_type);
            rs_read_char(inf,&t->t_disguise);
            rs_read_char(inf,&t->t_oldch);
            
            rs_read_int(inf, &listid);
            rs_read_int(inf, &index);
            t->t_reserved = -1;

            if (listid == 0)
            {
                if (index == 1)
                    t->t_dest = &hero;
                else
                    t->t_dest = NULL;
            }
            else if (listid == 1)
            {
                t->t_dest     = NULL;
                t->t_reserved = index;
            }
            else if (listid == 2)
            {
                struct object *obj;
                item = get_list_item(lvl_obj,index);
                if (item != NULL)
                {
                    obj = OBJPTR(item);
                    t->t_dest = &obj->o_pos;
                }
            }
            else if (listid == 3)
            {
                t->t_dest = &rooms[index].r_gold;
            }
            else
                t->t_dest = NULL;
            

            rs_read_short(inf,&t->t_flags);
            rs_read_stats(inf,&t->t_stats);
            rs_read_object_list(inf,&t->t_pack);
        }
    }
    else format_error = TRUE;
    
    return(READSTAT);
}

rs_fix_monster_list(list)
struct linked_list *list;
{
    struct linked_list *item;

    for(item = list; item != NULL; item = item->l_next)
        rs_fix_thing(THINGPTR(item));
}

rs_write_monster_list(FILE *savef, struct linked_list *l)
{
    int cnt = 0;
    
    rs_write_int(savef, RSID_MONSTERLIST);

    cnt = list_size(l);

    rs_write_int(savef, cnt);

    if (cnt < 1)
        return(WRITESTAT);

    while (l != NULL) {
        rs_write_thing(savef, (struct thing *)l->l_data);
        l = l->l_next;
    }
    
    return(WRITESTAT);
}

rs_read_monster_list(int inf, struct linked_list **list)
{
    int id;
    int i, cnt;
    struct linked_list *l = NULL, *previous = NULL, *head = NULL;

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
                l = new_item(sizeof(struct thing));
                l->l_prev = previous;
                if (previous != NULL)
                    previous->l_next = l;
                rs_read_thing(inf,(struct thing *)l->l_data);
                if (previous == NULL)
                    head = l;
                previous = l;
            }
        

            if (l != NULL)
                l->l_next = NULL;

            *list = head;
        }
    }
    else format_error = TRUE;
    
    return(READSTAT);
}

rs_write_magic_items(FILE *savef, struct magic_item *i, int count)
{
    int n;
    
    rs_write_int(savef, RSID_MAGICITEMS);
    rs_write_int(savef, count);

    for(n=0;n<count;n++)
    {
        rs_write(savef,i[n].mi_name,sizeof(i[n].mi_name));
        rs_write_int(savef,i[n].mi_prob);
        rs_write_int(savef,i[n].mi_worth);
    }
    
    return(WRITESTAT);
}

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
                id,RSID_MAGICITEMS);
            printf("Sorry, invalid save game format");
            format_error = TRUE;
        }   
        else if (rs_read_int(inf, &value) != 0)
        {
            if (value > count)
            {
                printf("Incorrect number of magic items in block. %d > %d.",
                    value,count);
                printf("Sorry, invalid save game format");
                format_error = TRUE;
            }
            else
            {
                for(n = 0; n < value; n++)
                {
                    rs_read(inf,mi[n].mi_name,sizeof(mi[n].mi_name));
                    rs_read_int(inf,&mi[n].mi_prob);
                    rs_read_int(inf,&mi[n].mi_worth);
                }
            }
        }
    }
    
    return(READSTAT);
}

rs_write_str_t(FILE *savef, str_t *st)
{
    rs_write_short(savef,st->st_str);
    rs_write_short(savef,st->st_add);

    return(WRITESTAT);
}

rs_read_str_t(int inf, str_t *st)
{
    rs_read_short(inf,&st->st_str);
    rs_read_short(inf,&st->st_add);
    
    return(READSTAT);
}

rs_write_object_reference(FILE *savef, struct linked_list *list, 
    struct object *item)
{
    int i;
    
    i = find_list_ptr(list, item);
    rs_write_int(savef, i);

    return(WRITESTAT);
}

rs_read_object_reference(int inf, struct linked_list *list, 
    struct object **item)
{
    int i;
    
    rs_read_int(inf, &i);
    *item = get_list_item(list,i);
            
    return(READSTAT);
}

rs_write_room_reference(FILE *savef, struct room *rp)
{
    int i, room = -1;
    
    for (i = 0; i < MAXROOMS; i++)
        if (&rooms[i] == rp)
            room = i;

    rs_write_int(savef, room);

    return(WRITESTAT);
}

rs_read_room_reference(int inf, struct room **rp)
{
    int i;
    
    rs_read_int(inf, &i);

	*rp = &rooms[i];
            
    return(READSTAT);
}

rs_save_file(FILE *savef)
{
    int endian = 0x01020304;
    big_endian = ( *((char *)&endian) == 0x01 );

    rs_write_thing(savef, &player);                     
    rs_write_object_list(savef, lvl_obj);               
    rs_write_monster_list(savef, mlist);                
    rs_write_traps(savef, traps, MAXTRAPS);             
    rs_write_rooms(savef, rooms, MAXROOMS);             
    rs_write_room_reference(savef, oldrp);              
    rs_write_stats(savef,&max_stats);                   
    rs_write_monsters(savef,monsters,26);               
    rs_write_object_reference(savef, player.t_pack, cur_weapon); 
    rs_write_object_reference(savef, player.t_pack, cur_armor);
    rs_write_object_reference(savef, player.t_pack, cur_ring[0]);
    rs_write_object_reference(savef, player.t_pack, cur_ring[1]);
    rs_write_magic_items(savef, things,   NUMTHINGS);   
    rs_write_magic_items(savef, s_magic,  MAXSCROLLS);  
    rs_write_magic_items(savef, p_magic,  MAXPOTIONS);  
    rs_write_magic_items(savef, r_magic,  MAXRINGS);    
    rs_write_magic_items(savef, ws_magic, MAXSTICKS);      
    rs_write_int(savef, level);                         
    rs_write_int(savef, purse);                         
    rs_write_int(savef, mpos);                          
    rs_write_int(savef, ntraps);                        
    rs_write_int(savef, no_move);                       
    rs_write_int(savef, no_command);                    
    rs_write_int(savef, inpack);                        
    rs_write_int(savef, max_hp);                        
    rs_write_int(savef, total);                         
    rs_write_ints(savef,a_chances,MAXARMORS);           
    rs_write_ints(savef,a_class,MAXARMORS);             
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
    rs_write_char(savef, take);
    rs_write(savef,prbuf,80);
    rs_write(savef,outbuf,BUFSIZ);
    rs_write_char(savef, runch);
    rs_write_scrolls(savef);
    rs_write_potions(savef);
    rs_write_rings(savef);
    rs_write_sticks(savef);
    rs_write_strings(savef, a_names,  MAXARMORS);       
    rs_write(savef,whoami,80);
    rs_write(savef,fruit,80);
    rs_write(savef,huh,80);
    rs_write(savef,file_name,80);
    rs_write(savef,home,80);
    rs_write_window(savef, cw);
    rs_write_window(savef, hw);
    rs_write_window(savef, mw);
    rs_write_window(savef, stdscr);
    rs_write_boolean(savef, running);                   
    rs_write_boolean(savef, playing);                   
    rs_write_boolean(savef, wizard);                    
    rs_write_boolean(savef, after);
    rs_write_boolean(savef, notify);                    
    rs_write_boolean(savef, fight_flush);               
    rs_write_boolean(savef, terse);                     
    rs_write_boolean(savef, door_stop);                 
    rs_write_boolean(savef, jump);                      
    rs_write_boolean(savef, slow_invent);               
    rs_write_boolean(savef, firstmove);                 
    rs_write_boolean(savef, waswizard);                 
    rs_write_boolean(savef, askme);                     
    rs_write_boolean(savef, amulet);                    
    rs_write_boolean(savef, in_shell);                  
    rs_write_coord(savef,&oldpos);                      
    rs_write_coord(savef,&delta);                       
    rs_write_coord(savef, &ch_ret);                     /* chase.c      */
    rs_write_char(savef,countch);                       /* command.c    */
    rs_write_char(savef,direction);                     /* command.c    */
    rs_write_char(savef,newcount);                      /* command.c    */
    rs_write_daemons(savef, &d_list[0], 20);            /* daemon.c     */
    rs_write_int(savef,between);                        /* daemons.c    */
    rs_write_words(savef,rainbow,NCOLORS);
    rs_write_words(savef,sylls,NSYLLS);
    rs_write_words(savef,stones,NSTONES);
    rs_write_words(savef,wood,NWOOD);
    rs_write_words(savef,metal,NMETAL);
    rs_write_int(savef,num_checks);                     /* main.c       */
    rs_write(savef,lvl_mons,sizeof(lvl_mons));          /* monsters.c   */
    rs_write(savef,wand_mons,sizeof(wand_mons));        /* monsters.c   */
    rs_write_coord(savef, &nh);                         /* move.c       */

    return(WRITESTAT);
}

rs_restore_file(int inf)
{
    int endian = 0x01020304;
    big_endian = ( *((char *)&endian) == 0x01 );
    
    rs_read_thing(inf, &player);                        
    rs_read_object_list(inf, &lvl_obj);                 
    rs_read_monster_list(inf, &mlist);                  
    rs_fix_thing(&player);
    rs_fix_monster_list(mlist);
    rs_read_traps(inf, traps, MAXTRAPS);
    rs_read_rooms(inf, rooms, MAXROOMS);
    rs_read_room_reference(inf, &oldrp);
    rs_read_stats(inf,&max_stats);                      
    rs_read_monsters(inf,monsters,26);                  
    rs_read_object_reference(inf, player.t_pack, &cur_weapon);
    rs_read_object_reference(inf, player.t_pack, &cur_armor);
    rs_read_object_reference(inf, player.t_pack, &cur_ring[0]);
    rs_read_object_reference(inf, player.t_pack, &cur_ring[1]);
    rs_read_magic_items(inf, things,NUMTHINGS);         
    rs_read_magic_items(inf, s_magic,MAXSCROLLS);       
    rs_read_magic_items(inf, p_magic,MAXPOTIONS);       
    rs_read_magic_items(inf, r_magic,MAXRINGS);         
    rs_read_magic_items(inf, ws_magic,MAXSTICKS);       
    rs_read_int(inf, &level);                           
    rs_read_int(inf, &purse);                           
    rs_read_int(inf, &mpos);                            
    rs_read_int(inf, &ntraps);                          
    rs_read_int(inf, &no_move);                         
    rs_read_int(inf, &no_command);                      
    rs_read_int(inf, &inpack);                          
    rs_read_int(inf, &max_hp);                          
    rs_read_int(inf, &total);                           
    rs_read_ints(inf,a_chances,MAXARMORS);              
    rs_read_ints(inf,a_class,MAXARMORS);                
    rs_read_int(inf, &lastscore);                       
    rs_read_int(inf, &no_food);                         
    rs_read_int(inf, &seed);                            
    rs_read_int(inf, &count);                           
    rs_read_int(inf, &dnum);                            
    rs_read_int(inf, &fung_hit);                        
    rs_read_int(inf, &quiet);                           
    rs_read_int(inf, &max_level);                       
    rs_read_int(inf, &food_left);                       
    rs_read_int(inf, &group);                           
    rs_read_int(inf, &hungry_state);                    
    rs_read_char(inf, &take);
    rs_read(inf,prbuf,80);
    rs_read(inf,outbuf,BUFSIZ);
    rs_read_char(inf, &runch);
    rs_read_scrolls(inf);
    rs_read_potions(inf);
    rs_read_rings(inf);
    rs_read_sticks(inf);
    rs_read_new_strings(inf, a_names,MAXARMORS);        
    rs_read(inf,whoami,80);
    rs_read(inf,fruit,80);
    rs_read(inf,huh,80);
    rs_read(inf,file_name,80);
    rs_read(inf,home,80);
    rs_read_window(inf, cw);
    rs_read_window(inf, hw);
    rs_read_window(inf, mw);
    rs_read_window(inf, stdscr);
    rs_read_boolean(inf, &running);                     
    rs_read_boolean(inf, &playing);                     
    rs_read_boolean(inf, &wizard);                      
    rs_read_boolean(inf, &after);
    rs_read_boolean(inf, &notify);                      
    rs_read_boolean(inf, &fight_flush);                 
    rs_read_boolean(inf, &terse);                       
    rs_read_boolean(inf, &door_stop);                   
    rs_read_boolean(inf, &jump);                        
    rs_read_boolean(inf, &slow_invent);                 
    rs_read_boolean(inf, &firstmove);                   
    rs_read_boolean(inf, &waswizard);                   
    rs_read_boolean(inf, &askme);                       
    rs_read_boolean(inf, &amulet);                      
    rs_read_boolean(inf, &in_shell);                    
    rs_read_coord(inf,&oldpos);                         
    rs_read_coord(inf,&delta);                          
    rs_read_coord(inf, &ch_ret);                        /* chase.c      */
    rs_read_char(inf,&countch);                         /* command.c    */
    rs_read_char(inf,&direction);                       /* command.c    */
    rs_read_char(inf,&newcount);                        /* command.c    */
    rs_read_daemons(inf, d_list, 20);                   /* daemon.c     */
    rs_read_int(inf,&between);                          /* daemons.c    */
    rs_read_words(inf,rainbow,NCOLORS);
    rs_read_words(inf,sylls,NSYLLS);
    rs_read_words(inf,stones,NSTONES);
    rs_read_words(inf,wood,NWOOD);
    rs_read_words(inf,metal,NMETAL);
    rs_read_int(inf,&num_checks);                       /* main.c       */
    rs_read(inf, lvl_mons, sizeof(lvl_mons));           /* monsters.c   */
    rs_read(inf, wand_mons, sizeof(wand_mons));         /* monsters.c   */
    rs_read_coord(inf, &nh);                            /* move.c       */
    
    return(READSTAT);
}
