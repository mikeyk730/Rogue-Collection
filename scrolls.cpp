//Read a scroll and let it happen
//scrolls.c   1.4 (AI Design) 12/14/84

#include <stdio.h>

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
#include "thing.h"
#include "weapons.h"
#include "things.h"
#include "hero.h"

//Scroll types
#define S_CONFUSE   0
#define S_MAP       1
#define S_HOLD      2
#define S_SLEEP     3
#define S_ARMOR     4
#define S_IDENT     5
#define S_SCARE     6
#define S_GFIND     7
#define S_TELEP     8
#define S_ENCH      9
#define S_CREATE    10
#define S_REMOVE    11
#define S_AGGR      12
#define S_NOP       13
#define S_VORPAL    14

bool s_know[MAXSCROLLS];    //Does he know what a scroll does
char *s_guess[MAXSCROLLS];         //Players guess at what scroll is
struct Array s_names[MAXSCROLLS];  //Names of the scrolls

static char *c_set = "bcdfghjklmnpqrstvwxyz";
static char *v_set = "aeiou";

const char *laugh = "you hear maniacal laughter%s.";
const char *in_dist = " in the distance";

struct MagicItem s_magic[MAXSCROLLS] =
{
  {"monster confusion",   8, 140},
  {"magic mapping",       5, 150},
  {"hold monster",        3, 180},
  {"sleep",               5,   5},
  {"enchant armor",       8, 160},
  {"identify",           27, 100},
  {"scare monster",       4, 200},
  {"food detection",      4,  50},
  {"teleportation",       7, 165},
  {"enchant weapon",     10, 150},
  {"create monster",      5,  75},
  {"remove curse",        8, 105},
  {"aggravate monsters",  4,  20},
  {"blank paper",         1,   5},
  {"vorpalize weapon",    1, 300}
};

int does_know_scroll(int type)
{
  return s_know[type];
}

void discover_scroll(int type)
{
  s_know[type] = true;
}

int get_scroll_value(int type)
{
  return s_magic[type].worth;
}

const char* get_scroll_name(int type)
{
  return s_magic[type].name;
}

const char* get_scroll_guess(int type)
{
  return s_guess[type];
}

void set_scroll_guess(int type, const char* value)
{
  strcpy(s_guess[type], value);
}

//random_char_in(): return random character in given string
char random_char_in(char *string)
{
  return (string[rnd(strlen(string))]);
}

//getsyl(): generate a random syllable
char* getsyl()
{
  static char _tsyl[4];

  _tsyl[3] = 0;
  _tsyl[2] = random_char_in(c_set);
  _tsyl[1] = random_char_in(v_set);
  _tsyl[0] = random_char_in(c_set);
  return (_tsyl);
}

//init_names: Generate the names of the various scrolls
void init_names()
{
  int nsyl;
  char *cp, *sp;
  int i, nwords;

  for (i = 0; i<MAXSCROLLS; i++)
  {
    cp = prbuf;
    nwords = rnd(in_small_screen_mode()?3:4)+2;
    while (nwords--)
    {
      nsyl = rnd(2)+1;
      while (nsyl--)
      {
        sp = getsyl();
        if (&cp[strlen(sp)]>&prbuf[MAXNAME-1]) {nwords = 0; break;}
        while (*sp) *cp++ = *sp++;
      }
      *cp++ = ' ';
    }
    *--cp = '\0';
    //I'm tired of thinking about this one so just in case .....
    prbuf[MAXNAME] = 0;
    s_know[i] = false;
    s_guess[i] = (char *)&_guesses[iguess++];
    strcpy((char*)&s_names[i], prbuf);
    if (i>0) s_magic[i].prob += s_magic[i-1].prob;
  }
}

const char* get_title(int type)
{
  return s_names[type].storage;
}

void init_new_scroll(ITEM* scroll)
{
  scroll->type = SCROLL;
  scroll->which = pick_one(s_magic, MAXSCROLLS);
}

void read_monster_confusion()
{
  //Scroll of monster confusion.  Give him that power.
  player.set_can_confuse(true);
  msg("your hands begin to glow red");
}

