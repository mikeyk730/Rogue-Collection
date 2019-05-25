//File for the fun ends. Death or a total win
//rip.c        1.4 (A.I. Design)       12/14/84

#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <cstring>
#ifdef _WIN32
#include <io.h>
#else
#include <unistd.h>
#endif
#include <fcntl.h>
#include <time.h>
#include <sys/stat.h>
#include "random.h"
#include "game_state.h"
#include "rip.h"
#include "output_shim.h"
#include "io.h"
#include "things.h"
#include "misc.h"
#include "monsters.h"
#include "stick.h"
#include "potion.h"
#include "scroll.h"
#include "ring.h"
#include "armor.h"
#include "hero.h"
#include "level.h"
#include "mach_dep.h"

#define TOPSCORES 10

#ifdef __linux__
#define _close close
#define _open open
#define _read read
#define _write write
#define _creat creat
#define _S_IREAD S_IREAD
#define _S_IWRITE S_IWRITE
#define _O_BINARY 0
#define _O_RDWR O_RDWR
#define _O_TRUNC O_TRUNC
#endif

static int sc_fd;

struct LeaderboardEntry
{
    char name[38];
    int rank;
    int gold;
    int fate;
    int level;
};

void get_scores(struct LeaderboardEntry *top10);
void put_scores(struct LeaderboardEntry *top10);
void pr_scores(int newrank, struct LeaderboardEntry *top10);
int add_scores(struct LeaderboardEntry *newscore, struct LeaderboardEntry *oldlist);

int get_year()
{
    time_t rawtime;
    struct tm* timeinfo;

    time(&rawtime);
    timeinfo = localtime(&rawtime);

    return timeinfo->tm_year + 1900;
}

//score: Figure score and post it.
void score(int amount, int flags, char monst)
{
    game->screen().cursor(true);
    struct LeaderboardEntry his_score, top_ten[TOPSCORES];
    int rank = 0;
    //char response = ' ';
    const int LINES = game->screen().lines();

    if (amount || flags || monst)
    {
        std::string filename = game->options.get_environment("autosave_pc");
        if(!filename.empty())
            game->save_game(filename);

        game->screen().move(LINES - 1, 0);
        game->screen().printw("[Press Enter to see rankings]");
        clear_typeahead_buffer();
        wait_for('\r');
        game->screen().move(LINES - 1, 0);
    }

    int count = 0;
    while ((sc_fd = _open(game->options.get_environment("scorefile").c_str(), _O_BINARY)) < 0)
    {
        if (count++ > 0)
            return;
        //game->screen().printw("\n");
        //if (game->wizard().did_cheat() || (amount == 0))
        //    return;
        //str_attr("No scorefile: %Create %Retry %Abort");

        //bool reread;
        //do {
        //    reread = false;
        //    switch (response = readchar())
        //    {
        //    case 'c': case 'C':
                _close(_creat(game->options.get_environment("scorefile").c_str(), _S_IREAD | _S_IWRITE));
        //    case 'r': case 'R':
        //        break;
        //    case 'a': case 'A':
        //        return;
        //    default:
        //        reread = true;
        //    }
        //} while (reread);
    }
    game->screen().printw("\n");
    get_scores(&top_ten[0]);
    if (!game->wizard().did_cheat())
    {
        strcpy(his_score.name, game->hero().get_name().c_str());
        his_score.gold = amount;
        his_score.fate = flags ? flags : monst;
        his_score.level = game->max_level();
        his_score.rank = game->hero().m_stats.m_level;
        rank = add_scores(&his_score, &top_ten[0]);
    }
    _close(sc_fd);
    if (rank > 0)
    {
        sc_fd = _open(game->options.get_environment("scorefile").c_str(), _O_RDWR | _O_TRUNC | _O_BINARY, _S_IREAD | _S_IWRITE);
        if (sc_fd >= 0) {
            put_scores(&top_ten[0]);
            _close(sc_fd);
        }
    }
    pr_scores(rank, &top_ten[0]);
    game->screen().move(LINES - 1, 0);
    game->screen().printw("[Press Enter to quit]");
    clear_typeahead_buffer();
    wait_for('\r');
}

void get_scores(struct LeaderboardEntry *top10)
{
    int i, retcode = 1;

    for (i = 0; i < TOPSCORES; i++, top10++)
    {
        if (retcode > 0)
            retcode = _read(sc_fd, top10, sizeof(struct LeaderboardEntry));
        if (retcode <= 0)
            top10->gold = 0;
    }
}

