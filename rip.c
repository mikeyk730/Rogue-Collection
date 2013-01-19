//File for the fun ends. Death or a total win
//rip.c        1.4 (A.I. Design)       12/14/84

#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <io.h>
#include <sys/stat.h>

#include "rogue.h"
#include "rip.h"
#include "curses.h"
#include "io.h"
#include "mach_dep.h"
#include "things.h"
#include "misc.h"

static int sc_fd;

//external curses IBM (jll)

//score: Figure score and post it.

void score(int amount, int flags, char monst)
{
  struct sc_ent his_score, top_ten[TOPSCORES];
  int rank = 0;
  char response = ' ';

  if (amount || flags || monst)
  {
    move(LINES-1, 0);
    cursor(TRUE);
    printw("[Press Enter to see rankings]");
    flush_type();
    wait_for('\r');
    move(LINES-1, 0);
  }
  while ((sc_fd = _open(s_score, 0))<0)
  {
    printw("\n");
    if (noscore || (amount==0)) return;
    str_attr("No scorefile: %Create %Retry %Abort");
reread:
    switch (response = readchar())
    {
      case 'c': case 'C': _close(_creat(s_score, _S_IREAD | _S_IWRITE));
      case 'r': case 'R': break;
      case 'a': case 'A': return;
      default: goto reread;
    }
  }
  printw("\n");
  get_scores(&top_ten[0]);
  if (noscore!=TRUE)
  {
    strcpy(his_score.sc_name, whoami);
    his_score.sc_gold = amount;
    his_score.sc_fate = flags?flags:monst;
    his_score.sc_level = max_level;
    his_score.sc_rank = pstats.s_lvl;
    rank = add_scores(&his_score, &top_ten[0]);
  }
  _close(sc_fd);
  if (rank>0)
  {
    if ((sc_fd = _creat(s_score, _S_IREAD | _S_IWRITE))>=0) {put_scores(&top_ten[0]); _close(sc_fd);}
  }
  pr_scores(rank, &top_ten[0]);

}

void get_scores(struct sc_ent *top10)
{
  int i, retcode = 1;

  for (i = 0; i<TOPSCORES; i++, top10++)
  {
    if (retcode>0) retcode = _read(sc_fd, top10, sizeof(struct sc_ent));
    if (retcode<=0) top10->sc_gold = 0;
  }
}

void put_scores(struct sc_ent *top10)
{
  int i;

  for (i = 0; (i<TOPSCORES) && top10->sc_gold; i++, top10++)
  {
    if (_write(sc_fd, top10, sizeof(struct sc_ent))<=0) return;
  }
}

void pr_scores(int newrank, struct sc_ent *top10)
{
  int i;
  int curl;
  char dthstr[30];
  char *altmsg;

  clear();
  high();
  mvaddstr(0, 0, "Guildmaster's Hall Of Fame:");
  standend();
  yellow();
  mvaddstr(2, 0, "Gold");
  for (i = 0; i<TOPSCORES; i++, top10++)
  {
    altmsg = NULL;
    brown();
    if (newrank-1==i)
    {
      yellow();
    }
    if (top10->sc_gold<=0) break;
    curl = 4+((COLS==40)?(i*2):i);
    move(curl, 0);
    printw("%d ", top10->sc_gold);
    move(curl, 6);
    if (newrank-1!=i) red();
    printw("%s", top10->sc_name);
    if ((newrank)-1!=i) brown();
    if (top10->sc_level>=26) altmsg = " Honored by the Guild";
    if (isalpha(top10->sc_fate))
    {
      sprintf(dthstr, " killed by %s", killname((0xff&top10->sc_fate), TRUE));
      if (COLS==40 && strlen(dthstr)>23) strcpy(dthstr, " killed");
    }
    else switch(top10->sc_fate)
    {
      case 2: altmsg = " A total winner!"; break;
      case 1: strcpy(dthstr, " quit"); break;
      default: strcpy(dthstr, " weirded out");
    }
    if ((strlen(top10->sc_name)+10+strlen(he_man[top10->sc_rank-1])) < (size_t)COLS)
    {
      if (top10->sc_rank>1 && (strlen(top10->sc_name))) printw(" \"%s\"", he_man[top10->sc_rank-1]);
    }
    if (COLS==40) move(curl+1, 6);
    if (altmsg==NULL) printw("%s on level %d", dthstr, top10->sc_level);
    else addstr(altmsg);
  }
  standend();
  if (COLS==80) addstr("\n\n\n\n");
}

