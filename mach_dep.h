//setup: Get starting setup for all games
void setup();

//clear_typeahead_buffer: Flush typeahead for traps, etc.
void clear_typeahead_buffer();

void credits();

int getkey();

//readchar: Return the next input character, from the macro or from the keyboard.
int readchar();

bool is_caps_lock_on();
bool is_scroll_lock_on();
bool is_num_lock_on();
bool is_shift_pressed();

void beep();
int no_char();
void tick_pause();

