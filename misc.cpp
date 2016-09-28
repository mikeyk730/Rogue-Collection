//All sorts of miscellaneous routines
//misc.c       1.4             (A.I. Design)   12/14/84

#include <algorithm>
#include <stdio.h>

#include "rogue.h"
#include "game_state.h"
#include "io.h"
#include "daemons.h"
#include "misc.h"
#include "monsters.h"
#include "pack.h"
#include "output_interface.h"
#include "chase.h"
#include "rip.h"
#include "new_leve.h"
#include "list.h"
#include "mach_dep.h"
#include "main.h"
#include "fight.h"
#include "daemon.h"
#include "level.h"
#include "rings.h"
#include "thing.h"
#include "scrolls.h"
#include "potions.h"
#include "sticks.h"
#include "armor.h"
#include "hero.h"
#include "room.h"
#include "monster.h"

const int MACROSZ = 41;

//tr_name: Print the name of a trap
char *tr_name(byte type)
{
  switch (type)
  {
  case T_DOOR: return "a trapdoor";
  case T_BEAR: return "a beartrap";
  case T_SLEEP: return "a sleeping gas trap";
  case T_ARROW: return "an arrow trap";
  case T_TELEP: return "a teleport trap";
  case T_DART: return "a poison dart trap";
  }
  msg("weird trap: %d", type);
  return NULL;
}

//look: A quick glance all around the player
void look(bool wakeup)
{
  int x, y;
  byte ch, pch;
  Monster* monster;
  struct Room *room;
  int ey, ex;
  int passcount = 0;
  byte pfl, fp;
  int sy, sx, sumhero, diffhero;
  const int COLS = game->screen().columns();

  room = game->hero().room;
  pfl = game->level().get_flags(game->hero().pos);
  pch = game->level().get_tile(game->hero().pos);
  //if the hero has moved
  if (!equal(oldpos, game->hero().pos))
  {
    if (!game->hero().is_blind())
    {
      for (x = oldpos.x-1; x<=(oldpos.x+1); x++)
        for (y = oldpos.y-1; y<=(oldpos.y+1); y++)
        {
          if ((y==game->hero().pos.y && x==game->hero().pos.x) || offmap({x,y})) continue;
          game->screen().move(y, x);
          ch = game->screen().curch();
          if (ch==FLOOR)
          {
            if (oldrp->is_dark() && !oldrp->is_gone())
                game->screen().addch(' ');
          }
          else
          {
            fp = game->level().get_flags({x, y});
            //if the maze or passage (that the hero is in!!) needs to be redrawn (passages once drawn always stay on) do it now.
            if (((fp&F_MAZE) || (fp&F_PASS)) && (ch!=PASSAGE) && (ch!=STAIRS) && ((fp&F_PNUM)==(pfl & F_PNUM))) 
              game->screen().addch(PASSAGE);
          }
        }
    }
    oldpos = game->hero().pos;
    oldrp = room;
  }
  ey = game->hero().pos.y+1;
  ex = game->hero().pos.x+1;
  sx = game->hero().pos.x-1;
  sy = game->hero().pos.y-1;
  if (game->modifiers.stop_at_door() && !game->modifiers.first_move() && game->modifiers.is_running()) {
      sumhero = game->hero().pos.y+game->hero().pos.x; 
      diffhero = game->hero().pos.y-game->hero().pos.x;
  }
  for (y = sy; y<=ey; y++) if (y>0 && y<maxrow) for (x = sx; x<=ex; x++)
  {
    if (x<=0 || x>=COLS) continue;
    if (!game->hero().is_blind())
    {
      if (y==game->hero().pos.y && x==game->hero().pos.x) continue;
    }
    else if (y!=game->hero().pos.y || x!=game->hero().pos.x) continue;
    //THIS REPLICATES THE moat() MACRO.  IF MOAT IS CHANGED, THIS MUST BE CHANGED ALSO ?? What does this really mean ??
    fp = game->level().get_flags({x, y});
    ch = game->level().get_tile({x, y});
    //No Doors
    if (pch!=DOOR && ch!=DOOR)
      //Either hero or other in a passage
      if ((pfl&F_PASS)!=(fp & F_PASS))
      {
        //Neither is in a maze
        if (!(pfl&F_MAZE) && !(fp&F_MAZE)) continue;
      }
      //Not in same passage
      else if ((fp&F_PASS) && (fp&F_PNUM)!=(pfl & F_PNUM)) continue;
      if ((monster = game->level().monster_at({x, y}))!=NULL) if (game->hero().detects_others() && monster->is_invisible())
      {
        if (game->modifiers.stop_at_door() && !game->modifiers.first_move())
            game->modifiers.m_running = false;
        continue;
      }
      else
      {
        if (wakeup) wake_monster({x,y});
        if (monster->oldch != ' ' || (!(room->is_dark()) && !game->hero().is_blind())) monster->oldch = game->level().get_tile({x, y});
        if (game->hero().can_see_monster(monster)) ch = monster->disguise;
      }
      //The current character used for IBM ARMOR doesn't look right in Inverse
      if ((ch!=PASSAGE) && (fp&(F_PASS|F_MAZE))) if (ch!=ARMOR) game->screen().standout();
      game->screen().move(y, x);
      game->screen().addch(ch);
      game->screen().standend();
      if (game->modifiers.stop_at_door() && !game->modifiers.first_move() && game->modifiers.is_running())
      {
        switch (run_character)
        {
        case 'h': if (x==ex) continue; break;
        case 'j': if (y==sy) continue; break;
        case 'k': if (y==ey) continue; break;
        case 'l': if (x==sx) continue; break;
        case 'y': if ((y+x)-sumhero>=1) continue; break;
        case 'u': if ((y-x)-diffhero>=1) continue; break;
        case 'n': if ((y+x)-sumhero<=-1) continue; break;
        case 'b': if ((y-x)-diffhero<=-1) continue; break;
        }
        switch (ch)
        {
        case DOOR: 
            if (x==game->hero().pos.x || y==game->hero().pos.y) 
                game->modifiers.m_running = false;
            break;
        case PASSAGE:
            if (x==game->hero().pos.x || y==game->hero().pos.y) 
                passcount++; 
            break;
        case FLOOR: case VWALL: case HWALL: case ULWALL: case URWALL: case LLWALL: case LRWALL: case ' ': 
            break;
        default:
            game->modifiers.m_running = false; 
            break;
        }
      }
  }
  if (game->modifiers.stop_at_door() && !game->modifiers.first_move() && passcount>1)
      game->modifiers.m_running = false;
  game->screen().move(game->hero().pos.y, game->hero().pos.x);
  //todo:check logic
  if ((game->level().get_flags(game->hero().pos)&F_PASS) || (was_trapped>1) || (game->level().get_flags(game->hero().pos)&F_MAZE)) 
      game->screen().standout();
  game->screen().addch(PLAYER);
  game->screen().standend();
  if (was_trapped) {beep(); was_trapped = 0;}
}