int add_scores(struct sc_ent *newscore, struct sc_ent *oldlist)
{
  struct sc_ent *sentry, *insert;
  int retcode = TOPSCORES+1;

  for (sentry = &oldlist[TOPSCORES-1]; sentry>=oldlist; sentry--)
  {
    if ((unsigned)newscore->sc_gold>(unsigned)sentry->sc_gold)
    {
      insert = sentry;
      retcode--;
      if ((insert<&oldlist[TOPSCORES-1]) && sentry->sc_gold) sentry[1] = *sentry;
    }
    else break;
  }
  if (retcode==11) return 0;
  *insert = *newscore;
  return retcode;
}

//death: Do something really fun when he dies
void death(char monst)
{
  char *killer;
  char buf[MAXSTR];
  int year;


  purse -= purse/10;

  clear();
  drop_curtain();
  clear();
  brown();
  box((COLS==40)?1:7, (COLS-28)/2, 22, (COLS+28)/2);
  standend();
  center(10, "REST");
  center(11, "IN");
  center(12, "PEACE");
  red();
  center(21, "  *    *      * ");
  green();
  center(22, "___\\/(\\/)/(\\/ \\\\(//)\\)\\/(//)\\\\)//(\\__");
  standend();
  center(14, whoami);
  standend();
  killer = killname(monst, TRUE);
  strcpy(buf, "killed by");
  center(15, buf);
  center(16, killer);
  sprintf(buf, "%u Au", purse);
  center(18, buf);
  year = 2012; //TODO
  sprintf(buf, "%u", year);
  center(19, buf);
  raise_curtain();
  move(LINES-1, 0);
  score(purse, 0, monst);

  exit(0);
}

