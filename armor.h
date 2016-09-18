int get_default_class(int type);
void init_new_armor(Item* armor);
const char* get_inv_name_armor(Item* armor);

const char* get_armor_name(int type);

//wear: The player wants to wear something, so let him/her put it on.
void wear();

//take_off: Get the armor off of the player's back
void take_off();

//waste_time: Do nothing but let other things happen
void waste_time();