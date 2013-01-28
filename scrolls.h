//init_names: Generate the names of the various scrolls
void init_names();
const char* get_name(int type);

void read_scroll();

int is_scare_monster_scroll(ITEM* item);

int is_bad_scroll(ITEM* item);
int does_know_scroll(int type);
void discover_scroll(int type);
