//init_names: Generate the names of the various scrolls
void init_names();
const char* get_title(int type);
void init_new_scroll(ITEM* scroll);
int get_scroll_value(int type);
const char* get_scroll_name(int type);
const char* get_scroll_guess(int type);
void set_scroll_guess(int type, const char* value);

void read_scroll();

int is_scare_monster_scroll(ITEM* item);

int is_bad_scroll(ITEM* item);
int does_know_scroll(int type);
void discover_scroll(int type);
