//File for the fun ends. Death or a total win
//rip.c        1.4 (A.I. Design)       12/14/84

#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <io.h>
#include <fcntl.h>
#include <time.h>
#include <sys/stat.h>

#include "rogue.h"
#include "game_state.h"
#include "rip.h"
#include "output_interface.h"
#include "io.h"
#include "mach_dep.h"
#include "things.h"
#include "misc.h"
#include "monsters.h"
#include "sticks.h"
#include "potions.h"
#include "scrolls.h"
#include "rings.h"
#include "armor.h"
#include "hero.h"
#include "level.h"

#define TOPSCORES 10

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
    struct LeaderboardEntry his_score, top_ten[TOPSCORES];
    int rank = 0;
    char response = ' ';
    const int LINES = game->screen().lines();

    if (amount || flags || monst)
    {
        game->screen().move(LINES - 1, 0);
        game->screen().cursor(true);
        game->screen().printw("[Press Enter to see rankings]");
        clear_typeahead_buffer();
        wait_for('\r');
        game->screen().move(LINES - 1, 0);
    }
    while ((sc_fd = _open(game->get_environment("scorefile").c_str(), 0)) < 0)
    {
        game->screen().printw("\n");
        if (game->wizard().did_cheat() || (amount == 0)) return;
        str_attr("No scorefile: %Create %Retry %Abort");

        bool reread;
        do {
            reread = false;
            switch (response = readchar())
            {
            case 'c': case 'C':
                _close(_creat(game->get_environment("scorefile").c_str(), _S_IREAD | _S_IWRITE));
            case 'r': case 'R': 
                break;
            case 'a': case 'A': 
                return;
            default:
                reread = true;
            }
        } while (reread);
    }
    game->screen().printw("\n");
    get_scores(&top_ten[0]);
    if (!game->wizard().did_cheat())
    {
        strcpy(his_score.name, game->hero().get_name().c_str());
        his_score.gold = amount;
        his_score.fate = flags ? flags : monst;
        his_score.level = max_level();
        his_score.rank = game->hero().stats.level;
        rank = add_scores(&his_score, &top_ten[0]);
    }
    _close(sc_fd);
    if (rank > 0)
    {
        sc_fd = _open(game->get_environment("scorefile").c_str(), _O_RDWR | _O_TRUNC | _O_BINARY, _S_IREAD | _S_IWRITE);
        if (sc_fd >= 0) {
            put_scores(&top_ten[0]);
            _close(sc_fd);
        }
    }
    pr_scores(rank, &top_ten[0]);
    game->screen().printw("[Press Enter to quit]");
    clear_typeahead_buffer();
    wait_for('\r');
}