void put_scores(struct LeaderboardEntry *top10)
{
    int i;

    for (i = 0; (i < TOPSCORES) && top10->gold; i++, top10++)
    {
        if (_write(sc_fd, top10, sizeof(struct LeaderboardEntry)) <= 0) return;
    }
}

void pr_scores(int newrank, struct LeaderboardEntry *top10)
{
    int i;
    int curl;
    char dthstr[30];
    const char *altmsg;

    const int COLS = game->screen().columns();

    game->screen().clear();
    if (game->options.act_like_v1_1()) {
        game->screen().standout();
        game->screen().mvaddstr({ 0, 2 }, "Guildmaster's Hall Of Fame:");
        game->screen().standend();
    }
    else {
        game->screen().high();
        game->screen().mvaddstr({ 0, 0 }, "Guildmaster's Hall Of Fame:");
        game->screen().standend();
        game->screen().yellow();
        game->screen().mvaddstr({ 0, 2 }, "Gold");
    }
    for (i = 0; i < TOPSCORES; i++, top10++)
    {
        altmsg = NULL;
        game->screen().brown();
        if (newrank - 1 == i) {
            game->screen().yellow();
        }
        if (game->options.act_like_v1_1()) {
            game->screen().standend();
        }

        bool change_color((newrank - 1 != i) && !game->options.act_like_v1_1());
        if (top10->gold <= 0) break;
        curl = 4 + ((COLS == 40) ? (i * 2) : i);
        game->screen().move(curl, 0);
        game->screen().printw("%d %s", top10->gold, game->options.act_like_v1_1() ? "gold pieces. " : "");
        if (!game->options.act_like_v1_1())
            game->screen().move(curl, 6);
        if (change_color)
            game->screen().red();
        game->screen().printw("%s", top10->name);
        if (change_color)
            game->screen().brown();
        if (top10->level >= 26)
            altmsg = " Honored by the Guild";
        if (isalpha(top10->fate))
        {
            sprintf(dthstr, " killed by %s", killname((0xff & top10->fate), true));
            if (COLS == 40 && strlen(dthstr) > 23) strcpy(dthstr, " killed");
        }
        else switch (top10->fate)
        {
        case 2:
            strcpy(dthstr, " won");
            altmsg = " A total winner!";
            break;
        case 1:
            strcpy(dthstr, " quit");
            break;
        default:
            strcpy(dthstr, " weirded out");
        }
        if ((strlen(top10->name) + 10 + strlen(level_titles[top10->rank - 1])) < (size_t)COLS)
        {
            if (top10->rank > 1 && (strlen(top10->name)) && !game->options.act_like_v1_1())
                game->screen().printw(" \"%s\"", level_titles[top10->rank - 1]);
        }
        if (COLS == 40)
            game->screen().move(curl + 1, 6);
        if (altmsg == NULL || game->options.act_like_v1_1())
            game->screen().printw("%s on level %d", dthstr, top10->level);
        else
            game->screen().addstr(altmsg);
    }
    game->screen().standend();
    game->screen().addstr(" ");
    if (COLS == 80) game->screen().addstr("\n\n\n\n");
}

int add_scores(struct LeaderboardEntry *newscore, struct LeaderboardEntry *oldlist)
{
    struct LeaderboardEntry *sentry, *insert;
    int retcode = TOPSCORES + 1;

    for (sentry = &oldlist[TOPSCORES - 1]; sentry >= oldlist; sentry--)
    {
        if ((unsigned)newscore->gold > (unsigned)sentry->gold)
        {
            insert = sentry;
            retcode--;
            if ((insert < &oldlist[TOPSCORES - 1]) && sentry->gold) sentry[1] = *sentry;
        }
        else break;
    }
    if (retcode == 11) return 0;
    *insert = *newscore;
    return retcode;
}