void read_magic_mapping()
{
  //Scroll of magic mapping.
  int x, y;
  byte ch;
  AGENT* monster;
  
  s_know[S_MAP] = true;
  msg("oh, now this scroll has a map on it");
  //Take all the things we want to keep hidden out of the window
  for (y = 1; y<maxrow; y++) for (x = 0; x<COLS; x++)
  {
    switch (ch = get_tile(y, x))
    {
    case VWALL: case HWALL: case ULWALL: case URWALL: case LLWALL: case LRWALL:
      if (!(get_flags(y, x)&F_REAL)) {
        ch = DOOR; 
        set_tile(y, x, DOOR);
        unset_flag(y, x, F_REAL);
      }
    case DOOR: case PASSAGE: case STAIRS:
      if ((monster = monster_at(y, x)) != NULL) 
        if (monster->oldch == ' ')
          monster->oldch = ch;
      break;
    default: ch = ' ';
    }
    if (ch==DOOR)
    {
      move(y, x);
      if (curch()!=DOOR) 
        standout();
    }
    if (ch!=' ') 
      mvaddch(y, x, ch);
    standend();
  }
}

void read_hold_monster()
{
  //Hold monster scroll.  Stop all monsters within two spaces from chasing after the hero.
  int x, y;
  AGENT* monster;

  for (x = player.pos.x-3; x<=player.pos.x+3; x++) {
    if (x>=0 && x<COLS) {
      for (y = player.pos.y-3; y<=player.pos.y+3; y++) {
        if ((y>0 && y<maxrow) && ((monster = monster_at(y, x)) != NULL))
        {
          monster->set_running(false);
          monster->set_is_held(true);
        }
      }
    }
  }
}

void read_sleep()
{
  //Scroll which makes you fall asleep
  s_know[S_SLEEP] = true;
  sleep_timer += rnd(SLEEP_TIME)+4;
  player.set_running(false);
  msg("you fall asleep");
}

void read_enchant_armor()
{
  if (get_current_armor()!=NULL)
  {
    get_current_armor()->armor_class--;
    get_current_armor()->remove_curse();
    ifterse("your armor glows faintly", "your armor glows faintly for a moment");
  }
}

void read_identify()
{
  //Identify, let the rogue figure something out
  s_know[S_IDENT] = true;
  msg("this scroll is an identify scroll");
  if (strcmp(s_menu, "on") == 0) 
    more(" More ");
  whatis();
}

void read_scare_monster()
{
  //Reading it is a mistake and produces laughter at the poor rogue's boo boo.
  msg(laugh, short_msgs() ? "" : in_dist);
}

void read_food_detection()
{
  //Scroll of food detection
  ITEM* item;
  byte discover = false;

  for (item = lvl_obj; item != NULL; item = next(item))
  {
    if (item->type==FOOD)
    {
      discover = true;
      standout();
      mvaddch(item->pos.y, item->pos.x, FOOD);
      standend();
    }
    //as a bonus this will detect amulets as well
    else if (item->type==AMULET)
    {
      discover = true;
      standout();
      mvaddch(item->pos.y, item->pos.x, AMULET);
      standend();
    }
  }
  if (discover) {
    s_know[S_GFIND] = true; 
    msg("your nose tingles as you sense food");
  }
  else 
    ifterse("you hear a growling noise close by", "you hear a growling noise very close to you");
}

void read_teleportation()
{
  //Scroll of teleportation: Make him disappear and reappear
  struct Room *cur_room;
  cur_room = player.room;
  teleport();
  if (cur_room != player.room) 
    s_know[S_TELEP] = true;
}

void read_enchant_weapon()
{
  if (get_current_weapon() == NULL || get_current_weapon()->type != WEAPON)
    msg("you feel a strange sense of loss");
  else
  {
    get_current_weapon()->remove_curse();
    if (rnd(2)==0) get_current_weapon()->hit_plus++;
    else get_current_weapon()->damage_plus++;
    ifterse("your %s glows blue", "your %s glows blue for a moment", get_weapon_name(get_current_weapon()->which));
  }
}

void read_create_monster()
{
  AGENT* monster;
  Coord position;

  if (plop_monster(player.pos.y, player.pos.x, &position) && (monster = create_agent()) != NULL)
    new_monster(monster, randmonster(false, get_level()), &position, get_level());
  else 
    ifterse("you hear a faint cry of anguish", "you hear a faint cry of anguish in the distance");
}

