//ring_on: Put a ring on a hand
void ring_on();

//ring_off: Take off a ring
void ring_off();

//gethand: Which hand is the hero interested in?
int gethand();

//ring_eat: How much food does this ring use up?
int ring_eat(int hand);

//ring_num: Print ring bonuses
char *ring_num(THING *obj);

int is_ring_on_hand(int h, int r);
int is_wearing_ring(int r);
