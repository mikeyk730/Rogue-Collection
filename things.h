//inv_name: Return the name of something as it would appear in an inventory.

char *inv_name(ITEM *obj, bool drop);

void chopmsg(char *s, char *shmsg, char *lnmsg, ...);

//drop: Put something down
void drop();

//can_drop: Do special checks for dropping or unweilding|unwearing|unringing
int can_drop(ITEM *op);

//new_thing: Return a new thing
ITEM* new_item();

//pick_one: Pick an item out of a list of nitems possible magic items
int pick_one(struct magic_item *magic, int nitems);

//discovered: list what the player has discovered in this game of a certain type
void discovered();

//print_disc: Print what we've discovered of type 'type'
void print_disc(byte type);

//set_order: Set up order for list

void set_order(short *order, int numthings);

//add_line: Add a line to the list of discoveries
int add_line(char *use, char *fmt, char *arg);

//end_line: End the list of lines
int end_line(char *use);

//nothing: Set up prbuf so that message for "nothing found" is there
char *nothing(byte type);