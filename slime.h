#pragma once

struct Monster;

void slime_split(Monster* monster);

int new_slime(Monster* monster);

bool plop_monster(int r, int c, Coord *cp);