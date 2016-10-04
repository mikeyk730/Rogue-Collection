//Various input/output functions
//io.c         1.4             (A.I. Design) 12/10/84
#include <ctime>
#include <chrono>
#include <sstream>
#include <stdio.h>
#include <ctype.h>
#include <stdarg.h>

#include "rogue.h"
#include "game_state.h"
#include "io.h"
#include "output_interface.h"
#include "misc.h"
#include "mach_dep.h"
#include "strings.h"
#include "command.h"
#include "rings.h"
#include "hero.h"
#include "level.h"
#include "pack.h"
#include "agent.h"

#define PT(i,j)  ((COLS==40)?i:j)

char msgbuf[BUFSIZE];

static int newpos = 0;
static char *formats = "scud%", *bp, left_justify;
static int min_width, max_width;
static char ibuf[6];

bool terse = false;
bool expert = false;

bool short_msgs()
{
    return in_small_screen_mode() || in_brief_mode();
}

void set_small_screen_mode(bool enable)
{
    terse = enable;
}

bool in_small_screen_mode()
{
    return terse;
}

void set_brief_mode(bool enable)
{
    expert = enable;
}

bool in_brief_mode()
{
    return expert;
}

//msg: Display a message at the top of the screen.
void ifterse(const char *tfmt, const char *format, ...)
{
    char dest[1024 * 16];
    va_list argptr;
    va_start(argptr, format);
    vsprintf(dest, expert ? tfmt : format, argptr);
    va_end(argptr);
    msg(dest);
}

void reset_msg_position()
{
    game->msg_position = 0;
}

void msg(const char *format, ...)
{
    //if the string is "", just clear the line
    if (*format == '\0') { 
        game->screen().move(0, 0); 
        game->screen().clrtoeol(); 
        reset_msg_position();
        return;
    }

    char dest[1024 * 16];
    va_list argptr;
    va_start(argptr, format);
    vsprintf(dest, format, argptr);
    va_end(argptr);

    //otherwise add to the message and flush it out
    doadd(dest);
    endmsg();
}

void unsaved_msg(const char * format, ...)
{
    //if the string is "", just clear the line
    if (*format == '\0') {
        game->screen().move(0, 0);
        game->screen().clrtoeol();
        reset_msg_position();
        return;
    }

    char dest[1024 * 16];
    va_list argptr;
    va_start(argptr, format);
    vsprintf(dest, format, argptr);
    va_end(argptr);

    //otherwise add to the message and flush it out
    doadd(dest);
    endmsg();
    reset_msg_position();
}

void clear_msg()
{
    msg("");
}

//addmsg: Add things to the current message
void addmsg(const char *format, ...)
{
    char dest[1024 * 16];
    va_list argptr;
    va_start(argptr, format);
    vsprintf(dest, format, argptr);
    va_end(argptr);

    doadd(dest);
}

//endmsg: Display a new msg (giving him a chance to see the previous one if it is up there with the -More-)
void endmsg()
{
    game->log("msg", msgbuf);
    strcpy(game->last_message, msgbuf);
    if (game->msg_position) {
        look(false); 
        game->screen().move(0, game->msg_position);
        more(" More "); 
    }
    //All messages should start with uppercase, except ones that start with a pack addressing character
    if (islower(msgbuf[0]) && msgbuf[1] != ')') 
        msgbuf[0] = toupper(msgbuf[0]);
    putmsg(0, msgbuf);
    game->msg_position = newpos;
    newpos = 0;
}

//More: tag the end of a line and wait for a space
void more(const char *msg)
{
    int x, y;
    int i, msz;
    char mbuf[80];
    int morethere = true;
    int covered = false;
    const int COLS = game->screen().columns();

    msz = strlen(msg);
    game->screen().getrc(&x, &y);
    //it is reasonable to assume that if the you are no longer on line 0, you must have wrapped.
    if (x != 0) { x = 0; y = COLS; }
    if ((y + msz) > COLS) { game->screen().move(x, y = COLS - msz); covered = true; }
    for (i = 0; i < msz; i++)
    {
        mbuf[i] = game->screen().curch();
        if ((i + y) < (COLS - 2)) game->screen().move(x, y + i + 1);
        mbuf[i + 1] = 0;
    }
    game->screen().move(x, y);
    game->screen().standout();
    game->screen().addstr(msg);
    game->screen().standend();
    while (readchar() != ' ')
    {
        if (covered && morethere) { game->screen().move(x, y); game->screen().addstr(mbuf); morethere = false; }
        else if (covered) { game->screen().move(x, y); game->screen().standout(); game->screen().addstr(msg); game->screen().standend(); morethere = true; }
    }
    game->screen().move(x, y);
    game->screen().addstr(mbuf);
}

//doadd: Perform an add onto the message buffer
void doadd(char *format, ...)
{
    va_list argptr;
    va_start(argptr, format);
    vsprintf(&msgbuf[newpos], format, argptr);
    va_end(argptr);

    newpos = strlen(msgbuf);
}

