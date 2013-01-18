//init_player: Roll up the rogue
init_player();

//init_things: Initialize the probabilities for types of things
init_things();

//init_colors: Initialize the potion color scheme for this time
init_colors();

//init_names: Generate the names of the various scrolls
init_names();

//getsyl(): generate a random syllable
char* getsyl();

//rchr(): return random character in given string
rchr(char *string);

//init_stones: Initialize the ring stone setting scheme for this time
init_stones();

//init_materials: Initialize the construction materials for wands and staffs
init_materials();

//init_ds(): Allocate things data space
init_ds();