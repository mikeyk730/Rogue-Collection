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

# include <stdio.h>

# include "types.h"
# include "globals.h"
# include "termtokens.h"

# define GETLOGCHAR	fgetc(logfile)
# define ISPRT(c)	((c) >= ' ' && (c) <= '~')

int number1 = 0;
int number2 = 0;
int number3 = 0;

/* Last kind of message to echo file */
static int cecho = 0;

/* Game record file 'echo' option */
static FILE  *fecho=NULL;

int rogue_log_open (const char *filename)
{
  fecho = fopen (filename, "w");

  if (fecho != NULL) {
    fprintf (fecho, "Rogomatic Game Log\n\n"); fflush (fecho);
    cecho = 1;
  }

  return (fecho != NULL);
}

void rogue_log_close ()
{
  if (cecho)
    fprintf (fecho, "\n");
  else
    fprintf (fecho, "\"\n");

  fflush (fecho);
  fclose (fecho);
}

void rogue_log_write_command (char c)
{
  if (logging) {
    if (cecho) {
      fprintf (fecho, "\nC: \"%c", c);
      cecho = !cecho;
    }
    else {
      fprintf (fecho, "%c", c);
    }

    fflush (fecho);
  }
}

void rogue_log_write_token (char ch)
{
  /* Log the tokens */
  if (logging) {
    if (!cecho) {
      fprintf (fecho, "\"\nR: ");
      cecho = !cecho;
    }

    if (ISPRT (ch))
      fprintf (fecho, "%c", ch);
    else {
      switch (ch) {
        case BS_TOK: fprintf (fecho, "{bs}");                   break;
        case CE_TOK: fprintf (fecho, "{ce}");                   break;
        case CL_TOK: fprintf (fecho, "{ff}");                   break;
        case CM_TOK: fprintf (fecho, "{cm(%d,%d)}", number1, number2);  break;
        case CR_TOK: fprintf (fecho, "{cr}");                   break;
        case ER_TOK: fprintf (fecho, "{ERRESC}", ch);           break;
        case LF_TOK: fprintf (fecho, "{lf}");                   break;
        case ND_TOK: fprintf (fecho, "{nd(%d)}", number1);      break;
        case SE_TOK: fprintf (fecho, "{se}");                   break;
        case SO_TOK: fprintf (fecho, "{so}");                   break;
        case TA_TOK: fprintf (fecho, "{ta}");                   break;
        case UP_TOK: fprintf (fecho, "{up}");                   break;
        case HM_TOK: fprintf (fecho, "{hm}");                   break;
        case CH_TOK: fprintf (fecho, "{ch(%d,%d)}", number1, number2); break;
        case NU_TOK: fprintf (fecho, "{nu(%d)}", number1);      break;
        case NR_TOK: fprintf (fecho, "{nr(%d)}", number1);      break;
        case NL_TOK: fprintf (fecho, "{nl(%d)}", number1);      break;
        case SC_TOK: fprintf (fecho, "{sc}");                   break;
        case RC_TOK: fprintf (fecho, "{rc}");                   break;
        case SR_TOK: fprintf (fecho, "{sr}");                   break;
        case CB_TOK: fprintf (fecho, "{cb}");                   break;
        default:     fprintf (fecho, "{ERR%o}", ch);
          ch = ER_TOK;
      }
    }

    fflush (fecho);
  }
}


/* Debuglog for the frogue */
static FILE *froguelog = NULL;

void open_frogue_debuglog (const char *file)
{
  froguelog = fopen (file,"w");
}

void open_frogue_fd_debuglog (int frogue_fd_dl)
{
  froguelog = fdopen (frogue_fd_dl,"w");
}

#define PUTDEBUGCHAR(c) {if (froguelog != NULL) {fputc(c,froguelog); fflush (froguelog);}}

void close_frogue_debuglog ()
{
  if (froguelog != NULL)
    fclose (froguelog);
}


/* Log from rogue */
static FILE *frogue = NULL;

void open_frogue (const char *file)
{
  frogue = fopen (file, "r");
}

void open_frogue_fd (int frogue_fd)
{
  frogue = fdopen (frogue_fd, "r");
}