//putmsg: put a msg on the line, make sure that it will fit, if it won't scroll msg sideways until he has read it all
void putmsg(int msgline, const char *msg)
{
    const int COLS = game->screen().columns();
    const char *curmsg, *lastmsg = 0, *tmpmsg;
    int curlen;

    curmsg = msg;
    do
    {
        scrl(msgline, lastmsg, curmsg);
        newpos = curlen = strlen(curmsg);
        if (curlen > COLS)
        {
            more(" Cont ");
            lastmsg = curmsg;
            do
            {
                tmpmsg = stpbrk(curmsg, " ");
                //If there are no blanks in line
                if ((tmpmsg == 0 || tmpmsg >= &lastmsg[COLS]) && lastmsg == curmsg) { curmsg = &lastmsg[COLS]; break; }
                if ((tmpmsg >= (lastmsg + COLS)) || (strlen(curmsg) < (size_t)COLS)) break;
                curmsg = tmpmsg + 1;
            } while (1);
        }
    } while (curlen > COLS);
}

//scrl: scroll a message across the line
void scrl(int msgline, const char *str1, const char *str2)
{
    const int COLS = game->screen().columns();
    char *fmt;

    if (COLS > 40) fmt = "%.80s"; else fmt = "%.40s";
    if (str1 == 0)
    {
        game->screen().move(msgline, 0);
        if (strlen(str2) < (size_t)COLS) game->screen().clrtoeol();
        game->screen().printw(fmt, str2);
    }
    else while (str1 <= str2)
    {
        game->screen().move(msgline, 0);
        game->screen().printw(fmt, str1++);
        if (strlen(str1) < (size_t)(COLS - 1)) game->screen().clrtoeol();
    }
}

//unctrl: Print a readable version of a certain character
char *unctrl(unsigned char ch)
{
    static char chstr[9]; //Defined in curses library

    if (isspace(ch))
        strcpy(chstr, " ");
    else if (!isprint(ch))
        if (ch < ' ')
            sprintf(chstr, "^%c", ch + '@');
        else
            sprintf(chstr, "\\x%x", ch);
    else {
        chstr[0] = ch;
        chstr[1] = 0;
    }
    return chstr;
}

//update_status_bar: Display the important stats line.  Keep the cursor where it was.
void update_status_bar()
{
    int oy, ox;
    static int s_hungry;
    static int s_level, s_pur = -1, s_hp;
    static int s_elvl = 0;
    static char *state_name[] = { "      ", "Hungry", "Weak", "Faint", "?" };

    const int COLS = game->screen().columns();

    handle_key_state();
    game->screen().getrc(&oy, &ox);
    game->screen().yellow();
    //Level:
    if (s_level != game->get_level())
    {
        s_level = game->get_level();
        game->screen().move(PT(22, 23), 0);
        game->screen().printw("Level:%-4d", game->get_level());
    }
    //Hits:
    if (s_hp != game->hero().get_hp())
    {
        s_hp = game->hero().get_hp();
        game->screen().move(PT(22, 23), 12);
        if (game->hero().get_hp() < 100) {
            game->screen().printw("Hits:%2d(%2d) ", game->hero().get_hp(), game->hero().m_stats.m_max_hp);
            //just in case they get wraithed with 3 digit max hits
            game->screen().addstr("  ");
        }
        else
            game->screen().printw("Hits:%3d(%3d) ", game->hero().get_hp(), game->hero().m_stats.m_max_hp);
    }

    //Str:
    game->screen().move(PT(22, 23), 26);
    game->screen().printw("Str:%2d(%2d) ", game->hero().calculate_strength(), game->hero().calculate_max_strength());

    //Gold
    if (s_pur != game->hero().get_purse())
    {
        s_pur = game->hero().get_purse();
        game->screen().move(23, PT(0, 40));
        game->screen().printw("Gold:%-5u", game->hero().get_purse());
    }
    //Armor:
    game->screen().move(23, PT(12, 52));
    game->screen().printw("Armor:%-2d", game->hero().armor_for_display());

    //Exp:
    if (!game->options.use_exp_level_names())
    {
        game->screen().move(23, PT(22, 62));
        game->screen().printw("Exp:%d/%d", game->hero().m_stats.m_level, game->hero().experience());
    }
    else if (s_elvl != game->hero().m_stats.m_level)
    {
        s_elvl = game->hero().m_stats.m_level;
        game->screen().move(23, PT(22, 62));
        game->screen().printw("%-12s", level_titles[s_elvl - 1]);
    }
    //Show raw food counter in wizard mode
    if (game->wizard().show_food_counter()) {
        s_hungry = game->hero().get_food_left();
        std::ostringstream ss;
        ss << s_hungry;
        game->screen().bold();
        game->screen().move(24, PT(28, 58));
        game->screen().addstr(state_name[0]);
        game->screen().move(24, PT(28, 58));
        game->screen().addstr(ss.str().c_str());
        game->screen().standend();
    }
    //Hungry state
    else if (s_hungry != game->hero().get_hungry_state())
    {
        s_hungry = game->hero().get_hungry_state();
        game->screen().move(24, PT(28, 58));
        game->screen().addstr(state_name[0]);
        game->screen().move(24, PT(28, 58));
        if (game->hero().get_hungry_state()) {
            game->screen().bold();
            game->screen().addstr(state_name[game->hero().get_hungry_state()]);
            game->screen().standend();
        }
    }
    game->screen().standend();
    game->screen().move(oy, ox);
}

