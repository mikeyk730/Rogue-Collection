#pragma once

#ifdef ROGUE_COLLECTION
void __declspec(dllexport) init_game(struct DisplayInterface* screen, struct InputInterface* input, int lines, int cols);
#define GAME_MAIN                     __declspec(dllexport) rogue_main
#define SHELL_CMD                     msg("You're a long way from a terminal"); after = FALSE
#define NO_SAVE_RETURN                msg("Use Ctrl+S to save your game"); return
#else
#define GAME_MAIN                     main
#define SHELL_CMD                     shell()
#define NO_SAVE_RETURN
#endif

#ifdef USE_PC_STYLE
void setup_colors();
int translate_type(int t);
#define PC_GFX_SET_CURSOR(b)          curs_set(b)
#define PC_GFX_SETUP_COLORS()         setup_colors()
#define PC_GFX_WALL_CASES             case LLWALL: case LRWALL: case URWALL: case ULWALL:
#define PC_GFX_OR_WALLS               || ch == LLWALL || ch == LRWALL || ch == URWALL || ch == ULWALL
#define PC_GFX_TRANSLATE(x)           translate_type(x)
#define PC_GFX_READABLE(x)            reverse_translate_type(x)
#if defined(ROGUE_5_2_1) || defined(ROGUE_5_4_2)
#define PC_GFX_PASSGE_COLOR(r,c,ch,cl)   if ((flat(r,c) & F_PASS) && ch != PASSAGE) PC_GFX_COLOR(cl)
#define PC_GFX_COLOR(c)               attron(COLOR_PAIR(c))
#define PC_GFX_NOCOLOR(c)             attroff(COLOR_PAIR(c))
#elif defined ROGUE_3_6_3
#define PC_GFX_PASSGE_COLOR(r,c,ch,cl)   if (roomin_rc(r,c) == NULL && ch != PASSAGE && ch != ' ') PC_GFX_COLOR(cl)
#define PC_GFX_COLOR(c)               wattron(cw, COLOR_PAIR(c))
#define PC_GFX_NOCOLOR(c)             wattroff(cw, COLOR_PAIR(c))
#define PC_GFX_COLOR_STD(c)           attron(COLOR_PAIR(c))
#define PC_GFX_NOCOLOR_STD(c)         attroff(COLOR_PAIR(c))
#endif
#define PC_GFX_TOMBSTONE()            fix_tombstone()
#define MORE_MSG                      "--More--"
#define REST_COMMAND                  '.'
#define REST_DESC
#define SPACE_IS_OK                   when ' ' : after = FALSE;
#define CLEAR_MSG                     msg("")
#define ZAP_CASE                      when 'z': case 'p'
#define ARMOR_DISPLAY_MOD             11
//#define MDK_LOG                       printf
#define MDK_LOG(...)
#else
#define PC_GFX_SET_CURSOR(b)
#define PC_GFX_SETUP_COLORS()
#define PC_GFX_WALL_CASES
#define PC_GFX_OR_WALLS
#define PC_GFX_TRANSLATE(x)           (x)
#define PC_GFX_READABLE(x)            (x)
#define PC_GFX_PASSGE_COLOR(r,c)
#define PC_GFX_COLOR(c)
#define PC_GFX_NOCOLOR(c)
#define PC_GFX_COLOR_STD(c)
#define PC_GFX_NOCOLOR_STD(c)
#define PC_GFX_TOMBSTONE()
#define MORE_MSG                      "--More--"
#define REST_COMMAND                  ' '
#define SPACE_IS_OK
#define CLEAR_MSG
#define REST_DESC                     "(space) "
#define ZAP_CASE                      when 'z' : do_zap(FALSE); when 'p'
#define ARMOR_DISPLAY_MOD             10
#define MDK_LOG(...)
#endif