//find_obj: Find the unclaimed object at y, x
Item *find_obj(Coord p)
{
  for (auto it = game->level().items.begin(); it != game->level().items.end(); ++it) {
      Item* op = *it;
      if (op->pos.y == p.y && op->pos.x == p.x) 
          return op;
  }

  debug("Non-object %c %d,%d", game->level().get_tile(p), p.y, p.x);
  return NULL; //NOTREACHED
}

//add_haste: Add a haste to the player
int add_haste(bool potion)
{
  if (game->hero().is_fast())
  {
    game->sleep_timer += rnd(8);
    game->hero().set_running(false);
    extinguish(nohaste);
    game->hero().set_is_fast(false);
    msg("you faint from exhaustion");
    return false;
  }
  else
  {
    game->hero().set_is_fast(true);
    if (potion) fuse(nohaste, 0, rnd(4)+10);
    return true;
  }
}

//aggravate: Aggravate all the monsters on this level
void aggravate()
{
    std::for_each(game->level().monsters.begin(), game->level().monsters.end(), [](Monster *monster){
        monster->start_run();
    });
}

//vowelstr: For printfs: if string starts with a vowel, return "n" for an "an".

const char *vowelstr(const char *str)
{
  switch (*str)
  {
  case 'a': case 'A': case 'e': case 'E': case 'i': case 'I': case 'o': case 'O': case 'u': case 'U':
      return "n";
  default:
      return "";
  }
}