#define GETROGUECHAR fgetc(frogue);
#define UNGETROGUECHAR(c) ungetc(c, frogue);

void close_frogue ()
{
  fclose (frogue);
}

static int matchnum (char ch)
{
  switch (ch) {
    case '0':
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
    case '9':
      return 1;
      break;
  }

  return 0;
}

static int fetchnum (char ch)
{
  char ch2;
  char num[20];
  int ind = 1;
  int done = 0;

  num[0] = ch;

  while (! done) {
    int ch2 = GETROGUECHAR;

    if (matchnum (ch2)) {
      num[ind] = ch2;
      ++ind;
      PUTDEBUGCHAR (ch2);
    }
    else {
      UNGETROGUECHAR (ch2);
      done = 1;
    }
  }

  num[ind] = '\0';

  return atoi (num);
}

static int match2 (char ch1, char ch2)
{
  char mch1 = GETROGUECHAR;
  char mch2 = GETROGUECHAR;

  if (ch1 == mch1 && ch2 == mch2) {
    PUTDEBUGCHAR (mch1);
    PUTDEBUGCHAR (mch2);
    return 1;
  }
  else {
    UNGETROGUECHAR (mch2);
    UNGETROGUECHAR (mch1);
    return 0;
  }
}

static int match3 (char ch1, char ch2, char ch3)
{
  char mch1 = GETROGUECHAR;
  char mch2 = GETROGUECHAR;
  char mch3 = GETROGUECHAR;

  if (ch1 == mch1 && ch2 == mch2 && ch3 == mch3) {
    PUTDEBUGCHAR (mch1);
    PUTDEBUGCHAR (mch2);
    PUTDEBUGCHAR (mch3);
    return 1;
  }
  else {
    UNGETROGUECHAR (mch3);
    UNGETROGUECHAR (mch2);
    UNGETROGUECHAR (mch1);
    return 0;
  }
}

static int match4 (char ch1, char ch2, char ch3, char ch4)
{
  char mch1 = GETROGUECHAR;
  char mch2 = GETROGUECHAR;
  char mch3 = GETROGUECHAR;
  char mch4 = GETROGUECHAR;

  if (ch1 == mch1 && ch2 == mch2 && ch3 == mch3 && ch4 == mch4) {
    PUTDEBUGCHAR (mch1);
    PUTDEBUGCHAR (mch2);
    PUTDEBUGCHAR (mch3);
    PUTDEBUGCHAR (mch4);
    return 1;
  }
  else {
    UNGETROGUECHAR (mch4);
    UNGETROGUECHAR (mch3);
    UNGETROGUECHAR (mch2);
    UNGETROGUECHAR (mch1);
    return 0;
  }
}

static int match5 (char ch1, char ch2, char ch3, char ch4, char ch5)
{
  char mch1 = GETROGUECHAR;
  char mch2 = GETROGUECHAR;
  char mch3 = GETROGUECHAR;
  char mch4 = GETROGUECHAR;
  char mch5 = GETROGUECHAR;

  if (ch1 == mch1 && ch2 == mch2 && ch3 == mch3 && ch4 == mch4 && ch5 == mch5) {
    PUTDEBUGCHAR (mch1);
    PUTDEBUGCHAR (mch2);
    PUTDEBUGCHAR (mch3);
    PUTDEBUGCHAR (mch4);
    PUTDEBUGCHAR (mch5);
    return 1;
  }
  else {
    UNGETROGUECHAR (mch5);
    UNGETROGUECHAR (mch4);
    UNGETROGUECHAR (mch3);
    UNGETROGUECHAR (mch2);
    UNGETROGUECHAR (mch1);
    return 0;
  }
}



