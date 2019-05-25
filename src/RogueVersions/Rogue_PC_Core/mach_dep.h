#pragma once

struct ExitGame {};

void sound_beep();
void go_to_sleep(int ms);
void exit_game(int status);

bool is_caps_lock_on();
bool is_num_lock_on();
bool is_scroll_lock_on();