void get_scores(struct LeaderboardEntry *top10)
{
    int i, retcode = 1;

    for (i = 0; i < TOPSCORES; i++, top10++)
    {
        if (retcode > 0) retcode = _read(sc_fd, top10, sizeof(struct LeaderboardEntry));
        if (retcode <= 0) top10->gold = 0;
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
    char *altmsg;

    const int COLS = game->screen().columns();

    game->screen().clear();
    game->screen().high();
    game->screen().mvaddstr(0, 0, "Guildmaster's Hall Of Fame:");
    game->screen().standend();
    game->screen().yellow();
    game->screen().mvaddstr(2, 0, "Gold");
    for (i = 0; i < TOPSCORES; i++, top10++)
    {
        altmsg = NULL;
        game->screen().brown();
        if (newrank - 1 == i)
        {
            game->screen().yellow();
        }
        if (top10->gold <= 0) break;
        curl = 4 + ((COLS == 40) ? (i * 2) : i);
        game->screen().move(curl, 0);
        game->screen().printw("%d ", top10->gold);
        game->screen().move(curl, 6);
        if (newrank - 1 != i) game->screen().red();
        game->screen().printw("%s", top10->name);
        if ((newrank)-1 != i) game->screen().brown();
        if (top10->level >= 26) altmsg = " Honored by the Guild";
        if (isalpha(top10->fate))
        {
            sprintf(dthstr, " killed by %s", killname((0xff & top10->fate), true));
            if (COLS == 40 && strlen(dthstr) > 23) strcpy(dthstr, " killed");
        }
        else switch (top10->fate)
        {
        case 2: altmsg = " A total winner!"; break;
        case 1: strcpy(dthstr, " quit"); break;
        default: strcpy(dthstr, " weirded out");
        }
        if ((strlen(top10->name) + 10 + strlen(level_titles[top10->rank - 1])) < (size_t)COLS)
        {
            if (top10->rank > 1 && (strlen(top10->name))) game->screen().printw(" \"%s\"", level_titles[top10->rank - 1]);
        }
        if (COLS == 40) game->screen().move(curl + 1, 6);
        if (altmsg == NULL) game->screen().printw("%s on level %d", dthstr, top10->level);
        else game->screen().addstr(altmsg);
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
    exit(0);
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

    game->screen().mvaddstr(LINES - 1, 0, "--Press space to continue--");
    wait_for(' ');
    game->screen().clear();
    game->screen().mvaddstr(0, 0, "   Worth  Item");
    oldpurse = game->hero().get_purse();
    for (auto it = game->hero().pack.begin(); it != game->hero().pack.end(); c++, ++it)
    {
        obj = *it;
        ItemClass* item_class = 0;
        switch (obj->type)
        {
        case FOOD:
            worth = 2 * obj->count;
            break;

        case WEAPON:
            switch (obj->which)
            {
            case MACE: worth = 8; break;
            case SWORD: worth = 15; break;
            case CROSSBOW: worth = 30; break;
            case ARROW: worth = 1; break;
            case DAGGER: worth = 2; break;
            case TWOSWORD: worth = 75; break;
            case DART: worth = 1; break;
            case BOW: worth = 15; break;
            case BOLT: worth = 1; break;
            case SPEAR: worth = 5; break;
            }
            worth *= 3 * (obj->get_hit_plus() + obj->get_damage_plus()) + obj->count;
            obj->set_known();
            break;

        case ARMOR:
            switch (obj->which)
            {
            case LEATHER: worth = 20; break;
            case RING_MAIL: worth = 25; break;
            case STUDDED_LEATHER: worth = 20; break;
            case SCALE_MAIL: worth = 30; break;
            case CHAIN_MAIL: worth = 75; break;
            case SPLINT_MAIL: worth = 80; break;
            case BANDED_MAIL: worth = 90; break;
            case PLATE_MAIL: worth = 150; break;
            }
            worth += (9 - obj->get_armor_class()) * 100;
            worth += (10 * (get_default_class(obj->which) - obj->get_armor_class()));
            obj->set_known();
            break;

        case SCROLL:
            item_class = &game->scrolls();
            worth = item_class->get_value(obj->which);
            worth *= obj->count;
            if (!item_class->is_discovered(obj->which)) worth /= 2;
            item_class->discover(obj->which);
            break;

        case POTION:
            item_class = &game->potions();
            worth = item_class->get_value(obj->which);
            worth *= obj->count;
            if (!item_class->is_discovered(obj->which)) worth /= 2;
            item_class->discover(obj->which);
            break;

        case RING:
            item_class = &game->rings();
            worth = item_class->get_value(obj->which);
            if (obj->which == R_ADDSTR || obj->which == R_ADDDAM || obj->which == R_PROTECT || obj->which == R_ADDHIT)
                if (obj->get_ring_level() > 0) worth += obj->get_ring_level() * 100;
                else worth = 10;
                if (!obj->is_known()) worth /= 2;
                obj->set_known();
                item_class->discover(obj->which);
                break;

        case STICK:
            item_class = &game->sticks();
            worth = item_class->get_value(obj->which);
            worth += 20 * obj->get_charges();
            if (!obj->is_known()) worth /= 2;
            obj->set_known();
            item_class->discover(obj->which);
            break;

        case AMULET:
            worth = 1000;
            break;
        }
        if (worth < 0) worth = 0;
        game->screen().move(c - 'a' + 1, 0);
        game->screen().printw("%c) %5d  %s", c, worth, obj->inventory_name(false).c_str());
        game->hero().adjust_purse(worth);
    }
    game->screen().move(c - 'a' + 1, 0);
    game->screen().printw("   %5u  Gold Pieces          ", oldpurse);
    score(game->hero().get_purse(), 2, 0);


    exit(0);
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
