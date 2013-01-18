//setup: Get starting setup for all games
setup();

clock_on();

no_clock();

//flush_type: Flush typeahead for traps, etc.
flush_type();

credits();

//readchar: Return the next input character, from the macro or from the keyboard.
readchar();

bdos(int fnum, int dxval);
isjr();

swint(int intno, struct sw_regs *rp);

set_ctrlb(state);

unsetup();

void one_tick();
