const char* get_weapon_name(int which);
void init_new_weapon(ITEM* weapon);
const char* get_inv_name_weapon(ITEM* weapon);

//missile: Fire a missile in a given direction
void missile(int ydelta, int xdelta);

//do_motion: Do the actual motion on the screen done by an object travelling across the room
void do_motion(ITEM *obj, int ydelta, int xdelta);

const char *short_name(ITEM *obj);

//fall: Drop an item someplace around here.
void fall(ITEM *obj, bool pr);

//init_weapon: Set up the initial goodies for a weapon
void init_weapon(ITEM *weapon, byte type);

//hit_monster: Does the missile hit the monster?
int hit_monster(int y, int x, ITEM *obj);

//num: Figure out the plus number for armor/weapons
char *num(int n1, int n2, char type);

//wield: Pull out a certain weapon
void wield();

//fallpos: Pick a random position around the given (y, x) coordinates
int fallpos(ITEM *obj, Coord *newpos);

