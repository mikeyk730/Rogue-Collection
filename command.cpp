//Read and execute the user commands
//command.c   1.44    (A.I. Design)   2/14/85

#include "rogue.h"
#include "daemons.h"
#include "daemon.h"
#include "command.h"
#include "main.h"
#include "io.h"
#include "wizard.h"
#include "misc.h"
#include "fakedos.h"
#include "save.h"
#include "armor.h"
#include "weapons.h"
#include "sticks.h"
#include "curses.h"
#include "passages.h"
#include "mach_dep.h"
#include "move.h"
#include "rings.h"
#include "things.h"
#include "potions.h"
#include "pack.h"
#include "scrolls.h"
#include "level.h"
#include "food.h"
#include "hero.h"

#include <ctype.h>

#define MACROSZ  41
static int lastcount;
static byte lastch, do_take, lasttake;

void command()
{
  int ntimes;

  if (player.is_fast())
      ntimes = rnd(2) + 2;
  else ntimes = 1;
  while (ntimes--)
  {
    status();
    SIG2();
    if (no_command)
    {
      if (--no_command<=0) {msg("you can move again"); no_command = 0;}
    }
    else execcom();
    do_fuses();
    do_daemons();
    for (ntimes = LEFT; ntimes<=RIGHT; ntimes++) if (get_ring(ntimes)) switch (get_ring(ntimes)->which)
    {
    case R_SEARCH: search(); break;
    case R_TELEPORT: if (rnd(50)==17) teleport(); break;
    }
  }
}

int com_char()
{
  int same, ch;
  ch = readchar();
  same = (fastmode==faststate);
  if (same) fastmode = faststate;
  else fastmode = !faststate;
  switch (ch)
  {
  case '\b': ch = 'h'; break;
  case '+': ch = 't'; break;
  case '-': ch = 'z'; break;
  }
  if (mpos && !running) msg("");
  return ch;
}

//Read a command, setting things up according to prefix like devices. Return the command character to be executed.
int get_prefix()
{
  int retch, ch, junk;

  after = true;
  fastmode = faststate;
  look(true);
  if (!running) door_stop = false;
  do_take = true;
  again = false;
  if (--count>0) {do_take = lasttake; retch = lastch; fastmode = false;}
  else
  {
    count = 0;
    if (running) {retch = runch; do_take = lasttake;}
    else
    {
      for (retch = 0; retch==0;)
      {
        switch (ch = com_char())
        {
        case '0': case '1': case '2': case '3': case '4': case '5': case '6': case '7': case '8': case '9':
          junk = count*10;
          if ((junk += ch-'0')>0 && junk<10000) count = junk;
          show_count();
          break;

        case 'f':
          fastmode = !fastmode; 
          break;
        case 'g': 
          do_take = false; 
          break;
        case 'a': 
          retch = lastch; 
          count = lastcount; 
          do_take = lasttake; 
          again = true; 
          break;
        case ' ': break;
        case ESCAPE: door_stop = false; count = 0; show_count(); break;
        default: retch = ch; break;
        }
      }
    }
  }
  if (count) fastmode = false;
  switch (retch)
  {
  case 'h': case 'j': case 'k': case 'l': case 'y': case 'u': case 'b': case 'n':
    if (fastmode && !running)
    {
      if (!player.is_blind()) {
          door_stop = true; 
          firstmove = true;
      }
      retch = toupper(retch);
    }

  case 'H': case 'J': case 'K': case 'L': case 'Y': case 'U': case 'B': case 'N': case 'q': case 'r': case 's': case 'z': case 't': case '.':

  case CTRL('D'): case 'C':

    break;

  default: count = 0;
  }
  if (count || lastcount) show_count();
  lastch = retch;
  lastcount = count;
  lasttake = do_take;
  return retch;
}

void show_count()
{
  move(LINES-2, COLS-4);
  if (count) printw("%-4d", count);
  else addstr("    ");
}

void execcom()
{
  Coord mv;
  int ch;

  do
  {
    switch (ch = get_prefix())
    {
    case 'h': case 'j': case 'k': case 'l': case 'y': case 'u': case 'b': case 'n': find_dir(ch, &mv); do_move(mv.y, mv.x); break;
    case 'H': case 'J': case 'K': case 'L': case 'Y': case 'U': case 'B': case 'N': do_run(tolower(ch)); break;
    case 't': if (get_dir()) missile(delta.y, delta.x); else after = false; break;
    case 'Q': after = false; quit(); break;
    case 'i': after = false; inventory(player.pack, 0, ""); break;
    case 'd': drop(); break;
    case 'q': quaff(); break;
    case 'r': read_scroll(); break;
    case 'e': eat(); break;
    case 'w': wield(); break;
    case 'W': wear(); break;
    case 'T': take_off(); break;
    case 'P': ring_on(); break;
    case 'R': ring_off(); break;
    case 'c': after = false; call(); break;
    case '>': after = false; d_level(); break;
    case '<': after = false; u_level(); break;
    case '/': after = false; help(helpobjs); break;
    case '?': after = false; help(helpcoms); break;
    case '!': after = false; fakedos(); break;
    case 's': search(); break;
    case 'z': if (get_dir()) do_zap(); else after = false; break;
    case 'D': after = false; discovered(); break;
    case CTRL('T'):
      {
        bool new_value = !in_brief_mode();
        set_brief_mode(new_value);
        msg(new_value ? "Ok, I'll be brief" : "Goodie, I can use big words again!"); 
        after = false;
        break;
      }
    case 'F': after = false; do_macro(macro, MACROSZ); break;
    case CTRL('F'): after = false; typeahead = macro; break;
    case CTRL('R'): after = false; msg(huh); break;

    case 'v':
      after = false;
      msg("Rogue version %d.%d", REV, VER);
      break;

    case 'S': after = false; save_game(); break;
    case '.': doctor(); break;

    case '^':
      after = false;
      if (get_dir())
      {
        Coord lookat;

        lookat.y = player.pos.y+delta.y;
        lookat.x = player.pos.x+delta.x;
        if (get_tile(lookat.y, lookat.x)!=TRAP) msg("no trap there.");
        else msg("you found %s", tr_name(get_flags(lookat.y, lookat.x)&F_TMASK));
      }
      break;

    case 'o': after = false; msg("i don't have any options, oh my!"); break;
    case CTRL('L'): after = false; msg("the screen looks fine to me"); break;

    case CTRL('W'):
      after = false; 
      set_wizard(!is_wizard());
      msg(is_wizard() ? "You are now a wizard!" : "You feel your magic powers fade away"); 
      break; 

    default:
      if (is_wizard()) {
        switch(ch){
          //Wizard commands
        case 'C': 
          after = false; create_obj(); break;
        case 'X': 
          after = false; show_map(true); break;
        case 'Z': 
          after = false;  show_map(false); break;
        default:
          after = false; 
          msg("illegal command '%s'", unctrl(ch)); 
          count = 0; 
        }
      }
      else{
        after = false; 
        msg("illegal command '%s'", unctrl(ch)); 
        count = 0; 
        break;
      }
    }
    if (take && do_take) pick_up(take);
    take = 0;
    if (!running) door_stop = false;
  } while (after==false);
}
