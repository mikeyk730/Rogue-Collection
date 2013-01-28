//init_stones: Initialize the ring stone setting scheme for this time
void init_stones();
const char* get_stone(int type);
void init_new_ring(ITEM* ring);
int get_ring_value(int type);
const char* get_ring_name(int type);
const char* get_ring_guess(int type);
void set_ring_guess(int type, const char* value);
const char* get_inv_name_ring(ITEM* ring);

//ring_on: Put a ring on a hand
void ring_on();

//ring_off: Take off a ring
void ring_off();

//gethand: Which hand is the hero interested in?
int gethand();

//ring_eat: How much food does this ring use up?
int ring_eat(int hand);

//ring_num: Print ring bonuses
char *ring_num(ITEM *obj);

int is_ring_on_hand(int h, int r);
int is_wearing_ring(int r);
int does_know_ring(int type);
void discover_ring(int type);
