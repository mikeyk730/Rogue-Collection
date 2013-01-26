//Read a scroll and let it happen
//scrolls.c   1.4 (AI Design) 12/14/84

#include "rogue.h"
#include "scrolls.h"
#include "monsters.h"
#include "pack.h"
#include "list.h"
#include "curses.h"
#include "io.h"
#include "main.h"
#include "misc.h"
#include "wizard.h"
#include "slime.h"
#include "level.h"

char *laugh = "you hear maniacal laughter%s.";
char *in_dist = " in the distance";

//read_scroll: Read a scroll from the pack and do the appropriate thing
void read_scroll()
{
  THING *obj;
  int y, x;
  byte ch;
  THING *op;
  bool discardit = FALSE;

  obj = get_item("read", SCROLL);
  if (obj==NULL) return;
  if (obj->o_type!=SCROLL) {msg("there is nothing on it to read"); return;}
  ifterse("the scroll vanishes", "as you read the scroll, it vanishes");
  //Calculate the effect it has on the poor guy.
  if (obj==cur_weapon) cur_weapon = NULL;
  switch (obj->o_which)
  {
  case S_CONFUSE: //Scroll of monster confusion.  Give him that power.
    player.t_flags |= CANHUH;
    msg("your hands begin to glow red");
    break;

  case S_ARMOR:
    if (cur_armor!=NULL)
    {
      cur_armor->o_ac--;
      cur_armor->o_flags &= ~ISCURSED;
      ifterse("your armor glows faintly", "your armor glows faintly for a moment");
    }
    break;

  case S_HOLD: //Hold monster scroll.  Stop all monsters within two spaces from chasing after the hero.
    for (x = hero.x-3; x<=hero.x+3; x++)
      if (x>=0 && x<COLS)
        for (y = hero.y-3; y<=hero.y+3; y++)
          if ((y>0 && y<maxrow) && ((op = moat(y, x))!=NULL))
          {
            op->t_flags &= ~ISRUN;
            op->t_flags |= ISHELD;
          }
          break;

  case S_SLEEP: //Scroll which makes you fall asleep
    s_know[S_SLEEP] = TRUE;
    no_command += rnd(SLEEP_TIME)+4;
    player.t_flags &= ~ISRUN;
    msg("you fall asleep");
    break;

  case S_CREATE:
    {
      coord mp;

      if (plop_monster(hero.y, hero.x, &mp) && (op = new_item())!=NULL) new_monster(op, randmonster(FALSE), &mp);
      else ifterse("you hear a faint cry of anguish", "you hear a faint cry of anguish in the distance");

      break;
    }

  case S_IDENT: //Identify, let the rogue figure something out
    s_know[S_IDENT] = TRUE;
    msg("this scroll is an identify scroll");
    if (!strcmp(s_menu, "on") || !strcmp(s_menu, "sel")) more(" More ");
    whatis(TRUE);
    break;

  case S_MAP: //Scroll of magic mapping.
    s_know[S_MAP] = TRUE;
    msg("oh, now this scroll has a map on it");
    //Take all the things we want to keep hidden out of the window
    for (y = 1; y<maxrow; y++) for (x = 0; x<COLS; x++)
    {
      switch (ch = chat(y, x))
      {
      case VWALL: case HWALL: case ULWALL: case URWALL: case LLWALL: case LRWALL:
        if (!(flags_at(y, x)&F_REAL)) {
          ch = DOOR; 
          set_chat(y, x, DOOR);
          unset_flag(y, x, F_REAL);
        }
      case DOOR: case PASSAGE: case STAIRS:
        if ((op = moat(y, x))!=NULL) if (op->t_oldch==' ') op->t_oldch = ch;
        break;
      default: ch = ' ';
      }
      if (ch==DOOR)
      {
        move(y, x);
        if (curch()!=DOOR) standout();
      }
      if (ch!=' ') mvaddch(y, x, ch);
      standend();
    }
    break;

  case S_GFIND: //Scroll of food detection
    ch = FALSE;
    for (op = lvl_obj; op!=NULL; op = next(op))
    {
      if (op->o_type==FOOD)
      {
        ch = TRUE;
        standout();
        mvaddch(op->o_pos.y, op->o_pos.x, FOOD);
        standend();
      }
      //as a bonus this will detect amulets as well
      else if (op->o_type==AMULET)
      {
        ch = TRUE;
        standout();
        mvaddch(op->o_pos.y, op->o_pos.x, AMULET);
        standend();
      }
    }
    if (ch) {s_know[S_GFIND] = TRUE; msg("your nose tingles as you sense food");}
    else ifterse("you hear a growling noise close by", "you hear a growling noise very close to you");
    break;

  case S_TELEP: //Scroll of teleportation: Make him disappear and reappear
    {
      struct room *cur_room;

      cur_room = proom;
      teleport();
      if (cur_room!=proom) s_know[S_TELEP] = TRUE;

      break;
    }

  case S_ENCH:
    if (cur_weapon==NULL || cur_weapon->o_type!=WEAPON) msg("you feel a strange sense of loss");
    else
    {
      cur_weapon->o_flags &= ~ISCURSED;
      if (rnd(2)==0) cur_weapon->o_hplus++;
      else cur_weapon->o_dplus++;
      ifterse("your %s glows blue", "your %s glows blue for a moment", w_names[cur_weapon->o_which]);
    }
    break;

  case S_SCARE: //Reading it is a mistake and produces laughter at the poor rogue's boo boo.
    msg(laugh, terse || expert?"":in_dist);
    break;

  case S_REMOVE:
    if (cur_armor!=NULL) cur_armor->o_flags &= ~ISCURSED;
    if (cur_weapon!=NULL) cur_weapon->o_flags &= ~ISCURSED;
    if (cur_ring[LEFT]!=NULL) cur_ring[LEFT]->o_flags &= ~ISCURSED;
    if (cur_ring[RIGHT]!=NULL) cur_ring[RIGHT]->o_flags &= ~ISCURSED;
    ifterse("somebody is watching over you", "you feel as if somebody is watching over you");
    break;

  case S_AGGR: //This scroll aggravates all the monsters on the current level and sets them running towards the hero
    aggravate();
    ifterse("you hear a humming noise", "you hear a high pitched humming noise");
    break;

  case S_NOP:
    msg("this scroll seems to be blank");
    break;

  case S_VORPAL:
    //Extra Vorpal Enchant Weapon
    //    Give weapon +1,+1
    //    Is extremely vorpal against one certain type of monster
    //    Against this type (o_enemy) the weapon gets:
    // +4,+4
    // The ability to zap one such monster into oblivion
    //
    //    Some of these are cursed and if the rogue misses her saving
    //    throw she will be forced to attack monsters of this type
    //    whenever she sees one (not yet implemented)
    //
    //If he doesn't have a weapon I get to chortle again!
    if (cur_weapon==NULL || cur_weapon->o_type!=WEAPON) msg(laugh, terse || expert?"":in_dist);
    else
    {
      //You aren't allowed to doubly vorpalize a weapon.
      if (cur_weapon->o_enemy!=0)
      {
        msg("your %s vanishes in a puff of smoke", w_names[cur_weapon->o_which]);
        detach(ppack, cur_weapon);
        discard(cur_weapon);
        cur_weapon = NULL;
      }
      else
      {
        cur_weapon->o_enemy = pick_mons();
        cur_weapon->o_hplus++;
        cur_weapon->o_dplus++;
        cur_weapon->o_charges = 1;
        msg(flash, w_names[cur_weapon->o_which], terse || expert?"":intense);
      }
    }
    break;

  default: msg("what a puzzling scroll!"); return;
  }
  look(TRUE); //put the result of the scroll on the screen
  status();
  //Get rid of the thing
  inpack--;
  if (obj->o_count>1) obj->o_count--;
  else {detach(ppack, obj); discardit = TRUE;}
  call_it(s_know[obj->o_which], &s_guess[obj->o_which]);
  if (discardit) discard(obj);
}
