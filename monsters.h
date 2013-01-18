//randmonster: Pick a monster to show up.  The lower the level, the meaner the monster.
char randmonster(bool wander);

//new_monster: Pick a new monster and add it to the list
void new_monster(THING *tp, byte type, coord *cp);

//f_restor(): restor initial damage string for flytraps
void f_restor();

//expadd: Experience to add for this monster's level/hit points
int exp_add(THING *tp);

//wanderer: Create a new wandering monster and aim it at the player
void wanderer();

//wake_monster: What to do when the hero steps next to a monster
THING *wake_monster(int y, int x);

//give_pack: Give a pack to a monster if it deserves one
void give_pack(THING *tp);

//pick_mons: Choose a sort of monster for the enemy of a vorpally enchanted weapon
char pick_mons();

//moat(x,y): returns pointer to monster at coordinate. if no monster there return NULL
THING *moat(int my, int mx);
