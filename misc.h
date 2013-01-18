//tr_name: Print the name of a trap
char *tr_name(byte type);

//look: A quick glance all around the player
look(bool wakeup);

//find_obj: Find the unclaimed object at y, x
THING *find_obj(int y, int x);

//eat: She wants to eat something, so let her try
void eat();

//chg_str: Used to modify the player's strength.  It keeps track of the highest it has been, just in case
void chg_str(int amt);

//add_str: Perform the actual add, checking upper and lower bound
add_str(str_t *sp, int amt);

//add_haste: Add a haste to the player
add_haste(bool potion);

//aggravate: Aggravate all the monsters on this level
aggravate();

//vowelstr: For printfs: if string starts with a vowel, return "n" for an "an".

char *vowelstr(char *str);

//is_current: See if the object is one of the currently used items
is_current(THING *obj);

//get_dir: Set up the direction co_ordinate for use in various "prefix" commands
get_dir();

find_dir(byte ch, coord *cp);

//sign: Return the sign of the number
sign(int nm);

//spread: Give a spread around a given number (+/- 10%)
spread(int nm);

//call_it: Call an object something after use.
call_it(bool know, char **guess);

//step_ok: Returns true if it is ok to step on ch
step_ok(ch);

//goodch: Decide how good an object is and return the correct character for printing.
goodch(THING *obj);

//help: prints out help screens
help(char **helpscr);

DISTANCE(int y1, int x1, int y2, int x2);

_ce(coord *a, coord *b);

INDEX(y,x);

offmap(y,x);

winat(int y, int x);

//search: Player gropes about him to find hidden things.
void search();


//d_level: He wants to go down a level
d_level();

//u_level: He wants to go up a level
u_level();

//call: Allow a user to call a potion, scroll, or ring something
void call();

//prompt player for definition of macro
do_macro(char *buf, int sz);

me();

istest();