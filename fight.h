//fight: The player attacks the monster.
int fight(coord *mp, char mn, THING *weap, bool thrown);

//attack: The monster attacks the player
void attack(THING *mp);

//swing: Returns true if the swing hits
bool swing(int at_lvl, int op_arm, int wplus);

//check_level: Check to see if the guy has gone up a level.
void check_level();

//roll_em: Roll several attacks
bool roll_em(THING *thatt, THING *thdef, THING *weap, bool hurl);

//prname: The print name of a combatant
char *prname(char *who, bool upper);

//hit: Print a message to indicate a successful hit
void hit(char *er, char *ee);

//miss: Print a message to indicate a poor swing
void miss(char *er, char *ee);

//save_throw: See if a creature save against something
int save_throw(int which, THING *tp);

//save: See if he saves against various nasty things
int save(int which);

//str_plus: Compute bonus/penalties for strength on the "to hit" roll
int str_plus(str_t str);

//add_dam: Compute additional damage done for exceptionally high or low strength
int add_dam(str_t str);

//raise_level: The guy just magically went up a level.
void raise_level();

//thunk: A missile hit or missed a monster
void thunk(THING *weap, char *mname, char *does, char *did);

//remove: Remove a monster from the screen
void remove_mons(coord *mp, THING *tp, bool waskill);

//is_magic: Returns true if an object radiates magic
int is_magic(THING *obj);

//killed: Called to put a monster to death
void killed(THING *tp, bool pr);