/*
   Cap-   TCap    Variable              Description
   name   Code
   am     am      auto_right_margin     terminal has automatic margins
   ???    bs      ???                   ???
   el     ce      clr_eol               clear to end of line
   clear  cl      clear_screen          clear screen and home cursor
   cup    cm      cursor_address        move to row #1 columns #2
   cols   co      columns               number of columns in a line
   lines  li      lines                 number of lines on screen or page
   smso   so      enter_standout_mode   begin standout mode
   rmso   se      exit_standout_mode    exit standout mode
   ???    pt      ???                   ???
   ht     ta      tab                   tab to next 8-space hardware tab stop
   cuu1   up      cursor_up             up one line
   db     db      memory_below          display may be retained below the screen
   xenl   xn      eat_newline_glitch    newline ignored after 80 cols (concept)
 */

/*
   Cap-   TCap    ROGUETERM  VT100
   name   Code
   am     am                                   terminal has automatic margins
   cols   co      80         80                number of columns in a line
   lines  li      24         24                number of lines on screen or page
   kbs    bs      ^H         ^H         BS_TOK backspace
   ht     ta      ^I         ^I         TA_TOK tab to next hw stop
   cr     cr      ^M         ^M         CR_TOK carriage return
   cud    cud1    ^J         ^J         LF_TOK line feed
   rmacs                     ^O                end alternate char set
   cuu1   up      ^[;        ^[[A       UP_TOK up one line
   home                      ^[[H       HM_TOK home cursor (if no cup)
   clear  cl      ^L         ^[[H^[[J   CL_TOK clear screen and home cursor
   el     ce      ^[^S       ^[[K       CE_TOK clear to end of line
   rmso   se      ^[d        ^[[m       SE_TOK exit standout mode
   smam                      ^[[?7h     AU_TOK turn on automatic margins
   smkx                      ^[[?1h^[=         enter 'keyboard_transmit' mode
   rmkx                      ^[[?1l^[>         leave 'keyboard_transmit' mode
   smso   so      ^[D        ^[[7m      SO_TOK begin standout mode
   cuf                       ^[[#C      RT_TOK move #1 characters to the right
   cub                       ^[[#D      LT_TOK move #1 characters to the left
   cup    cm      ^[a# #     ^[[#;#H    CM_TOK move to row #1 columns #2
   csr                       ^[[#;#r    CH_TOK change region to line #1 to line #2
   enacs                     ^[(B^[)0          enable alternate char set

   db     db
   xenl   xn
   ???    pt

 */

/*
 * getroguetoken: get a command from Rogue (either a character or a
 * cursor motion sequence).
 */