void read_remove_curse()
{
    if (get_current_armor())
        get_current_armor()->remove_curse();
    if (get_current_weapon())
        get_current_weapon()->remove_curse();
    if (get_ring(LEFT))
        get_ring(LEFT)->remove_curse();
    if (get_ring(RIGHT))
        get_ring(RIGHT)->remove_curse();

    ifterse("somebody is watching over you", "you feel as if somebody is watching over you");
}

void read_aggravate_monsters()
{
  //This scroll aggravates all the monsters on the current level and sets them running towards the hero
  aggravate();
  ifterse("you hear a humming noise", "you hear a high pitched humming noise");
}

void read_blank_paper()
{
  msg("this scroll seems to be blank");
}

void read_vorpalize_weapon()
{
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
  if (get_current_weapon()==NULL || get_current_weapon()->type!=WEAPON) 
    msg(laugh, short_msgs()?"":in_dist);
  else
  {
    //You aren't allowed to doubly vorpalize a weapon.
    if (get_current_weapon()->enemy!=0)
    {
      msg("your %s vanishes in a puff of smoke", get_weapon_name(get_current_weapon()->which));
      detach_item(&player.pack, get_current_weapon());
      discard_item(get_current_weapon());
      set_current_weapon(NULL);
    }
    else
    {
      get_current_weapon()->enemy = pick_monster();
      get_current_weapon()->hit_plus++;
      get_current_weapon()->damage_plus++;
      get_current_weapon()->charges = 1;
      msg(flash, get_weapon_name(get_current_weapon()->which), short_msgs()?"":intense);
    }
  }
}

void(*scroll_functions[MAXSCROLLS])() =
{
  read_monster_confusion,
  read_magic_mapping,
  read_hold_monster,
  read_sleep,
  read_enchant_armor,
  read_identify,
  read_scare_monster,
  read_food_detection,
  read_teleportation,
  read_enchant_weapon,
  read_create_monster,
  read_remove_curse,
  read_aggravate_monsters,
  read_blank_paper,
  read_vorpalize_weapon
};

//read_scroll: Read a scroll from the pack and do the appropriate thing
void read_scroll()
{
  ITEM *scroll;

  scroll = get_item("read", SCROLL);
  if (scroll==NULL) return;
  if (scroll->type!=SCROLL) {msg("there is nothing on it to read"); return;}
  ifterse("the scroll vanishes", "as you read the scroll, it vanishes");
  if (scroll==get_current_weapon()) set_current_weapon(NULL);

  //Call the function for this scroll
  if (scroll->which >= 0 && scroll->which < MAXSCROLLS)
    scroll_functions[scroll->which]();
  else {
    msg("what a puzzling scroll!");
    return;
  }

  look(true); //put the result of the scroll on the screen
  status();
  call_it(s_know[scroll->which], &s_guess[scroll->which]);

  //Get rid of the thing
  if (scroll->count > 1)
    scroll->count--;
  else {
    detach_item(&player.pack, scroll); 
    discard_item(scroll);
  }
}

int is_scare_monster_scroll(ITEM* item)
{
  return item && item->type == SCROLL && 
    item->which == S_SCARE;
}

int is_bad_scroll(ITEM* item)
{
  return item && item->type == SCROLL &&
    (item->which == S_SLEEP || item->which == S_CREATE || item->which == S_AGGR);
}

const char* get_inv_name_scroll(ITEM* obj)
{
  char *pb = prbuf;
  int which = obj->which;

  if (obj->count==1) {
    strcpy(pb, "A scroll ");
    pb = &prbuf[9];
  }
  else {
    sprintf(pb, "%d scrolls ", obj->count); 
    pb = &prbuf[strlen(prbuf)];
  }
  if (does_know_scroll(which) || is_wizard()) 
    sprintf(pb, "of %s", get_scroll_name(which));
  else if (*get_scroll_guess(which)) 
    sprintf(pb, "called %s", get_scroll_guess(which));
  else
    chopmsg(pb, "titled '%.17s'", "titled '%s'", get_title(which));

  return prbuf;
}
