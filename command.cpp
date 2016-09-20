//Read and execute the user commands
//command.c   1.44    (A.I. Design)   2/14/85

#include "rogue.h"
#include "game_state.h"
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
    if (sleep_timer)
    {
      if (--sleep_timer<=0) {
          msg("you can move again");
          sleep_timer = 0;
      }
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
  int ch;
  ch = readchar();
  switch (ch)
  {
  case '\b': ch = 'h'; break;
  case '+': ch = 't'; break;
  case '-': ch = 'z'; break;
  }
  if (msg_position && !running) msg("");
  return ch;
}

//Read a command, setting things up according to prefix like devices. Return the command character to be executed.
int read_command()
{
  int command, ch, junk;

  counts_as_turn = true;
  bool was_fast_play_enabled = fast_play_enabled;
  fastmode = fast_play_enabled;
  look(true);
  if (!running) 
      stop_at_door = false;
  do_take = true;
  again = false;

  --repeat_cmd_count;
  if (repeat_cmd_count || lastcount)
      show_count();

  if (repeat_cmd_count > 0) {
      do_take = lasttake;
      command = lastch;
      fastmode = false;
  }
  else
  {
    repeat_cmd_count = 0;
    if (running) {
        command = run_character;
        do_take = lasttake;
    }
    else
    {
      for (command = 0; command==0;)
      {
        ch = com_char();
        if (was_fast_play_enabled != fast_play_enabled)
            fastmode = !fastmode;
        switch (ch)
        {
        case '0': case '1': case '2': case '3': case '4': case '5': case '6': case '7': case '8': case '9':
          junk = repeat_cmd_count*10;
          if ((junk += ch-'0')>0 && junk<10000) 
              repeat_cmd_count = junk;
          show_count();
          break;

        case 'f': // f: toggle fast mode for this turn
          fastmode = !fastmode; 
          break;
        case 'g': // g + direction: move onto an item without picking it up
          do_take = false; 
          break;
        case 'a': //a: repeat last command
          command = lastch; 
          repeat_cmd_count = lastcount; 
          do_take = lasttake; 
          again = true; 
          break;
        case ' ': 
            break;
        case ESCAPE:
            stop_at_door = false;
            repeat_cmd_count = 0; 
            show_count(); 
            break;
        default: command = ch; break;
        }
      }
    }
  }
  if (repeat_cmd_count)
      fastmode = false;
  switch (command)
  {
  case 'h': case 'j': case 'k': case 'l': case 'y': case 'u': case 'b': case 'n':
    if (fastmode && !running)
    {
      if (!player.is_blind()) {
          stop_at_door = true; 
          firstmove = true;
      }
      command = toupper(command);
    }

  case 'H': case 'J': case 'K': case 'L': case 'Y': case 'U': case 'B': case 'N': case 'q': case 'r': case 's': case 'z': case 't': case '.':

  case CTRL('D'): case 'C':

    break;

  default: 
      repeat_cmd_count = 0;
  }

  lastcount = repeat_cmd_count;
  lastch = command;  
  lasttake = do_take;
  return command;
}

void show_count()
{
  move(LINES-2, COLS-4);
  if (repeat_cmd_count > 0) 
      printw("%-4d", repeat_cmd_count);
  else
      addstr("    ");
}

void execcom()
{
  Coord mv;
  int ch;

  do
  {
    switch (ch = read_command())
    {
    case 'h': case 'j': case 'k': case 'l': case 'y': case 'u': case 'b': case 'n': 
        find_dir(ch, &mv);
        do_move(mv.y, mv.x);
        break;
    case 'H': case 'J': case 'K': case 'L': case 'Y': case 'U': case 'B': case 'N': 
        do_run(tolower(ch));
        break;
    case 't':
        if (get_dir())
            missile(delta.y, delta.x); 
        else 
            counts_as_turn = false; 
        break;
    case 'Q':
        counts_as_turn = false; 
        quit(); 
        break;
    case 'i':
        counts_as_turn = false;
        inventory(player.pack, 0, ""); 
        break;
    case 'd':
        drop();
        break;
    case 'q':
        quaff(); 
        break;
    case 'r': 
        read_scroll();
        break;
    case 'e':
        eat();
        break;
    case 'w':
        wield(); 
        break;
    case 'W':
        wear(); 
        break;
    case 'T': 
        take_off();
        break;
    case 'P':
        ring_on();
        break;
    case 'R': 
        ring_off();
        break;
    case 'c':
        counts_as_turn = false; 
        call();
        break;
    case '>':
        counts_as_turn = false; 
        go_down_stairs();
        break;
    case '<':
        counts_as_turn = false; 
        go_up_stairs(); 
        break;
    case '/':
        counts_as_turn = false; 
        help(helpobjs);
        break;
    case '?':
        counts_as_turn = false; 
        help(helpcoms); 
        break;
    case '!': 
        counts_as_turn = false; 
        fakedos(); 
        break;
    case 's':
        search(); 
        break;
    case 'z': 
        if (get_dir()) 
            do_zap();
        else 
            counts_as_turn = false; 
        break;
    case 'D': 
        counts_as_turn = false; 
        discovered();
        break;
    case CTRL('T'):
      {
        bool new_value = !in_brief_mode();
        set_brief_mode(new_value);
        msg(new_value ? "Ok, I'll be brief" : "Goodie, I can use big words again!"); 
        counts_as_turn = false;
        break;
      }
    case 'F':
        counts_as_turn = false;
        record_macro();
        break;
    case CTRL('F'):
        counts_as_turn = false;
        //todo: revisit macro later, this definitely isn't safe
        //typeahead = game->get_environment("macro").c_str();
        break;
    case CTRL('R'): 
        counts_as_turn = false;
        msg(last_message);
        break;
    case 'v':
      counts_as_turn = false;
      msg("Rogue version %d.%d", REV, VER);
      break;
    case 'S':
        counts_as_turn = false;
        save_game(); 
        break;
    case '.':
        doctor(); break;

    case '^':
      counts_as_turn = false;
      if (get_dir())
      {
        Coord lookat;

        lookat.y = player.pos.y+delta.y;
        lookat.x = player.pos.x+delta.x;
        if (get_tile(lookat.y, lookat.x)!=TRAP) 
            msg("no trap there.");
        else msg("you found %s", tr_name(get_flags(lookat.y, lookat.x)&F_TMASK));
      }
      break;

    case 'o': 
        counts_as_turn = false; 
        msg("i don't have any options, oh my!");
        break;
    case CTRL('L'): 
        counts_as_turn = false; 
        msg("the screen looks fine to me"); 
        break;

    case CTRL('W'):
      counts_as_turn = false; 
      game->hero().toggle_wizard();
      break; 

    default:
      if (game->hero().is_wizard()) {
        switch(ch){
          //Wizard commands
        case 'C': 
          counts_as_turn = false; create_obj(); break;
        case 'X': 
          counts_as_turn = false; show_map(true); break;
        case 'Z': 
          counts_as_turn = false;  show_map(false); break;
        default:
          counts_as_turn = false; 
          msg("illegal command '%s'", unctrl(ch)); 
          repeat_cmd_count = 0; 
        }
      }
      else{
        counts_as_turn = false; 
        msg("illegal command '%s'", unctrl(ch)); 
        repeat_cmd_count = 0; 
        break;
      }
    }
    if (take && do_take)
        pick_up(take);
    take = 0;
    if (!running) 
        stop_at_door = false;
  } while (counts_as_turn==false);
}