//wait_for: Sit around until the guy types the right key
void wait_for(char ch)
{
    char c;

    if (ch == '\n') while ((c = readchar()) != '\n' && c != '\r') continue;
    else while (readchar() != ch) continue;
}

//show_win: Function used to display a window and wait before returning
void show_win(char *message)
{
    game->screen().mvaddstr({ 0, 0 }, message);
    game->screen().move(game->hero().position().y, game->hero().position().x);
    wait_for(' ');
}

//This routine reads information from the keyboard. It should do all the strange processing that is needed to retrieve sensible data from the user
int getinfo_impl(char *str, int size)
{
    char *retstr, ch;
    int readcnt = 0;
    bool wason;
    int ret = 1;

    retstr = str;
    *str = 0;
    wason = game->screen().cursor(true);
    while (ret == 1) switch (ch = getkey())
    {
    case ESCAPE:
        while (str != retstr) { backspace(); readcnt--; str--; }
        ret = *str = ESCAPE;
        str[1] = 0;
        game->screen().cursor(wason);
        break;
    case '\b':
        if (str != retstr) { backspace(); readcnt--; str--; }
        break;
    default:
        if (readcnt >= size) { beep(); break; }
        readcnt++;
        game->screen().addch(ch);
        *str++ = ch;
        if ((ch & 0x80) == 0) break;
    case '\n':
    case '\r':
        *str = 0;
        game->screen().cursor(wason);
        ret = ch;
        break;
    }
    return ret;
}

void backspace()
{
    int x, y;
    game->screen().getrc(&x, &y);
    if (--y < 0) y = 0;
    game->screen().move(x, y);
    game->screen().addch(' ');
    game->screen().move(x, y);
}

//str_attr: format a string with attributes.
//
//    formats:
//        %i - the following character is turned inverse vidio
//        %I - All characters upto %$ or null are turned inverse vidio
//        %u - the following character is underlined
//        %U - All characters upto %$ or null are underlined
//        %$ - Turn off all attributes
//
//     Attributes do not nest, therefore turning on an attribute while
//     a different one is in effect simply changes the attribute.
//
//     "No attribute" is the default and is set on leaving this routine
//
//     Eventually this routine will contain colors and character intensity
//     attributes.  And I'm not sure how I'm going to interface this with
//     printf certainly '%' isn't a good choice of characters.  jll.

void str_attr(char *str)
{
    while (*str)
    {
        if (*str == '%') { str++; game->screen().standout(); }
        game->screen().addch(*str++);
        game->screen().standend();
    }
}

void handle_key_state()
{
    static bool numl = false, capsl = false, scrl = false;
    static int nspot, cspot, tspot;
    bool num_lock_on = is_num_lock_on(),
        caps_lock_on = is_caps_lock_on(),
        scroll_lock_on = is_scroll_lock_on();
    static int bighand, littlehand;
    int showtime = false;

    const int COLS = game->screen().columns();
    const int LINES = game->screen().lines();

    if (COLS == 40) {
        nspot = 10;
        cspot = 19;
        tspot = 35;
    }
    else {
        nspot = 20;
        cspot = 39;
        tspot = 75;
    }

    if (!game->in_replay() && game->fast_play() != scroll_lock_on)
    {
        game->set_fast_play(scroll_lock_on);
        game->cancel_repeating_cmd();
        game->stop_run_cmd();
    }

    if ( scrl != game->fast_play()) {
        scrl = game->fast_play();
        game->screen().move(LINES - 1, 0);
        if (game->fast_play()) {
            game->screen().bold();
            game->screen().addstr("Fast Play");
            game->screen().standend();
        }
        else game->screen().addstr("         ");
    }

    if (numl != num_lock_on)
    {
        numl = num_lock_on;
        game->cancel_repeating_cmd();
        //show_count();
        game->stop_run_cmd();
        game->screen().move(LINES - 1, nspot);
        if (numl) {
            game->screen().bold();
            game->screen().addstr("NUM LOCK");
            game->screen().standend();
        }
        else game->screen().addstr("        ");
    }
    if (capsl != caps_lock_on)
    {
        capsl = caps_lock_on;
        game->screen().move(LINES - 1, cspot);
        if (capsl) {
            game->screen().bold();
            game->screen().addstr("CAP LOCK");
            game->screen().standend();
        }
        else game->screen().addstr("        ");
    }
    if (showtime)
    {
        auto now = std::chrono::system_clock::now();
        auto time = std::chrono::system_clock::to_time_t(now);
        tm local_time = *localtime(&time);

        bighand = local_time.tm_hour % 12;
        if (bighand == 0)
            bighand = 12;
        littlehand = local_time.tm_min;

        game->screen().move(24, tspot);
        game->screen().bold();
        game->screen().printw("%2d:%2d", bighand, littlehand);
        game->screen().standend();
    }
}

char *noterse(char *str)
{
    return (short_msgs() ? "" : str);
}
