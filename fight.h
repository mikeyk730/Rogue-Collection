//fight: The player attacks the monster.
int fight(Coord *mp, char mn, ITEM *weap, bool thrown);

//attack: The monster attacks the player
void attack(AGENT *mp);

//swing: Returns true if the swing hits
bool swing(int at_lvl, int op_arm, int wplus);

//check_level: Check to see if the guy has gone up a level.
void check_level();

//roll_em: Roll several attacks
bool roll_em(AGENT *thatt, AGENT *thdef, ITEM *weap, bool hurl);

//prname: The print name of a combatant
char *prname(const char *who, bool upper);

//hit: Print a message to indicate a successful hit
void display_hit_msg(const char *er, const char *ee);

//miss: Print a message to indicate a poor swing
void miss(const char *er, const char *ee);

//save_throw: See if a creature save against something
int save_throw(int which, AGENT *tp);

//save: See if he saves against various nasty things
int save(int which);

//str_plus: Compute bonus/penalties for strength on the "to hit" roll
int str_plus(unsigned int str);

//add_dam: Compute additional damage done for exceptionally high or low strength
int add_dam(unsigned int str);

//raise_level: The guy just magically went up a level.
void raise_level();

//thunk: A missile hit or missed a monster
void display_throw_msg(ITEM *weap, const char *mname, char *does, char *did);

//remove: Remove a monster from the screen
void remove_mons(Coord *mp, AGENT *tp, bool waskill);

//is_magic: Returns true if an object radiates magic
int is_magic(ITEM *obj);

//killed: Called to put a monster to death
void killed(AGENT *tp, bool pr);
