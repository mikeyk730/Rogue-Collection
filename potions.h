//quaff: Quaff a potion from the pack
void quaff();

//invis_on: Turn on the ability to see invisible
void invis_on();

//turn_see: Put on or off seeing monsters on this level
bool turn_see(bool turn_off);

//th_effect: Compute the effect of this potion hitting a monster.
void th_effect(ITEM *obj, AGENT *tp);

int is_bad_potion(ITEM* obj);
int does_know_potion(int type);
void discover_potion(int type);
