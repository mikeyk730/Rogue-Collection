//setup: Get starting setup for all games
void setup();

//flush_type: Flush typeahead for traps, etc.
void flush_type();

void credits();

int getkey();

//readchar: Return the next input character, from the macro or from the keyboard.
int readchar();

bool is_caps_lock_on();
bool is_scroll_lock_on();
bool is_num_lock_on();

void beep();
void _halt();
int no_char();
