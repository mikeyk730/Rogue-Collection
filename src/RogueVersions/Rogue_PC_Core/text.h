#pragma once
#include "game_state.h"

typedef enum {
    text_defeated_monster,
    text_use,
    text_item_vanishes,
    text_scroll_vanishes,
    text_found_gold,
    text_hungry,
    text_aggravate_monsters,
    text_restore_strength,
    text_remove_armor,
    text_sick,
    text_gain_strength,
    text_was_called,
    text_bite,
    text_remove_curse,
    text_dart_hit,
    text_dart_miss,
    text_found_trap,
    text_wear_armor,
    text_call_it,
    text_enchant_armor,
    text_enchant_weapon,
    text_monster_detect_fail,
    text_magic_detect_fail,
    text_missile_vanishes
} text_id;

const char* get_text(text_id id);