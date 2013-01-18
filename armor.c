//This file contains misc functions for dealing with armor
//@(#)armor.c 1.2 (AI Design) 2/12/84

#include "rogue.h"
#include "armor.h"
#include "io.h"
#include "pack.h"
#include "things.h"
#include "daemon.h"

//wear: The player wants to wear something, so let him/her put it on.
void wear()
{
  THING *obj;
  char *sp;

  if (cur_armor!=NULL)
  {
    msg("you are already wearing some%s.", noterse(".  You'll have to take it off first"));
    after = FALSE;
    return;
  }
  if ((obj = get_item("wear", ARMOR))==NULL) return;
  if (obj->o_type!=ARMOR) {msg("you can't wear that"); return;}
  waste_time();
  obj->o_flags |= ISKNOW ;
  sp = inv_name(obj, TRUE);
  cur_armor = obj;
  msg("you are now wearing %s", sp);
}

//take_off: Get the armor off of the player's back
void take_off()
{
  THING *obj;

  if ((obj = cur_armor)==NULL)
  {
    after = FALSE;
    msg("you aren't wearing any armor");
    return;
  }
  if (!can_drop(cur_armor)) return;
  cur_armor = NULL;
  msg("you used to be wearing %c) %s", pack_char(obj), inv_name(obj, TRUE));
}

//waste_time: Do nothing but let other things happen
void waste_time()
{
  do_daemons();
  do_fuses();
}