//death: Do something really fun when he dies
void death(char monst)
{
    const int COLS = game->screen().columns();
    const int LINES = game->screen().lines();

    char *killer;
    char buf[MAXSTR];

    game->hero().adjust_purse(-(game->hero().get_purse() / 10));

    if (!game->options.act_like_v1_1())
        game->screen().drop_curtain();
    game->screen().clear();
    game->screen().brown();
    game->screen().box((COLS == 40) ? 1 : 7, (COLS - 28) / 2, 22, (COLS + 28) / 2);
    game->screen().standend();
    game->screen().center(10, "R E S T");
    game->screen().center(11, "I N");
    game->screen().center(12, "P E A C E");
    game->screen().red();
    game->screen().center(21, "  *    *      * ");
    game->screen().green();
    game->screen().center(22, "___\\/(\\/)/(\\/ \\\\(//)\\)\\/(//)\\\\)//(\\__");
    game->screen().standend();
    game->screen().center(14, game->hero().get_name().c_str());
    game->screen().standend();
    killer = killname(monst, true);
    strcpy(buf, "killed by");
    game->screen().center(15, buf);
    game->screen().center(16, killer);
    sprintf(buf, "%u Au", game->hero().get_purse());
    game->screen().center(18, buf);
    sprintf(buf, "%u", get_year());
    game->screen().center(19, buf);
    game->screen().raise_curtain();
    game->screen().move(LINES - 1, 0);
    score(game->hero().get_purse(), 0, monst);
    exit_game(0);
}

void print_total_winner()
{
    if (!in_small_screen_mode())
    {
        game->screen().standout();
        game->screen().printw("                                                               \n");
        game->screen().printw("  @   @               @   @           @          @@@  @     @  \n");
        game->screen().printw("  @   @               @@ @@           @           @   @     @  \n");
        game->screen().printw("  @   @  @@@  @   @   @ @ @  @@@   @@@@  @@@      @  @@@    @  \n");
        game->screen().printw("   @@@@ @   @ @   @   @   @     @ @   @ @   @     @   @     @  \n");
        game->screen().printw("      @ @   @ @   @   @   @  @@@@ @   @ @@@@@     @   @     @  \n");
        game->screen().printw("  @   @ @   @ @  @@   @   @ @   @ @   @ @         @   @  @     \n");
        game->screen().printw("   @@@   @@@   @@ @   @   @  @@@@  @@@@  @@@     @@@   @@   @  \n");
    }
    game->screen().printw("                                                               \n");
    game->screen().printw("     Congratulations, you have made it to the light of day!    \n");
    game->screen().standend();
    game->screen().printw("\nYou have joined the elite ranks of those who have escaped the\n");
    game->screen().printw("Dungeons of Doom alive.  You journey home and sell all your loot at\n");
    game->screen().printw("a great profit and are admitted to the fighters guild.\n");
}

void print_total_winner_v11()
{
    game->screen().move(14, 0);
    game->screen().printw("Congratulations!\nYou have made it to the light of day!\n\n\n\n");
    game->screen().printw("You journey home and sell all your\n");
    game->screen().printw("loot at a great profit and are\n");
    game->screen().printw("admitted to the fighters guild.\n\n\n");
}

//total_winner: Code for a winner
void total_winner()
{
    Item *obj;
    int worth;
    byte c = 'a';
    int oldpurse;
    const int LINES = game->screen().lines();

    game->screen().clear();

    if (game->options.act_like_v1_1())
        print_total_winner_v11();
    else
        print_total_winner();

    game->screen().mvaddstr({ 0, LINES - 1 }, "--Press space to continue--");
    wait_for(' ');
    game->screen().clear();
    game->screen().mvaddstr({ 0, 0 }, "   Worth  Item");
    oldpurse = game->hero().get_purse();
    for (auto it = game->hero().m_pack.begin(); it != game->hero().m_pack.end(); c++, ++it)
    {
        obj = *it;
        worth = obj->worth();
        if (worth < 0) worth = 0;
        obj->discover();

        game->screen().move(c - 'a' + 1, 0);
        game->screen().printw("%c) %5d  %s", c, worth, obj->inventory_name(game->hero(), false).c_str());
        game->hero().adjust_purse(worth);
    }
    game->screen().move(c - 'a' + 1, 0);
    game->screen().printw("   %5u  Gold Pieces          ", oldpurse);
    score(game->hero().get_purse(), 2, 0);

    exit_game(0);
}

//killname: Convert a code to a monster name
char *killname(char monst, bool doart)
{
    const char *sp;
    bool article;

    sp = prbuf;
    article = true;
    switch (monst)
    {
    case 'a': sp = "arrow"; break;
    case 'b': sp = "bolt"; break;
    case 'd': sp = "dart"; break;
    case 's': sp = "starvation"; article = false; break;
    case 'f': sp = "fall"; break;
    default:
        if (monst >= 'A' && monst <= 'Z')
            sp = get_monster_name(monst);
        else {
            sp = "God";
            article = false;
        }
        break;
    }
    if (doart && article) sprintf(prbuf, "a%s ", vowelstr(sp));
    else prbuf[0] = '\0';
    strcat(prbuf, sp);
    return prbuf;
}
