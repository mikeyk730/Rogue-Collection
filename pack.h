ITEM *pack_obj(byte ch, byte *chp);

//add_pack: Pick up an object and add it to the pack.  If the argument is non-null use it as the linked_list pointer instead of getting it off the ground.
void add_pack(ITEM *obj, bool silent);

//inventory: List what is in the pack
int inventory(ITEM *list, int type, char *lstr);

//pick_up: Add something to characters pack.
void pick_up(byte ch);

//get_item: Pick something out of a pack for a purpose
ITEM *get_item(char *purpose, int type);

//pack_char: Return which character would address a pack object
int pack_char(ITEM *obj);

//money: Add or subtract gold from the pack
void money(int value);

//true if player currently has amulet
int has_amulet();

//true if player ever had amulet
int had_amulet();

ITEM* get_current_weapon();
ITEM* get_current_armor();
ITEM* get_ring(int hand);

void set_current_weapon(ITEM* item);
void set_current_armor(ITEM* item);
void set_ring(int hand, ITEM* item);
