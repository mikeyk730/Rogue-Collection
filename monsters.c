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
void new_monster(THING *tp, byte type, coord *cp)
{
  struct monster *mp;
  int lev_add;

  if ((lev_add = level-AMULETLEVEL)<0) lev_add = 0;
  attach(mlist, tp);
  tp->t_type = type;
  tp->t_disguise = type;
  bcopy(tp->t_pos, *cp);
  tp->t_oldch = '@';
  tp->t_room = roomin(cp);
  mp = &monsters[tp->t_type-'A'];
  tp->t_stats.s_lvl = mp->m_stats.s_lvl+lev_add;
  tp->t_stats.s_maxhp = tp->t_stats.s_hpt = roll(tp->t_stats.s_lvl, 8);
  tp->t_stats.s_arm = mp->m_stats.s_arm-lev_add;
  tp->t_stats.s_dmg = mp->m_stats.s_dmg;
  tp->t_stats.s_str = mp->m_stats.s_str;
  tp->t_stats.s_exp = mp->m_stats.s_exp+lev_add*10+exp_add(tp);
  tp->t_flags = mp->m_flags;
  tp->t_turn = TRUE;
  tp->t_pack = NULL;
  if (ISWEARING(R_AGGR)) start_run(cp);
  if (type=='F') tp->t_stats.s_dmg = f_damage;
  if (type=='X') switch (rnd(level>25?9:8))
  {
  case 0: tp->t_disguise = GOLD; break;
  case 1: tp->t_disguise = POTION; break;
  case 2: tp->t_disguise = SCROLL; break;
  case 3: tp->t_disguise = STAIRS; break;
  case 4: tp->t_disguise = WEAPON; break;
  case 5: tp->t_disguise = ARMOR; break;
  case 6: tp->t_disguise = RING; break;
  case 7: tp->t_disguise = STICK; break;
  case 8: tp->t_disguise = AMULET; break;
  }
}

//f_restor(): restor initial damage string for flytraps
void f_restor()
{
  struct monster *mp = &monsters['F'-'A'];

  fung_hit = 0;
  strcpy(f_damage, mp->m_stats.s_dmg);
}

//expadd: Experience to add for this monster's level/hit points
int exp_add(THING *tp)
{
  int mod;

  if (tp->t_stats.s_lvl==1) mod = tp->t_stats.s_maxhp/8;
  else mod = tp->t_stats.s_maxhp/6;
  if (tp->t_stats.s_lvl>9) mod *= 20;
  else if (tp->t_stats.s_lvl>6) mod *= 4;
  return mod;
}

//wanderer: Create a new wandering monster and aim it at the player
void wanderer()
{
  int i;
  struct room *rp;
  THING *tp;
  coord cp;

  //can we allocate a new monster
  if ((tp = new_item())==NULL) return;
  do
  {
    i = rnd_room();
    if ((rp = &rooms[i])==proom) continue;
    rnd_pos(rp, &cp);
  } while (!(rp!=proom && step_ok(winat(cp.y, cp.x))));
  new_monster(tp, randmonster(TRUE), &cp);
  if (bailout) debug("wanderer bailout");
  //debug("started a wandering %s", monsters[tp->t_type-'A'].m_name);
  start_run(&tp->t_pos);
}

//wake_monster: What to do when the hero steps next to a monster
THING *wake_monster(int y, int x)
{
  THING *tp;
  struct room *rp;
  byte ch;
  int dst;

  if ((tp = moat(y, x))==NULL) return tp;
  ch = tp->t_type;
  //Every time he sees mean monster, it might start chasing him
  if (!on(*tp, ISRUN) && rnd(3)!=0 && on(*tp, ISMEAN) && !on(*tp, ISHELD) && !ISWEARING(R_STEALTH))
  {
    tp->t_dest = &hero;
    tp->t_flags |= ISRUN;
  }
  if (ch=='M' && !on(player, ISBLIND) && !on(*tp, ISFOUND) && !on(*tp, ISCANC) && on(*tp, ISRUN))
  {
    rp = proom;
    dst = DISTANCE(y, x, hero.y, hero.x);
    if ((rp!=NULL && !(rp->r_flags&ISDARK)) || dst<LAMP_DIST)
    {
      tp->t_flags |= ISFOUND;
      if (!save(VS_MAGIC))
      {
        if (on(player, ISHUH)) lengthen(unconfuse, rnd(20)+HUH_DURATION);
        else fuse(unconfuse, 0, rnd(20)+HUH_DURATION);
        player.t_flags |= ISHUH;
        msg("the medusa's gaze has confused you");
      }
    }
  }
  //Let greedy ones guard gold
  if (on(*tp, ISGREED) && !on(*tp, ISRUN))
  {
    tp->t_flags = tp->t_flags|ISRUN;
    if (proom->r_goldval) tp->t_dest = &proom->r_gold;
    else tp->t_dest = &hero;
  }
  return tp;
}

//give_pack: Give a pack to a monster if it deserves one
void give_pack(THING *tp)
{
  //check if we can allocate a new item
  if (total<MAXITEMS && rnd(100)<monsters[tp->t_type-'A'].m_carry) attach(tp->t_pack, new_thing());
}

//pick_mons: Choose a sort of monster for the enemy of a vorpally enchanted weapon
char pick_mons()
{
  char *cp = lvl_mons+strlen(lvl_mons);

  while (--cp>=lvl_mons && rnd(10)) ;
  if (cp<lvl_mons) return 'M';
  return *cp;
}

//moat(x,y): returns pointer to monster at coordinate. if no monster there return NULL
THING *moat(int my, int mx)
{
  THING *tp;

  for (tp = mlist; tp!=NULL; tp = next(tp)) if (tp->t_pos.x==mx && tp->t_pos.y==my) return (tp);
  return (NULL);
}
