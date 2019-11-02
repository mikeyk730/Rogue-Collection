#pragma once
#include "game_state.h"

typedef enum {
    text_defeated,
    text_use,
} text_id;

const char* get_text(GameState* game, text_id id);