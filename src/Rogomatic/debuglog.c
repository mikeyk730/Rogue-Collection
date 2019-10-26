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

#include <stdarg.h>
#include <stdio.h>
#include "types.h"
#include "globals.h"

#define MAXLINE 4096

FILE *debug = NULL;

static void
err_doit(int errnoflag, int error, const char *fmt, va_list ap)
{
  char buf[MAXLINE];

  vsnprintf(buf, MAXLINE, fmt, ap);

  if (debug != NULL) {
    fputs(buf, debug);
    fflush (debug);
  }
  else {
    fputs (buf, stderr);
    fflush (stderr);
  }
}

void debuglog_open (const char *log)
{
  if (g_debug)
    debug = fopen (log, "w");
}

void debuglog_close (void)
{
  if (debug != NULL)
    fclose (debug);
}

void debuglog (const char *fmt, ...)
{
  va_list  ap;

  va_start (ap, fmt);
  err_doit (0, 0, fmt, ap);
  va_end (ap);
}