char
getroguetoken (void)
{
  char ch, ch2;

  if (replaying)
    return (getlogtoken());

  ch = GETROGUECHAR; PUTDEBUGCHAR (ch);

  if (ch == ESC) {
    /* ^[ */
    ch = GETROGUECHAR; PUTDEBUGCHAR (ch);

    switch (ch) {
        /* ^[ */
      case '[':
        ch2 = GETROGUECHAR; PUTDEBUGCHAR (ch2);

        switch (ch2) {
            /* ^[[A    UP_TOK */
          case 'A':
            ch = UP_TOK;
            break;
            /* ^[[H        HM_TOK */
            /* ^[[H^[[J    CL_TOK */
          case 'H': {
            if (match3 (ESC, '[', 'J')) {
              ch = CL_TOK;
            }
            else {
              ch = HM_TOK;
            }
          }
          break;
          /* ^[[K        CE_TOK */
          case 'K':
            ch = CE_TOK;
            break;
            /* ^[[m        SE_TOK */
          case 'm':
            ch = SE_TOK;
            break;
            /* ^[[?7h            turn on automatic margins       */
            /* ^[[?1h^[=         enter 'keyboard_transmit' mode  */
            /* ^[[?1l^[>         leave 'keyboard_transmit' mode  */
          case '?': {
            if (match2 ('7', 'h')) {
              ch = ER_TOK;
            }
            else if (match4 ('1', 'h', ESC, '=')) {
              ch = ER_TOK;
            }
            else if (match4 ('1', 'l', ESC, '>')) {
              ch = ER_TOK;
            }
            else {
              ch = GETROGUECHAR; PUTDEBUGCHAR (ch);
              debuglog ("UNRECOGNIZED 1 : ^[[?%c\n",ch);
            }
          }
          break;
          /* ^[[#A             up #1 lines                         */
          /* ^[[#B             down #1 lines                       */
          /* ^[[#C      RT_TOK move #1 characters to the right     */
          /* ^[[#D      LT_TOK move #1 characters to the left      */
          /* ^[[1K      CB_TOK Clear to beginning of line          */
          /* ^[[7m      SO_TOK begin standout mode */
          /* ^[[#;#H    CM_TOK move to row #1 columns #2           */
          /* ^[[#;#r    CH_TOK change region to line #1 to line #2 */
          default:

            if (matchnum (ch2)) {
              number1 = fetchnum (ch2);
              char nch = GETROGUECHAR; PUTDEBUGCHAR (nch);

              switch (nch) {
                case 'm':

                  if (number1 == 7) {
                    /* Start standout mode */
                    ch = SO_TOK;
                  }
                  else {
                    ch = ER_TOK;
                  }

                  break;

                case 'A':
                  /* move up by number1 */
                  ch = NU_TOK;
                  break;
                case 'B':
                  /* move down by number1 */
                  ch = ND_TOK;
                  break;
                case 'C':
                  /* move right by number1 */
                  ch = NR_TOK;
                  break;
                case 'D':
                  /* move left by number1 */
                  ch = NL_TOK;
                  break;
                case 'K':

                  if (number1 == 1) {
                    /* Clear to beginning of line */
                    ch = CB_TOK;
                  }
                  else {
                    ch = ER_TOK;
                  }

                  break;
                case ';': {
                  char ch3 = GETROGUECHAR; PUTDEBUGCHAR (ch3);

                  if (matchnum (ch3)) {
                    number2 = fetchnum (ch3);
                    char nch2 = GETROGUECHAR; PUTDEBUGCHAR (nch2);

                    switch (nch2) {
                      case 'H':
                        /* move to position number1, number2 */
                        ch = CM_TOK;
                        break;
                      case 'r':
                        /* change scroll region */
                        ch = CH_TOK;
                        break;
                      case ';': {
                        char ch4 = GETROGUECHAR;
                        PUTDEBUGCHAR (ch4);

                        if (matchnum (ch4)) {
                          number3 = fetchnum (ch4);
                          char ch5 = GETROGUECHAR;
                          PUTDEBUGCHAR (ch5);

                          switch (ch5) {
                            case 'm': {
                              char ch6 = GETROGUECHAR;
                              PUTDEBUGCHAR (ch6);
                              ch = ER_TOK;
                            }
                            break;
                            default:
                              debuglog ("UNRECOGNIZED 7\n");
                              break;
                          }
                        }
                      }
                      break;
                      default:
                        debuglog ("UNRECOGNIZED 2\n");
                        break;
                    }
                  }
                }
                break;
                default:
                  debuglog ("UNRECOGNIZED 3\n");
                  break;
              }
            }
            else {
              debuglog ("UNRECOGNIZED 4\n");
            }
        }

        break;
      case '(':

        if (match4 ('B', ESC, ')', '0')) {
          ch = ER_TOK;
        }
        else {
          debuglog ("UNRECOGNIZED 5\n");
        }

        break;
        /* ^[7 save cursor */
      case '7':
        ch = SC_TOK;
        break;
        /* ^[8 restore cursor */
      case '8':
        ch = RC_TOK;
        break;
        /* ^[M scroll_reverse */
      case 'M':
        ch = SR_TOK;
        break;

      default:
        debuglog ("UNRECOGNIZED 6 ^[%c\n",ch);
    }
  }
  else if (ch == ctrl('O')) {
    ch = ER_TOK;
  }
  else if (ch == ctrl('H')) {
    ch = BS_TOK;
  }
  else if (ch == ctrl('J')) {
    ch = LF_TOK;
  }
  else if (ch == ctrl('M')) {
    ch = CR_TOK;
  }
  else if (ch == ctrl('I')) {
    ch = TA_TOK;
  }

  rogue_log_write_token (ch);

  return (ch);
}

/*
 * getoldcommand: retrieve the old command from a logfile we are replaying.
 */

