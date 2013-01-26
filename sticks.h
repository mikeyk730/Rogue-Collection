//fix_stick: Set up a new stick
void fix_stick(ITEM *cur);

//do_zap: Perform a zap with a wand
void do_zap();

//drain: Do drain hit points from player schtick
void drain();

//fire_bolt: Fire a bolt in a given direction from a specific starting place
void fire_bolt(coord *start, coord *dir, char *name);

//charge_str: Return an appropriate string for a wand charge
char *charge_str(ITEM *obj);