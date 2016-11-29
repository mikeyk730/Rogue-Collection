#pragma once

#define CLEAR_MSG                     msg("")

#ifdef ROGUE_COLLECTION
void __declspec(dllexport) init_game(struct DisplayInterface* screen, struct InputInterface* input, int lines, int cols);
#include <setjmp.h>
extern jmp_buf exception_env;

#define GAME_MAIN                     __declspec(dllexport) rogue_main
#define SHELL_CMD                     msg("You're a long way from a terminal"); after = FALSE
#define NO_SAVE_RETURN                msg("Use Ctrl+S to save your game"); return
//#define MDK_LOG                       printf
#define MDK_LOG(...)
#define EXIT(s)                       longjmp(exception_env,1);
#define EXITABLE(s)                   do { if (!setjmp(exception_env)) { s; } else { return 0; } } while(0)
#define ENDIT(...)
#else
#define GAME_MAIN                     main
#define SHELL_CMD                     shell()
#define NO_SAVE_RETURN
#define MDK_LOG(...)
#define EXIT(s)                       exit(s)
#define EXITABLE(s)                   s
#define ENDIT(s)                      endit(s)
#endif

#ifdef USE_PC_STYLE
void setup_colors();
int translate_type(int t);
int reverse_translate_type(int t);
void fix_tombstone();

#define MORE_MSG                      "-More-"
#define REST_COMMAND                  '.'
#define REST_DESC
#define SPACE_IS_OK                   when ' ' : after = FALSE;
#define ZAP_CASE                      when 'z': case 'p'
#define ARMOR_DISPLAY(ac)             (10-(ac))
#define PC_GFX_SET_CURSOR(enable)     curs_set(enable)
#define PC_GFX_SETUP_COLORS()         setup_colors()
#define PC_GFX_TRANSLATE(ch)          translate_type(ch)
#define PC_GFX_READABLE(ch)           reverse_translate_type(ch)
#define PC_GFX_TOMBSTONE()            fix_tombstone()
#define PC_GFX_WALL_CASES             case LLWALL: case LRWALL: case URWALL: case ULWALL:

#if defined(ROGUE_5_2_1) || defined(ROGUE_5_4_2) || defined(ROGUE_5_3)
#define PC_GFX_PASSGE_COLOR(r,c,ch,cl)   if ((flat(r,c) & F_PASS) && ch != PASSAGE && ch != ' ') PC_GFX_COLOR(cl)
#define PC_GFX_COLOR(c)                  attron(COLOR_PAIR(c))
#define PC_GFX_NOCOLOR(c)                attroff(COLOR_PAIR(c))
#elif defined ROGUE_3_6_3
#define PC_GFX_PASSGE_COLOR(r,c,ch,cl)   if (roomin_rc(r,c) == NULL && ch != PASSAGE && ch != ' ') PC_GFX_COLOR(cw, cl)
#define PC_GFX_COLOR(w,c)                wattron((w), COLOR_PAIR(c))
#define PC_GFX_NOCOLOR(w,c)              wattroff((w), COLOR_PAIR(c))
#endif

#else
#define MORE_MSG                      "--More--"
#define REST_COMMAND                  ' '
#define SPACE_IS_OK
#define REST_DESC                     "(space) "
#define ZAP_CASE                      when 'z' : do_zap(FALSE); when 'p'
#define ARMOR_DISPLAY(ac)             (10-(ac))
#define PC_GFX_SET_CURSOR(enable)
#define PC_GFX_SETUP_COLORS()
#define PC_GFX_TRANSLATE(ch)          (ch)
#define PC_GFX_READABLE(ch)           (ch)
#define PC_GFX_TOMBSTONE()
#define PC_GFX_WALL_CASES
#define PC_GFX_PASSGE_COLOR(...)
#define PC_GFX_COLOR(...)
#define PC_GFX_NOCOLOR(...)
#endif
