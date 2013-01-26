//File with various monster functions in it
//monsters.c  1.4 (A.I. Design)       12/14/84

#include "rogue.h"
#include "monsters.h"
#include "daemons.h"
#include "list.h"
#include "main.h"
#include "chase.h"
#include "new_leve.h"
#include "rooms.h"
#include "things.h"
#include "io.h"
#include "misc.h"
#include "daemon.h"
#include "fight.h"
#include "rings.h"
#include "thing.h"

//List of monsters in rough order of vorpalness
static char *lvl_mons = "K BHISOR LCA NYTWFP GMXVJD";
static char *wand_mons = "KEBHISORZ CAQ YTW PUGM VJ ";

//randmonster: Pick a monster to show up.  The lower the level, the meaner the monster.
char randmonster(bool wander)
{
  int d;
  char *mons;

  mons = wander?wand_mons:lvl_mons;
  do
  {
    int r10 = rnd(5)+rnd(6);

    d = level+(r10-5);
    if (d<1) d = rnd(5)+1;
    if (d>26) d = rnd(5)+22;
  } while (mons[--d]==' ');
  return mons[d];
}

//new_monster: Pick a new monster and add it to the list
void new_monster(AGENT *tp, byte type, Coord *cp)
{
  struct Monster *mp;
  int lev_add;

  if ((lev_add = level-AMULETLEVEL)<0) lev_add = 0;
  attach_agent(&mlist, tp);
  tp->type = type;
  tp->disguise = type;
  bcopy(tp->pos, *cp);
  tp->oldch = '@';
  tp->room = roomin(cp);
  mp = &monsters[tp->type-'A'];
  tp->stats.level = mp->m_stats.level+lev_add;
  tp->stats.max_hp = tp->stats.hp = roll(tp->stats.level, 8);
  tp->stats.armor_class = mp->m_stats.armor_class-lev_add;
  tp->stats.damage = mp->m_stats.damage;
  tp->stats.str = mp->m_stats.str;
  tp->stats.exp = mp->m_stats.exp+lev_add*10+exp_add(tp);
  tp->flags = mp->m_flags;
  tp->turn = TRUE;
  tp->pack = NULL;
  if (is_wearing_ring(R_AGGR)) start_run(cp);
  if (type=='F') tp->stats.damage = f_damage;
  if (type=='X') switch (rnd(level>25?9:8))
  {
  case 0: tp->disguise = GOLD; break;
  case 1: tp->disguise = POTION; break;
  case 2: tp->disguise = SCROLL; break;
  case 3: tp->disguise = STAIRS; break;
  case 4: tp->disguise = WEAPON; break;
  case 5: tp->disguise = ARMOR; break;
  case 6: tp->disguise = RING; break;
  case 7: tp->disguise = STICK; break;
  case 8: tp->disguise = AMULET; break;
  }
}

//f_restor(): restor initial damage string for flytraps
void f_restor()
{
  struct Monster *mp = &monsters['F'-'A'];

  flytrap_hit = 0;
  strcpy(f_damage, mp->m_stats.damage);
}

//expadd: Experience to add for this monster's level/hit points
int exp_add(AGENT *tp)
{
  int mod;

  if (tp->stats.level==1) mod = tp->stats.max_hp/8;
  else mod = tp->stats.max_hp/6;
  if (tp->stats.level>9) mod *= 20;
  else if (tp->stats.level>6) mod *= 4;
  return mod;
}

//wanderer: Create a new wandering monster and aim it at the player
void wanderer()
{
  int i;
  struct Room *rp;
  AGENT *tp;
  Coord cp;

  //can we allocate a new monster
  if ((tp = create_agent())==NULL) return;
  do
  {
    i = rnd_room();
    if ((rp = &rooms[i])==player.room) continue;
    rnd_pos(rp, &cp);
  } while (!(rp!=player.room && step_ok(display_character(cp.y, cp.x))));
  new_monster(tp, randmonster(TRUE), &cp);
  if (bailout) debug("wanderer bailout");
  //debug("started a wandering %s", monsters[tp->type-'A'].m_name);
  start_run(&tp->pos);
}

//wake_monster: What to do when the hero steps next to a monster
AGENT *wake_monster(int y, int x)
{
  AGENT *tp;
  struct Room *rp;
  byte ch;
  int dst;

  if ((tp = monster_at(y, x))==NULL) return tp;
  ch = tp->type;
  //Every time he sees mean monster, it might start chasing him
  if (!on(*tp, ISRUN) && rnd(3)!=0 && on(*tp, ISMEAN) && !on(*tp, ISHELD) && !is_wearing_ring(R_STEALTH))
  {
    tp->dest = &player.pos;
    tp->flags |= ISRUN;
  }
  if (ch=='M' && !on(player, ISBLIND) && !on(*tp, ISFOUND) && !on(*tp, ISCANC) && on(*tp, ISRUN))
  {
    rp = player.room;
    dst = DISTANCE(y, x, player.pos.y, player.pos.x);
    if ((rp!=NULL && !(rp->flags&ISDARK)) || dst<LAMP_DIST)
    {
      tp->flags |= ISFOUND;
      if (!save(VS_MAGIC))
      {
        if (on(player, ISHUH)) lengthen(unconfuse, rnd(20)+HUH_DURATION);
        else fuse(unconfuse, 0, rnd(20)+HUH_DURATION);
        player.flags |= ISHUH;
        msg("the medusa's gaze has confused you");
      }
    }
  }
  //Let greedy ones guard gold
  if (on(*tp, ISGREED) && !on(*tp, ISRUN))
  {
    tp->flags = tp->flags|ISRUN;
    if (player.room->goldval) tp->dest = &player.room->gold;
    else tp->dest = &player.pos;
  }
  return tp;
}

//give_pack: Give a pack to a monster if it deserves one
void give_pack(AGENT *tp)
{
  //check if we can allocate a new item
  if (total_items<MAXITEMS && rnd(100)<monsters[tp->type-'A'].m_carry) 
    attach_item(&tp->pack, new_item());
}

//pick_mons: Choose a sort of monster for the enemy of a vorpally enchanted weapon
char pick_monster()
{
  char *cp = lvl_mons+strlen(lvl_mons);

  while (--cp>=lvl_mons && rnd(10)) ;
  if (cp<lvl_mons) return 'M';
  return *cp;
}

//moat(x,y): returns pointer to monster at coordinate. if no monster there return NULL
AGENT *monster_at(int my, int mx)
{
  AGENT *tp;

  for (tp = mlist; tp!=NULL; tp = next(tp)) if (tp->pos.x==mx && tp->pos.y==my) return (tp);
  return (NULL);
}
