//tr_name: Print the name of a trap
char *tr_name(byte type);

//look: A quick glance all around the player
void look(bool wakeup);

//find_obj: Find the unclaimed object at y, x
THING *find_obj(int y, int x);

//eat: She wants to eat something, so let her try
void eat();

//chg_str: Used to modify the player's strength.  It keeps track of the highest it has been, just in case
void chg_str(int amt);

//add_str: Perform the actual add, checking upper and lower bound
void add_str(str_t *sp, int amt);

//add_haste: Add a haste to the player
int add_haste(bool potion);

//aggravate: Aggravate all the monsters on this level
void aggravate();

//vowelstr: For printfs: if string starts with a vowel, return "n" for an "an".

char *vowelstr(char *str);

//is_current: See if the object is one of the currently used items
int is_current(THING *obj);

//get_dir: Set up the direction co_ordinate for use in various "prefix" commands
int get_dir();

bool find_dir(byte ch, coord *cp);

//sign: Return the sign of the number
int sign(int nm);

//spread: Give a spread around a given number (+/- 10%)
int spread(int nm);

//call_it: Call an object something after use.
void call_it(bool know, char **guess);

//step_ok: Returns true if it is ok to step on ch
int step_ok(int ch);

//goodch: Decide how good an object is and return the correct character for printing.
int goodch(THING *obj);

//help: prints out help screens
void help(char **helpscr);

int DISTANCE(int y1, int x1, int y2, int x2);

int _ce(coord *a, coord *b);

int INDEX(int y, int x);

int offmap(int y, int x);

int winat(int y, int x);

//search: Player gropes about him to find hidden things.
void search();


//d_level: He wants to go down a level
void d_level();

//u_level: He wants to go up a level
void u_level();

//call: Allow a user to call a potion, scroll, or ring something
void call();

//prompt player for definition of macro
void do_macro(char *buf, int sz);

int istest();