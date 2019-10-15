/*
 * Rog-O-Matic
 * Automatically exploring the dungeons of doom.
 *
 * Copyright (C) 2008 by Anthony Molinaro
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

# include <sys/types.h>
#ifndef _WIN32
# include <dirent.h>
#endif
# include <stdlib.h>

# include "install.h"

/* install.h has a RGMDIR and a LOCKFILE which are set to the install path
 * which is fine, unless you are testing things out when you might want a
 * local log directory.  So if there is an rlog directory in your current
 * working directory, it will be given preference over the variables
 * RGMDIR and LOCKFILE
 */
static const char *rgmpath  = "rlog";
static const char *lockpath = "rlog/RgmLock";

const char *
getRgmDir ()
{
#ifdef ROGUE_COLLECTION
  return rgmpath;
#else
  DIR *rgmdir = NULL;

  /* give preference to a directory in the current directory */
  if ((rgmdir = opendir (rgmpath)) != NULL) {
    closedir (rgmdir);
    return rgmpath;
  }
  else {
    return RGMDIR;
  }
#endif
}

const char *
getLockFile ()
{
#ifdef ROGUE_COLLECTION
  return lockpath;
#else
  DIR *rgmdir = NULL;

  /* give preference to a directory in the current directory */
  if ((rgmdir = opendir (rgmpath)) != NULL) {
    closedir (rgmdir);
    return lockpath;
  }
  else {
    return LOCKFILE;
  }
#endif
}