//is_current: See if the object is one of the currently used items
int is_current(Item *obj)
{
  if (obj==NULL) return false;
  if (obj==get_current_armor() || obj==get_current_weapon() || obj==get_ring(LEFT) || obj==get_ring(RIGHT))
  {
    msg("That's already in use");
    return true;
  }
  return false;
}

//get_dir: Set up the direction co_ordinate for use in various "prefix" commands
int get_dir()
{
  int ch;

  if (again) return true;
  msg("which direction? ");
  do {
    if ((ch = readchar())==ESCAPE) {
      msg(""); 
      return false;
    }
  } while (find_dir(ch, &delta)==0);
  msg("");
  if (game->hero().is_confused() && rnd(5)==0) do
  {
    delta.y = rnd(3)-1;
    delta.x = rnd(3)-1;
  } while (delta.y==0 && delta.x==0);
  return true;
}

bool find_dir(byte ch, Coord *cp)
{
  bool gotit;

  gotit = true;
  switch (ch)
  {
  case 'h': case'H': cp->y = 0; cp->x = -1; break;
  case 'j': case'J': cp->y = 1; cp->x = 0; break;
  case 'k': case'K': cp->y = -1; cp->x = 0; break;
  case 'l': case'L': cp->y = 0; cp->x = 1; break;
  case 'y': case'Y': cp->y = -1; cp->x = -1; break;
  case 'u': case'U': cp->y = -1; cp->x = 1; break;
  case 'b': case'B': cp->y = 1; cp->x = -1; break;
  case 'n': case'N': cp->y = 1; cp->x = 1; break;
  default: gotit = false; break;
  }
  return gotit;
}

//sign: Return the sign of the number
int sign(int n)
{
    if (n < 0)
        return -1;
    else
        return (n > 0);
}

//spread: Give a spread around a given number (+/- 10%)
int spread(int nm)
{
  int r = nm-nm/10+rnd(nm/5);
  return r;
}

//step_ok: Returns true if it is ok to step on ch
int step_ok(int ch)
{
  switch (ch)
  {
  case ' ': case VWALL: case HWALL: case ULWALL: case URWALL: case LLWALL: case LRWALL: return false;
  default: return ((ch<'A') || (ch>'Z'));
  }
}

//goodch: Decide how good an object is and return the correct character for printing.
int goodch(Item *obj)
{
  int ch = MAGIC;

  if (obj->is_cursed())
      ch = BMAGIC;
  switch (obj->type)
  {
  case ARMOR:
    if (obj->get_armor_class()>get_default_class(obj->which)) ch = BMAGIC;
    break;
  case WEAPON:
    if (obj->get_hit_plus()<0 || obj->get_damage_plus()<0) ch = BMAGIC;
    break;
  case SCROLL:
    if (is_bad_scroll(obj)) ch = BMAGIC;
    break;
  case POTION:
    if (is_bad_potion(obj)) ch = BMAGIC;
    break;
  case STICK:
    switch (obj->which) {case WS_HASTE_M: case WS_TELTO: ch = BMAGIC; break;}
    break;
  case RING:
    switch (obj->which)
    {
    case R_PROTECT: case R_ADDSTR: case R_ADDDAM: case R_ADDHIT: if (obj->get_ring_level()<0) ch = BMAGIC; break;
    case R_AGGR: case R_TELEPORT: ch = BMAGIC; break;
    }
    break;
  }
  return ch;
}

//help: prints out help screens
void help(const char*const* helpscr)
{
  int hcount = 0;
  int hrow, hcol;
  int isfull;
  byte answer = 0;

  game->screen().wdump();
  while (*helpscr && answer!=ESCAPE)
  {
    isfull = false;
    if ((hcount%(in_small_screen_mode()?23:46))==0) game->screen().clear();
    //determine row and column
    hcol = 0;
    if (in_small_screen_mode())
    {
      hrow = hcount%23;
      if (hrow==22) isfull = true;
    }
    else
    {
      hrow = (hcount%46)/2;
      if (hcount%2) hcol = 40;
      if (hrow==22 && hcol==40) isfull = true;
    }
    game->screen().move (hrow, hcol);
    game->screen().addstr(*helpscr++);
    //decide if we need print a continue type message
    if ((*helpscr==0) || isfull)
    {
      if (*helpscr==0) 
        game->screen().mvaddstr(24, 0, "--press space to continue--");
      else if (in_small_screen_mode()) 
        game->screen().mvaddstr(24, 0, "--Space for more, Esc to continue--");
      else 
        game->screen().mvaddstr(24, 0, "--Press space for more, Esc to continue--");
      do {
        answer = readchar();
      } while (answer!=' ' && answer!=ESCAPE);
    }
    hcount++;
  }
  game->screen().wrestor();
}

