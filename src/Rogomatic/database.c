/*
 * Rog-O-Matic
 * Automatically exploring the dungeons of doom.
 *
 * Copyright (C) 2008 by Anthony Molinaro
 * Copyright (C) 1985 by Appel, Jacobson, Hamey, and Mauldin.
 *
 * This file is part of Rog-O-Matic.
 *
 * Rog-O-Matic is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Rog-O-Matic is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Rog-O-Matic.  If not, see <http://www.gnu.org/licenses/>.
 */

/*
 * database.c:
 *
 * This file contains the code which handles the database of objects which
 * have been used, and what the real names of the items are.
 *
 * Note: pack_index was first used to keep certain errors from happening.
 *       various fixes seem to have worked so it is no longer needed, but
 *       it is nice to have when dumping the table...  with that said,
 *       we do not bother to keep the pack_index reference back to the
 *       inventory accurate later on.  only new items get it set.
 *
 */

# include <curses.h>
# include <string.h>
# include "types.h"
# include "globals.h"

# define TABLESIZE 101
# define NOTFOUND  (-1)

struct  {
  int   used;
  int   pack_index;
  stuff item_type;
  char  fakename[NAMSIZ];
  char  realname[NAMSIZ];
} dbase[TABLESIZE];

int datalen = 0;

/*
 * findfake: find the fakename database entry for 'string'
 *           and of item_type (both must match exactly).
 */

findfake (string, item_type)
char  *string;
stuff item_type;
{
  register int i;

  for (i = 0; i < datalen; i++)
    if (streq (dbase[i].fakename, string) &&
       (dbase[i].item_type == item_type))
      return (i);
    else {
      if (dbase[i].pack_index != -1) {
        if (inven[dbase[i].pack_index].type != dbase[i].item_type)
          dbase[i].pack_index = -1;
        else if (!(streq (inven[dbase[i].pack_index].str, dbase[i].fakename) ||
          streq (inven[dbase[i].pack_index].str, dbase[i].realname)))
          dbase[i].pack_index = -1;
      }
    }

  return (NOTFOUND);
}

/*
 * findentry: find the database entry for 'string'
 */

findentry (string)
char *string;
{
  register int i;

  for (i = 0; i < datalen; i++)
    if (streq (dbase[i].fakename, string) ||
        *dbase[i].realname && streq (dbase[i].realname, string))
      return (i);

  return (NOTFOUND);
}

/*
 * findentry_getfakename: find the database entry for 'string' and item_type,
 *     item_type must match, but the string can match either the fakename or
 *     the realname.  returns pointer to the fakename or "".
 */

char *findentry_getfakename (string, item_type)
char  *string;
stuff item_type;
{
  register int i;

  for (i = 0; i < datalen; i++)
    if ((dbase[i].item_type == item_type) &&
       (streq (dbase[i].fakename, string) ||
        *dbase[i].realname && streq (dbase[i].realname, string)))
      return (dbase[i].fakename);

  return ("");
}

/*
 * findentry_getrealname: find the database entry for 'string' and item_type,
 *     item_type must match, but the string can match either the fakename or
 *     the realname.  returns pointer to the realname or "".
 */

char *findentry_getrealname (string, item_type)
char  *string;
stuff item_type;
{
  register int i;

  for (i = 0; i < datalen; i++)
    if ((dbase[i].item_type == item_type) &&
       (streq (dbase[i].fakename, string) ||
        *dbase[i].realname && streq (dbase[i].realname, string)))
      return (dbase[i].realname);

  return ("");
}

/*
 * addobj: Add item to dbase.
 */

void addobj (codename, pack_index, item_type)
char  *codename;
int   pack_index;
stuff item_type;
{
  if (findfake (codename, item_type) == NOTFOUND) {
    dbase[datalen].pack_index = pack_index;
    dbase[datalen].item_type = item_type;
    memset (dbase[datalen].fakename, '\0', NAMSIZ);
    strncpy (dbase[datalen].fakename, codename, NAMSIZ-1);
    memset (dbase[datalen].realname, '\0', NAMSIZ);
    datalen++;
  }
}

/*
 * useobj: Indicate that we have used (i.e. read, quaffed, or zapped) an
 *         object with name 'string'.
 */

void useobj (string)
char *string;
{
  int i = findentry (string);

  if (i != NOTFOUND) {
    dbase[i].used = 1;
  }
}

/*
 * infername: Note that we now think that the object named 'codename' is
 * really named 'name' (e.g. scroll 'google plex' is really a scroll of
 * light).
 */

void infername (codename, name, item_type)
char  *codename;
char  *name;
stuff item_type;
{
  debuglog("infer: '%s' is '%s' (%s)\n", codename, name, get_item_type_string(item_type));

  register int i;

  i = findfake (codename, item_type);

  if (i == NOTFOUND) {
    dbase[datalen].item_type = item_type;
    memset (dbase[datalen].fakename, '\0', NAMSIZ);
    strncpy (dbase[datalen].fakename, codename, NAMSIZ-1);
    memset (dbase[datalen].realname, '\0', NAMSIZ);
    strncpy (dbase[datalen].realname, name, NAMSIZ-1);
    datalen++;
  }
  else {
    if (*dbase[i].realname && strcmp (dbase[i].realname, name))
      dwait (D_ERROR, "Inconsistent inference, infername: dbase[i].realname '%s', name '%s'",
             dbase[i].realname, name);
    else {
      memset (dbase[i].realname, '\0', NAMSIZ);
      strncpy (dbase[i].realname, name, NAMSIZ-1);
    }
  }
}

/*
 * used: Return true if we have marked 'codename' as used.
 */

int used (codename)
char *codename;
{
  register int i;

  for (i = 0; i < datalen; i++)
    if (streq (dbase[i].fakename, codename))
      return (dbase[i].used);

  dwait (D_PACK, "used: unknown codename '%s'", codename); /* mdk: added dwait */
  return 0;
}

/*
 * know: Return true if we know what the fake name for 'name' is.
 */

int know (name)
char *name;
{
  register int i;

  for (i = 0; i < datalen; i++)
    if (*dbase[i].realname && streq (dbase[i].realname, name))
      return (1);

  return (0);
}

/*
 * realname: Returns the real name of an object named 'codename'.
 */

char *realname (codename)
char *codename;
{
  register int i;

  for (i = 0; i < datalen; i++)
    if (*dbase[i].realname && streq (dbase[i].fakename, codename))
      return (dbase[i].realname);

  return ("");
}

/*
 * dumpdatabase: Debugging, dump the database on the screen.
 */

void dumpdatabase ()
{
  register int i;

  for (i = 0; i < datalen; i++) {
    at (i+1, 0);
    printw ("%02d %c|%01d|%01d %-32s %02d '%s'",
      i, (dbase[i].pack_index != -1) ? LETTER(dbase[i].pack_index) : ' ', dbase[i].item_type, dbase[i].used,
      dbase[i].realname, i, dbase[i].fakename);
  }
}
