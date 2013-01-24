//setup: Get starting setup for all games
void setup();

//flush_type: Flush typeahead for traps, etc.
void flush_type();

void credits();

int getkey();

//readchar: Return the next input character, from the macro or from the keyboard.
int readchar();

int is_caps_lock_on();
int is_scroll_lock_on();
int is_num_lock_on();
int is_shift_pressed();

void beep();
int no_char();
