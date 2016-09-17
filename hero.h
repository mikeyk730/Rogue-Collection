//init_player: Roll up the rogue
void init_player();

const char* get_name();
void set_name(const char* name);

int get_purse();
void adjust_purse(int delta);

void ingest();
void digest();
int get_hungry_state();
int get_food_left();

void set_wizard(int enable);
int is_wizard();
int did_cheat();
