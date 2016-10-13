#pragma once
#ifdef USE_PC_GFX
void setup_colors();
int translate_type(int t);

#define PC_GFX_SET_CURSOR(b)          curs_set(b)
#define PC_GFX_SETUP_COLORS()         setup_colors()
#define PC_GFX_WALL_CASES             case LLWALL: case LRWALL: case URWALL: case ULWALL:
#if defined(ROGUE_5_2_1) || defined(ROGUE_5_4_2)
#define PC_GFX_PASSGE_STANDOUT(r,c,ch)   if ((flat(r,c) & F_PASS) && ch != PASSAGE) PC_GFX_STANDOUT()
#define PC_GFX_STANDOUT()             attron(COLOR_PAIR(0x70))
#define PC_GFX_STANDEND()             attroff(COLOR_PAIR(0x70))
#else ifdef ROGUE_3_6_3
#define PC_GFX_PASSGE_STANDOUT(r,c,ch)   if (roomin_rc(r,c) == NULL && ch != PASSAGE && ch != ' ') PC_GFX_STANDOUT()
#define PC_GFX_STANDOUT()             wattron(cw, COLOR_PAIR(0x70))
#define PC_GFX_STANDEND()             wattroff(cw, COLOR_PAIR(0x70))
#define PC_GFX_TRANSLATE(x)           translate_type(x)
#endif
#define MORE_MSG                      " More "
#else
#define PC_GFX_SET_CURSOR(b)
#define PC_GFX_SETUP_COLORS()
#define PC_GFX_WALL_CASES
#define PC_GFX_PASSGE_STANDOUT(r,c)
#define PC_GFX_STANDOUT()
#define PC_GFX_STANDEND() 
#define PC_GFX_TRANSLATE(x)           (x)
#define MORE_MSG             "--More--"
#endif
