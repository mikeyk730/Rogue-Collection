THING *pack_obj(byte ch, byte *chp);

//add_pack: Pick up an object and add it to the pack.  If the argument is non-null use it as the linked_list pointer instead of getting it off the ground.
void add_pack(THING *obj, bool silent);

//inventory: List what is in the pack
int inventory(THING *list, int type, char *lstr);

//pick_up: Add something to characters pack.
void pick_up(byte ch);

//get_item: Pick something out of a pack for a purpose
THING *get_item(char *purpose, int type);

//pack_char: Return which character would address a pack object
int pack_char(THING *obj);

//money: Add or subtract gold from the pack
void money(int value);