getoldcommand (s)
register char *s;
{
  register int charcount = 0;
  char ch = ' ', term = '"', *startpat = "\nC: ";

  while (*startpat && (int) ch != EOF)
    { if ((ch = GETLOGCHAR) != *(startpat++)) startpat = "\nC: "; }

  if ((int) ch != EOF) {
    term = ch = GETLOGCHAR;

    while ((ch = GETLOGCHAR) != term && (int) ch != EOF && charcount++ < 128) {
      *(s++) = ch;
    }
  }

  *s = '\0';
}

/*
 * getlogtoken: routine to retrieve a rogue token from the log file.
 * This allows us to replay a game with all the diagnostic commands of
 * Rog-O-Matic at our disposal.					LGCH.
 */

int getlogtoken()
{
  int acceptline;
  int ch = GETLOGCHAR;
  int ch1, ch2, dig;

  while (ch == NEWLINE) {
    acceptline = 0;

    if ((ch = GETLOGCHAR) == 'R')
      if ((ch = GETLOGCHAR) == ':')
        if ((ch = GETLOGCHAR) == ' ') {
          ch = GETLOGCHAR;
          acceptline = 1;
        }

    if (!acceptline)
      while ((int) ch != NEWLINE && (int) ch != EOF)
        ch = GETLOGCHAR;
  }

  if (ch == '{') {
    ch1 = GETLOGCHAR;
    ch2 = GETLOGCHAR;
    ch = GETLOGCHAR;   /* Ignore the closing '}' */

    switch (ch1) {
      case 'b': ch = BS_TOK; break;
      case 'c':

        switch (ch2) {
          case 'e': ch = CE_TOK; break;
          case 'm':
            ch = CM_TOK;
            number1 = 0;

            while ((dig = GETLOGCHAR) != ',') {
              number1 = number1 * 10 + dig - '0';
            }

            number2 = 0;

            while ((dig = GETLOGCHAR) != ')')
              { number2 = number2 * 10 + dig - '0'; }

            GETLOGCHAR;		/* Ignore '}' */
            break;
          case 'r': ch = CR_TOK; break;
          case 'h':
            ch = CH_TOK;
            number1 = 0;

            while ((dig = GETLOGCHAR) != ',') {
              number1 = number1 * 10 + dig - '0';
            }

            number2 = 0;

            while ((dig = GETLOGCHAR) != ')')
              { number2 = number2 * 10 + dig - '0'; }

            GETLOGCHAR;		/* Ignore '}' */
            break;
          case 'b': ch = CB_TOK; break;
        }

        break;
      case 'f': ch = CL_TOK; break;
      case 'h': ch = HM_TOK; break;
      case 'l': ch = LF_TOK; break;
      case 'n':

        switch (ch2) {
          case 'd':
            ch = ND_TOK;
            number1 = 0;

            while ((dig = GETLOGCHAR) != ')') {
              number1 = number1 * 10 + dig - '0';
            }

            GETLOGCHAR;		/* Ignore '}' */
            break;
          case 'u':
            ch = NU_TOK;
            number1 = 0;

            while ((dig = GETLOGCHAR) != ')') {
              number1 = number1 * 10 + dig - '0';
            }

            GETLOGCHAR;		/* Ignore '}' */
            break;
          case 'r':
            ch = NR_TOK;
            number1 = 0;

            while ((dig = GETLOGCHAR) != ')') {
              number1 = number1 * 10 + dig - '0';
            }

            GETLOGCHAR;		/* Ignore '}' */
            break;
          case 'l':
            ch = NL_TOK;
            number1 = 0;

            while ((dig = GETLOGCHAR) != ')') {
              number1 = number1 * 10 + dig - '0';
            }

            GETLOGCHAR;		/* Ignore '}' */
            break;
        }

        break;
      case 's':

        switch (ch2) {
          case 'e': ch = SE_TOK; break;
          case 'o': ch = SO_TOK; break;
          case 'c': ch = SC_TOK; break;
          case 'r': ch = SR_TOK; break;
        }

        break;
      case 't': ch = TA_TOK; break;
      case 'u': ch = UP_TOK; break;
      case 'E':

        while (GETLOGCHAR != '}')
          ;

        ch = ER_TOK;
        break;
    }
  }

  return (ch);
}