//total_winner: Code for a winner
void total_winner()
{

  THING *obj;
  int worth;
  byte c;
  int oldpurse;

  clear();

#ifdef MINROG

  if (!terse)
  {
    standout();
    printw("                                                               \n");
    printw("  @   @               @   @           @          @@@  @     @  \n");
    printw("  @   @               @@ @@           @           @   @     @  \n");
    printw("  @   @  @@@  @   @   @ @ @  @@@   @@@@  @@@      @  @@@    @  \n");
    printw("   @@@@ @   @ @   @   @   @     @ @   @ @   @     @   @     @  \n");
    printw("      @ @   @ @   @   @   @  @@@@ @   @ @@@@@     @   @     @  \n");
    printw("  @   @ @   @ @  @@   @   @ @   @ @   @ @         @   @  @     \n");
    printw("   @@@   @@@   @@ @   @   @  @@@@  @@@@  @@@     @@@   @@   @  \n");
  }
  printw("                                                               \n");
  printw("     Congratulations, you have made it to the light of day!    \n");
  standend();
  printw("\nYou have joined the elite ranks of those who have escaped the\n");
  printw("Dungeons of Doom alive.  You journey home and sell all your loot at\n");
  printw("a great profit and are admitted to the fighters guild.\n");

#else

  printw("Congratulations!\n\nYou have made it to the light of day!\n\n\n\n");
  printw("You journey home and sell all your\n");
  printw("loot at a great profit and are\n");
  printw("admitted to the fighters guild.\n\n\n");

#endif MINROG

  mvaddstr(LINES-1, 0, "--Press space to continue--");
  wait_for(' ');
  clear();
  mvaddstr(0, 0, "   Worth  Item");
  oldpurse = purse;
  for (c = 'a', obj = ppack; obj!=NULL; c++, obj = next(obj))
  {
    switch (obj->o_type)
    {
      case FOOD:
        worth = 2*obj->o_count;
      break;

      case WEAPON:
        switch (obj->o_which)
        {
          case MACE: worth = 8; break;
          case SWORD: worth = 15; break;
          case CROSSBOW: worth = 30; break;
          case ARROW: worth = 1; break;
          case DAGGER: worth = 2; break;
          case TWOSWORD: worth = 75; break;
          case DART: worth = 1; break;
          case BOW: worth = 15; break;
          case BOLT: worth = 1; break;
          case SPEAR: worth = 5; break;
        }
        worth *= 3*(obj->o_hplus+obj->o_dplus)+obj->o_count;
        obj->o_flags |= ISKNOW;
      break;

      case ARMOR:
        switch (obj->o_which)
        {
          case LEATHER: worth = 20; break;
          case RING_MAIL: worth = 25; break;
          case STUDDED_LEATHER: worth = 20; break;
          case SCALE_MAIL: worth = 30; break;
          case CHAIN_MAIL: worth = 75; break;
          case SPLINT_MAIL: worth = 80; break;
          case BANDED_MAIL: worth = 90; break;
          case PLATE_MAIL: worth = 150; break;
        }
        worth += (9-obj->o_ac)*100;
        worth += (10*(a_class[obj->o_which]-obj->o_ac));
        obj->o_flags |= ISKNOW;
      break;

      case SCROLL:
        worth = s_magic[obj->o_which].mi_worth;
        worth *= obj->o_count;
        if (!s_know[obj->o_which]) worth /= 2;
        s_know[obj->o_which] = TRUE;
      break;

      case POTION:
        worth = p_magic[obj->o_which].mi_worth;
        worth *= obj->o_count;
        if (!p_know[obj->o_which]) worth /= 2;
        p_know[obj->o_which] = TRUE;
      break;

      case RING:
        worth = r_magic[obj->o_which].mi_worth;
        if (obj->o_which==R_ADDSTR || obj->o_which==R_ADDDAM || obj->o_which==R_PROTECT || obj->o_which==R_ADDHIT)
        if (obj->o_ac>0) worth += obj->o_ac*100;
        else worth = 10;
        if (!(obj->o_flags&ISKNOW)) worth /= 2;
        obj->o_flags |= ISKNOW;
        r_know[obj->o_which] = TRUE;
      break;

      case STICK:
        worth = ws_magic[obj->o_which].mi_worth;
        worth += 20*obj->o_charges;
        if (!(obj->o_flags&ISKNOW)) worth /= 2;
        obj->o_flags |= ISKNOW;
        ws_know[obj->o_which] = TRUE;
      break;

      case AMULET:
        worth = 1000;
      break;
    }
    if (worth<0) worth = 0;
    move(c-'a'+1, 0);
    printw("%c) %5d  %s", c, worth, inv_name(obj, FALSE));
    purse += worth;
  }
  move(c-'a'+1, 0);
  printw("   %5u  Gold Pieces          ", oldpurse);
  score(purse, 2, 0);


  exit(0);
}

//killname: Convert a code to a monster name
char *killname(char monst, bool doart)
{
  char *sp;
  bool article;

  sp = prbuf;
  article = TRUE;
  switch (monst)
  {
    case 'a': sp = "arrow"; break;
    case 'b': sp = "bolt"; break;
    case 'd': sp = "dart"; break;
    case 's': sp = "starvation"; article = FALSE; break;
    case 'f': sp = "fall"; break;
    default:
      if (monst>='A' && monst<='Z') sp = monsters[monst-'A'].m_name;
      else {sp = "God"; article = FALSE;}
    break;
  }
  if (doart && article) sprintf(prbuf, "a%s ", vowelstr(sp));
  else prbuf[0] = '\0';
  strcat(prbuf, sp);
  return prbuf;
}
