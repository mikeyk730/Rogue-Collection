#pragma once
#ifdef USE_PC_GFX
#define PC_GFX_SET_CURSOR(b)          curs_set(b)
#define PC_GFX_SETUP_COLORS()         setup_colors()
#define PC_GFX_WALL_CASES             case LLWALL: case LRWALL: case URWALL: case ULWALL:
#ifdef ROGUE_5_4_2
#define PC_GFX_PASSGE_STANDOUT(r,c)   if (flat(r,c) & F_PASS) attron(COLOR_PAIR(0x70))
#endif
#define PC_GFX_STANDOUT()             attron(COLOR_PAIR(0x70))
#define PC_GFX_STANDEND()             attroff(COLOR_PAIR(0x70))
#define MORE_MSG                      " More "
#else
#define PC_GFX_SET_CURSOR(b)
#define PC_GFX_SETUP_COLORS()
#define PC_GFX_WALL_CASES
#define PC_GFX_PASSGE_STANDOUT(r,c)
#define PC_GFX_STANDOUT()
#define PC_GFX_STANDEND() 
#define MORE_MSG             "--More--"
#endif
