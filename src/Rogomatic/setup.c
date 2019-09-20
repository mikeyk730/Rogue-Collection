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
 * setup.c:
 *
 * This is the program which forks and execs the Rogue & the Player
 */

# include <stdio.h>
# include <stdlib.h>
# include <signal.h>
#ifndef _WIN32
# include <unistd.h>
#else
#include <Windows.h>
#define R_OK 0
#define X_OK 0
#endif
# include "install.h"

# define READ    0
# define WRITE   1

/* Define the Rog-O-Matic pseudo-terminal (Concept Based) */

# define ROGUETERM "rg|rterm:am:bs:ce=^[^S:cl=^L:cm=^[a%+ %+ :co#80:li#24:so=^[D:se=^[d:pt:ta=^I:up=^[;:db:xn:"

int   frogue;
#ifndef _WIN32
int   trogue;
#else
void CreateProcessOrExit(LPSTR command, LPPROCESS_INFORMATION pi)
{
    STARTUPINFO si;
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);

    ZeroMemory(pi, sizeof(*pi));

    if(!CreateProcess(
        NULL,
        command,
        NULL,
        NULL,
        FALSE,
        0,
        "\0",//TODO: manage env
        NULL,
        &si,
        pi))
    {
        printf("Could not launch Rogue");
        exit(1);
    }
}

void CloseHandles(PROCESS_INFORMATION pi)
{
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
}
#endif

main (argc, argv)
int   argc;
char *argv[];

{
  int   ptc[2], ctp[2];
  int   child, score = 0, oldgame = 0;
  int   cheat = 0, noterm = 1, echo = 0, nohalf = 0, replay = 0;
  int   emacs = 0, rf = 0, terse = 0, user = 0, quitat = 2147483647;
  char  *rfile = "", *rfilearg = "", options[32];
  char  ropts[128], roguename[128];

  while (--argc > 0 && (*++argv)[0] == '-') {
    while (*++(*argv)) {
      switch (**argv) {
        case 'c': cheat++;        break; /* Will use trap arrows! */
        case 'e': echo++;         break; /* Echo file to roguelog */
        case 'f': rf++;           break; /* Next arg is the rogue file */
        case 'h': nohalf++;       break; /* No halftime show */
        case 'p': replay++;       break; /* Play back roguelog */
        case 'r': oldgame++;      break; /* Use saved game */
        case 's': score++;        break; /* Give scores only */
        case 't': terse++;        break; /* Give status lines only */
        case 'u': user++;         break; /* Start up in user mode */
        case 'w': noterm = 0;     break; /* Watched mode */
        case 'E': emacs++;        break; /* Emacs mode */
        default:  printf
          ("Usage: rogomatic [-cefhprstuwE] or rogomatic [file]\n");
          exit (1);
      }
    }

    if (rf) {
      if (--argc) rfilearg = *++argv;

      rf = 0;
    }
  }

  if (argc > 1) {
    printf ("Usage: rogomatic [-cefhprstuwE] or rogomatic <file>\n");
    exit (1);
  }

#ifndef _WIN32
  /* Find which rogue to use */
  if (*rfilearg) {
    if (access (rfilearg, R_OK|X_OK) == 0)	rfile = rfilearg;
    else				{ perror (rfilearg); exit (1); }
  }
  else if (access ("rogue", R_OK|X_OK) == 0)	rfile = "rogue";

# ifdef NEWROGUE
  else if (access (NEWROGUE, R_OK|X_OK) == 0)	rfile = NEWROGUE;

# endif
# ifdef ROGUE
  else if (access (ROGUE, R_OK|X_OK) == 0)	rfile = ROGUE;

# endif
  else {
    perror ("rogue");
    exit (1);
  }
#endif

  if (!replay && !score) quitat = findscore (rfile, "Rog-O-Matic");

  sprintf (options, "%d,%d,%d,%d,%d,%d,%d,%d",
           cheat, noterm, echo, nohalf, emacs, terse, user,quitat);
  sprintf (roguename, "Rog-O-Matic %s for %s", RGMVER, getname ());
  sprintf (ropts, "name=%s,%s,%s,%s,%s,%s,%s,%s,%s,%s",
           roguename, "fruit=apricot", "terse", "noflush", "noask",
           "jump", "step", "nopassgo", "inven=slow", "seefloor");

  if (score)  { dumpscore (argc==1 ? argv[0] : DEFVER); exit (0); }

  if (replay) { replaylog (argc==1 ? argv[0] : ROGUELOG, options); exit (0); }

#ifdef _WIN32
  PROCESS_INFORMATION rogueinfo;
  PROCESS_INFORMATION playerinfo;

  CreateProcessOrExit("RogueCollection.exe e --piped-input --piped-output", &rogueinfo);
  CreateProcessOrExit("RogueCollection.exe g", &playerinfo);

  WaitForSingleObject(rogueinfo.hProcess, INFINITE);
  WaitForSingleObject(playerinfo.hProcess, INFINITE);

  CloseHandles(rogueinfo);
  CloseHandles(playerinfo);
#else
  if ((pipe (ptc) < 0) || (pipe (ctp) < 0)) {
    fprintf (stderr, "Cannot get pipes!\n");
    exit (1);
  }

  trogue = ptc[WRITE];
  frogue = ctp[READ];

  if ((child = fork ()) == 0) {
    close (0);
    dup (ptc[READ]);
    close (1);
    dup (ctp[WRITE]);

    if (md_setenv ("TERM", "vt100", 1) != 0) {
      fprintf (stderr, "can't setenv (\"TERM\", \"vt100\", 1)\n");
    }

    if (md_setenv ("ROGUEOPTS", ropts, 1) != 0) {
      fprintf (stderr, "can't setenv (\"ROGUEOPTS\", \"%s\", 1)\n",ropts);
    }

    if (oldgame)  execl (rfile, rfile, "-r", 0);

    if (argc)     execl (rfile, rfile, argv[0], 0);

    execl (rfile, rfile, 0);
    _exit (1);
  }

  else {
    /* Encode the open files into a two character string */
    char ft[3];
    char rp[32];

    ft[0] = 'a' + frogue;
    ft[1] = 'a' + trogue;
    ft[2] = '\0';

    /* Pass the process ID of the Rogue process as an ASCII string */
    sprintf (rp, "%d", child);

    if (!author ()) nice (4);

    execl ("player", "player", ft, rp, options, roguename, 0);
# ifdef PLAYER
    execl (PLAYER, "player", ft, rp, options, roguename, 0);
# endif
    printf ("Rogomatic not available, 'player' binary missing.\n");
    kill (child, SIGKILL);
  }
#endif
}

/*
 * replaylog: Given a log file name and an options string, exec the player
 * process to replay the game.  No Rogue process is needed (since we are
 * replaying an old game), so the frogue and trogue file descrptiors are
 * given the fake value 'Z'.
 */

replaylog (fname, options)
char *fname, *options;
{
  execl ("player", "player", "ZZ", "0", options, fname, 0);
# ifdef PLAYER
  execl (PLAYER, "player", "ZZ", "0", options, fname, 0);
# endif
  printf ("Replay not available, 'player' binary missing.\n");
  exit (1);
}

/*
 * author:
 *	See if a user is an author of the program
 */

author()
{
  switch (md_getuid()) {
    case 1337:	/* Fuzzy */
    case 1313:	/* Guy */
    case 1241:	/* Andrew */
    case 345:	/* Leonard */
    case 342:	/* Gordon */
      return 1;
    default:	return 0;
  }
}