int distance(Coord a, Coord b)
{
    int dx, dy;
    dx = (a.x - b.x);
    dy = (a.y - b.y);
    return dx*dx + dy*dy;
}

int equal(Coord a, Coord b)
{
  return (a.x == b.x && a.y == b.y);
}

int offmap(Coord p)
{
    const int COLS = game->screen().columns();
    return (p.y < 1 || p.y >= maxrow || p.x < 0 || p.x >= COLS);
}

//search: Player gropes about him to find hidden things.
void search()
{
  int y, x;
  int ey, ex;

  if (game->hero().is_blind()) 
    return;
  ey = game->hero().pos.y+1;
  ex = game->hero().pos.x+1;
  for (y = game->hero().pos.y-1; y<=ey; y++) for (x = game->hero().pos.x-1; x<=ex; x++)
  {
    if ((y==game->hero().pos.y && x==game->hero().pos.x) || offmap({x,y})) 
      continue;
    if (!(game->level().is_real({ x, y }))) {
      switch (game->level().get_tile({x, y}))
      {
      case VWALL: case HWALL: case ULWALL: case URWALL: case LLWALL: case LRWALL:
        if (rnd(5)!=0) break;
        game->level().set_tile({x, y}, DOOR);
        game->level().set_flag({x, y}, F_REAL);
        repeat_cmd_count = game->modifiers.m_running = false;
        break;
      case FLOOR:
        if (rnd(2)!=0) break;
        game->level().set_tile({x, y}, TRAP);
        game->level().set_flag({x, y}, F_REAL);
        repeat_cmd_count = game->modifiers.m_running = false;
        msg("you found %s", tr_name(game->level().get_trap_type({ x, y })));
        break;
      }
    }
  }
}


//go_down_stairs: He wants to go down a level
void go_down_stairs()
{
    if (game->level().get_tile(game->hero().pos) != STAIRS && !game->hero().is_wizard())
        msg("I see no way down");
    else {
        next_level();
        game->level().new_level(true);
    }
}

//go_up_stairs: He wants to go up a level
void go_up_stairs()
{
    if (game->level().get_tile(game->hero().pos) == STAIRS || game->hero().is_wizard()){
        if (has_amulet() || game->hero().is_wizard()) {
            if (prev_level() == 0)
                total_winner();
            game->level().new_level(true);
            msg("you feel a wrenching sensation in your gut");
        }
        else
            msg("your way is magically blocked");
    }
    else msg("I see no way up");
}

//call: Allow a user to call a potion, scroll, or ring something
void call()
{
    Item *obj = get_item("call", CALLABLE);
    if (obj == NULL) 
        return;

    ItemClass* item_class;
    switch (obj->type)
    {
    case RING:
    case POTION:
    case SCROLL:
    case STICK:
        item_class = &game->item_class(obj->type);
        break;
    default:
        msg("you can't call that anything");
        return;
    }

    if (item_class->is_discovered(obj->which)) {
        msg("that has already been identified");
        return;
    }

    std::string called = item_class->get_guess(obj->which);
    if (called.empty())
        called = item_class->get_identifier(obj->which);
    msg("Was called \"%s\"", called.c_str());

    msg("what do you want to call it? ");
    getinfo(prbuf, MAXNAME);
    if (*prbuf && *prbuf != ESCAPE)
        item_class->set_guess(obj->which, prbuf);
    msg("");
}

//prompt player for definition of macro
void record_macro()
{
    char buffer[MACROSZ];
    char* buf = buffer;

    memset(buf, 0, MACROSZ);
    char *cp = prbuf;

    msg("F9 was %s, enter new macro: ", game->get_environment("macro").c_str());
    if (getinfo(prbuf, MACROSZ - 1) != ESCAPE)
    do {
        if (*cp != CTRL('F'))
            *buf++ = *cp;
    } while (*cp++);
    msg("");
    clear_typeahead_buffer();
    game->set_environment("macro", buf);
}
