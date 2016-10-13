#ifdef USE_PC_GFX
#include "pc_gfx_charmap.h"
#include "pc_gfx_macros.h"
#include <curses.h>
void init_game(struct DisplayInterface* screen, struct InputInterface* input)
{
    init_curses(screen, input);
}

void setup_colors()
{
#define init_color_pair(attr) init_pair((attr), (attr) & 0x0f, ((attr) & 0xf0) >> 4)
    start_color();
    init_color(0x00, 0, 0, 0); //black
    init_color(0x01, 0, 0, 667); //blue
    init_color(0x02, 0, 667, 0); //green
    init_color(0x03, 0, 667, 667); //cyan
    init_color(0x04, 667, 0, 0); //red
    init_color(0x05, 667, 0, 667); //magenta
    init_color(0x06, 667, 333, 0); //brown
    init_color(0x07, 667, 667, 667); //grey
    init_color(0x08, 250, 250, 250); //d_grey
    init_color(0x09, 333, 333, 1000); //l_blue
    init_color(0x0a, 333, 1000, 333); //l_green
    init_color(0x0b, 100, 1000, 1000); //l_cyan
    init_color(0x0c, 1000, 333, 333); //l_red
    init_color(0x0d, 1000, 100, 1000); //l_magenta
    init_color(0x0e, 1000, 1000, 100); //yellow
    init_color(0x0f, 1000, 1000, 1000); //white
    init_color_pair(0x01);
    init_color_pair(0x02);
    init_color_pair(0x04);
    init_color_pair(0x05);
    init_color_pair(0x06);
    init_color_pair(0x07);
    init_color_pair(0x09);
    init_color_pair(0x0A);
    init_color_pair(0x0D);
    init_color_pair(0x0E);
    init_color_pair(0x70);
    init_color_pair(0x71);
    init_color_pair(0x72);
    init_color_pair(0x74);
    init_color_pair(0x78);
    init_color_pair(0x7E);
    init_color_pair(0xA0);
    curs_set(0);
}

int translate_type(int t)
{
    switch (t) {
    case '*': return GOLD;
    case '!': return POTION;
    case '?': return SCROLL;
    case '$': return MAGIC;
    case ':': return FOOD;
    case ')': return WEAPON;
    case ']': return ARMOR;
    case ',': return AMULET;
    case '=': return RING;
    case '/': return STICK;
    }
    return t;
}
#endif