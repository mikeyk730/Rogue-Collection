/*
 * Rogue definitions and variable declarations
 *
 * @(#)rogue.h	3.38 (Berkeley) 6/15/81
 *
 * Rogue: Exploring the Dungeons of Doom
 * Copyright (C) 1980, 1981 Michael Toy, Ken Arnold and Glenn Wichman
 * All rights reserved.
 *
 * See the file LICENSE.TXT for full copyright and licensing information.
 */

#include "mdport.h"

/*
 * Maximum number of different things
 */
#define MAXROOMS 9
#define MAXTHINGS 9
#define MAXOBJ 9
#define MAXPACK 23
#define MAXTRAPS 10
#define	NUMTHINGS 7	/* number of types of things (scrolls, rings, etc.) */

/*
 * return values for get functions
 */
#define	NORM	0	/* normal exit */
#define	QUIT	1	/* quit option setting */
#define	MINUS	2	/* back up one option */

/*
 * All the fun defines
 */
#define CCHAR(x) ( (x) & A_CHARTEXT )
#define CMVINCH(x,y) CCHAR(mvinch((x),(y)))
#define CMVWINCH(x,y,z) CCHAR(mvwinch((x),(y),(z)))
#define next(ptr) (*ptr).l_next
#define prev(ptr) (*ptr).l_prev
#define ldata(ptr) (*ptr).l_data
#define inroom(rp, cp) (\
    (cp)->x <= (rp)->r_pos.x + ((rp)->r_max.x - 1) && (rp)->r_pos.x <= (cp)->x \
 && (cp)->y <= (rp)->r_pos.y + ((rp)->r_max.y - 1) && (rp)->r_pos.y <= (cp)->y)
#define winat(y, x) (CMVWINCH(mw,y,x)==' '?CMVWINCH(stdscr,y,x):CCHAR(winch(mw)))
#define debug if (wizard) msg
#define RN (((seed = seed*11109+13849) & 0x7fff) >> 1)
#define unc(cp) (cp).y, (cp).x
#define cmov(xy) move((xy).y, (xy).x)
#define DISTANCE(y1, x1, y2, x2) ((x2 - x1)*(x2 - x1) + (y2 - y1)*(y2 - y1))
#define OBJPTR(what)    (struct object *)((*what).l_data)
#define THINGPTR(what)  (struct thing *)((*what).l_data)
#define when break;case
#define otherwise break;default
#define until(expr) while(!(expr))
#define ce(a, b) ((a).x == (b).x && (a).y == (b).y)
#define draw(window) wrefresh(window)
#define hero player.t_pos
#define pstats player.t_stats
#define pack player.t_pack
#define attach(a,b) _attach(&a,b)
#define detach(a,b) _detach(&a,b)
#define free_list(a) _free_list(&a)
#ifndef max
#define max(a, b) ((a) > (b) ? (a) : (b))
#endif
#define on(thing, flag) (((thing).t_flags & flag) != 0)
#define off(thing, flag) (((thing).t_flags & flag) == 0)
#undef  CTRL
#define CTRL(ch) (ch & 037)
#define ALLOC(x) malloc((unsigned int) x)
#define FREE(x) free((char *) x)
#define	EQSTR(a, b, c)	(strncmp(a, b, c) == 0)
#define GOLDCALC (rnd(50 + 10 * level) + 2)
#define ISRING(h,r) (cur_ring[h] != NULL && cur_ring[h]->o_which == r)
#define ISWEARING(r) (ISRING(LEFT, r) || ISRING(RIGHT, r))
#define newgrp() ++group
#define o_charges o_ac
#define ISMULT(type) (type == POTION || type == SCROLL || type == FOOD)
#define ismons(c) ((c)>='A'&&(c)<='Z')

/*
 * Things that appear on the screens
 */
#ifndef USE_PC_STYLE
#define PASSAGE '#'
#define DOOR '+'
#define FLOOR '.'
#define PLAYER '@'
#define TRAP '^'
#define STAIRS '%'
#define GOLD '*'
#define POTION '!'
#define SCROLL '?'
#define MAGIC '$'
#define FOOD ':'
#define WEAPON ')'
#define ARMOR ']'
#define AMULET ','
#define RING '='
#define STICK '/'
#define VWALL '|'
#define HWALL '-'
#define ULWALL	    '-'
#define URWALL	    '-'
#define LLWALL	    '-'
#define LRWALL	    '-'
#else
#include <pc_gfx_charmap.h>
#endif
#define ROGUE_3_6_3
#include "../pc_gfx_macros.h"

#define TRAPDOOR '>'
#define ARROWTRAP '{'
#define SLEEPTRAP '$'
#define BEARTRAP '}'
#define TELTRAP '~'
#define DARTTRAP '`'
#define SECRETDOOR '&'
#define CALLABLE -1

/*
 * Various constants
 */
#define	PASSWD		"mTBellIQOsLNA"
#define BEARTIME 3
#define SLEEPTIME 5
#define HEALTIME 30
#define HOLDTIME 2
#define STPOS 0
#define WANDERTIME 70
#define BEFORE 1
#define AFTER 2
#define HUHDURATION 20
#define SEEDURATION 850
#define HUNGERTIME 1300
#define MORETIME 150
#define STOMACHSIZE 2000
#define ESCAPE 27
#define LEFT 0
#define RIGHT 1
#define BOLT_LENGTH 6

/*
 * Save against things
 */
#define VS_POISON 00
#define VS_PARALYZATION 00
#define VS_DEATH 00
#define VS_PETRIFICATION 01
#define VS_BREATH 02
#define VS_MAGIC  03

/*
 * Various flag bits
 */
#define ISDARK	0000001
#define ISCURSED 000001
#define ISBLIND 0000001
#define ISGONE	0000002
#define ISKNOW  0000002
#define ISRUN	0000004
#define ISFOUND 0000010
#define ISINVIS 0000020
#define ISMEAN  0000040
#define ISGREED 0000100
#define ISBLOCK 0000200
#define ISHELD  0000400
#define ISHUH   0001000
#define ISREGEN 0002000
#define CANHUH  0004000
#define CANSEE  0010000
#define ISMISL  0020000
#define ISCANC	0020000
#define ISMANY  0040000
#define ISSLOW	0040000
#define ISHASTE 0100000

/*
 * Potion types
 */
#define P_CONFUSE 0
#define P_PARALYZE 1
#define P_POISON 2
#define P_STRENGTH 3
#define P_SEEINVIS 4
#define P_HEALING 5
#define P_MFIND 6
#define P_TFIND 7
#define P_RAISE 8
#define P_XHEAL 9
#define P_HASTE 10
#define P_RESTORE 11
#define P_BLIND 12
#define P_NOP 13
#define MAXPOTIONS 14

/*
 * Scroll types
 */
#define S_CONFUSE 0
#define S_MAP 1
#define S_LIGHT 2
#define S_HOLD 3
#define S_SLEEP 4
#define S_ARMOR 5
#define S_IDENT 6
#define S_SCARE 7
#define S_GFIND 8
#define S_TELEP 9
#define S_ENCH 10
#define S_CREATE 11
#define S_REMOVE 12
#define S_AGGR 13
#define S_NOP 14
#define S_GENOCIDE 15
#define MAXSCROLLS 16

/*
 * Weapon types
 */
#define MACE 0
#define SWORD 1
#define BOW 2
#define ARROW 3
#define DAGGER 4
#define ROCK 5
#define TWOSWORD 6
#define SLING 7
#define DART 8
#define CROSSBOW 9
#define BOLT 10
#define SPEAR 11
#define MAXWEAPONS 12

/*
 * Armor types
 */
#define LEATHER 0
#define RING_MAIL 1
#define STUDDED_LEATHER 2
#define SCALE_MAIL 3
#define CHAIN_MAIL 4
#define SPLINT_MAIL 5
#define BANDED_MAIL 6
#define PLATE_MAIL 7
#define MAXARMORS 8

/*
 * Ring types
 */
#define R_PROTECT 0
#define R_ADDSTR 1
#define R_SUSTSTR 2
#define R_SEARCH 3
#define R_SEEINVIS 4
#define R_NOP 5
#define R_AGGR 6
#define R_ADDHIT 7
#define R_ADDDAM 8
#define R_REGEN 9
#define R_DIGEST 10
#define R_TELEPORT 11
#define R_STEALTH 12
#define MAXRINGS 13

/*
 * Rod/Wand/Staff types
 */

#define WS_LIGHT 0
#define WS_HIT 1
#define WS_ELECT 2
#define WS_FIRE 3
#define WS_COLD 4
#define WS_POLYMORPH 5
#define WS_MISSILE 6
#define WS_HASTE_M 7
#define WS_SLOW_M 8
#define WS_DRAIN 9
#define WS_NOP 10
#define WS_TELAWAY 11
#define WS_TELTO 12
#define WS_CANCEL 13
#define MAXSTICKS 14

/*
 * Now we define the structures and types
 */

/*
 * Help list
 */

struct h_list {
    int h_ch;
    char *h_desc;
};

extern struct h_list helpstr[];

/*
 * Coordinate data type
 */
typedef struct {
    int x;
    int y;
} coord;

typedef struct {
    int st_str;
    int st_add;
} str_t;

/*
 * Linked list data type
 */
struct linked_list {
    struct linked_list *l_next;
    struct linked_list *l_prev;
    char *l_data;			/* Various structure pointers */
};

/*
 * Stuff about magic items
 */

struct magic_item {
    char mi_name[30];
    int mi_prob;
    int mi_worth;
};

/*
 * Room structure
 */
struct room {
    coord r_pos;			/* Upper left corner */
    coord r_max;			/* Size of room */
    coord r_gold;			/* Where the gold is */
    int r_goldval;			/* How much the gold is worth */
    int r_flags;			/* Info about the room */
    int r_nexits;			/* Number of exits */
    coord r_exit[4];			/* Where the exits are */
};

/*
 * Array of all traps on this level
 */
struct trap {
    coord tr_pos;			/* Where trap is */
    int tr_type;			/* What kind of trap */
    int tr_flags;			/* Info about trap (i.e. ISFOUND) */
};

extern struct trap  traps[MAXTRAPS];

/*
 * Structure describing a fighting being
 */
struct stats {
    str_t s_str;			/* Strength */
    int s_exp;				/* Experience */
    int s_lvl;				/* Level of mastery */
    int s_arm;				/* Armor class */
    int s_hpt;				/* Hit points */
    char s_dmg[30];			/* String describing damage done */
};

/*
 * Structure for monsters and player
 */
struct thing {
    coord t_pos;			/* Position */
    int t_turn;			/* If slowed, is it a turn to move */
    int t_type;			/* What it is */
    int t_disguise;			/* What mimic looks like */
    int t_oldch;			/* Character that was where it was */
    coord *t_dest;			/* Where it is running to */
    int t_flags;			/* State word */
    struct stats t_stats;		/* Physical description */
    struct linked_list *t_pack;		/* What the thing is carrying */
	int t_reserved;         /* reserved for save/restore code */
};

/*
 * Array containing information on all the various types of mosnters
 */
struct monster {
    char m_name[20];			/* What to call the monster */
    int m_carry;			/* Probability of carrying something */
    int m_flags;			/* Things about the monster */
    struct stats m_stats;		/* Initial stats */
};

/*
 * Structure for a thing that the rogue can carry
 */

struct object {
    int o_type;				/* What kind of object it is */
    coord o_pos;			/* Where it lives on the screen */
    int o_launch;			/* What you need to launch it */
    char o_damage[8];			/* Damage if used like sword */
    char o_hurldmg[8];			/* Damage if thrown */
    int o_count;			/* Count for plural objects */
    int o_which;			/* Which object of a type it is */
    int o_hplus;			/* Plusses to hit */
    int o_dplus;			/* Plusses to damage */
    int o_ac;				/* Armor class */
    int o_flags;			/* Information about objects */
    int o_group;			/* Group number for this object */
};

struct delayed_action {
    int d_type;
    void (*d_func)();
    int d_arg;
    int d_time;
};

/*
 * Now all the global variables
 */

extern int                   a_chances[MAXARMORS];	/* Probabilities for armor */
extern int                   a_class[MAXARMORS];	/* Armor class for various armors */
extern char *                a_names[MAXARMORS];	/* Names of armor types */
extern int                   after;			/* True if we want after daemons */
extern int                   amulet;			/* He found the amulet */
extern int                   askme;			/* Ask about unidentified things */
extern int                   showac;			/* Show armor class instead of protection */
extern int                   between;
extern coord                 ch_ret;
extern int                   count;			/* Number of times to repeat command */
extern int                   cNCOLORS;
extern int                   cNMETAL;
extern int                   cNSTONES;
extern int                   cNWOOD;
extern struct object *       cur_armor;			/* What a well dresssed rogue wears */
extern struct object *       cur_ring[2];		/* Which rings are being worn */
extern struct object *       cur_weapon;		/* Which weapon he is weilding */
extern WINDOW *              cw;			/* Window that the player sees */
extern coord                 delta;			/* Change indicated to get_dir() */
extern int                   dnum;			/* Dungeon number */
extern int                   door_stop;			/* Stop running when we pass a door */
extern struct delayed_action d_list[20];
extern int                   fight_flush;		/* True if toilet input */
extern char                  file_name[80];		/* Save file name */
extern int                   firstmove;			/* First move after setting door_stop */
extern int                   food_left;			/* Amount of food in hero's stomach */
extern char                  fruit[80];			/* Favorite fruit */
extern int                   fung_hit;			/* Number of time fungi has hit */
extern int                   group;			/* Current group number */
extern char                  home[];			/* User's home directory */
extern WINDOW *              hw;			/* Used for the help command */
extern char                  huh[80];			/* The last message printed */
extern int                   hungry_state;		/* How hungry is he */
extern int                   in_shell;			/* True if executing a shell */
extern int                   inpack;			/* Number of things in pack */
extern int                   jump;			/* Show running as series of jumps */
extern int                   lastscore;			/* Score before this turn */
extern int                   level;			/* What level rogue is on */
extern char                  lvl_mons[27];
extern struct linked_list *  lvl_obj;			/* List of objects on this level */
extern int                   max_hp;			/* Player's max hit points */
extern int                   max_level;			/* Deepest player has gone */
extern struct stats          max_stats;			/* The maximum for the player */
extern char *                metal[];
extern struct linked_list *  mlist;			/* List of monsters on the level */
extern struct monster        monsters[26];		/* The initial monster states */
extern int                   mpos;			/* Where cursor is on top line */
extern WINDOW *              mw;			/* Used to store mosnters */
extern coord                 nh;
extern int                   no_command;		/* Number of turns asleep */
extern int                   no_food;			/* Number of levels without food */
extern int                   no_move;			/* Number of turns held in place */
extern int                   notify;			/* True if player wants to know */
extern int                   ntraps;			/* Number of traps on this level */
extern int                   num_checks;
extern coord                 oldpos;			/* Position before last look() call */
extern struct room *         oldrp;			/* Roomin(&oldpos) */
extern char *                p_colors[MAXPOTIONS];	/* Colors of the potions */
extern char *                p_guess[MAXPOTIONS];	/* Players guess at what potion is */
extern int                   p_know[MAXPOTIONS];	/* Does he know what a potion does */
extern struct magic_item     p_magic[MAXPOTIONS];	/* Names and chances for potions */
extern struct thing          player;			/* The rogue */
extern int                   playing;			/* True until he quits */
extern unsigned char         prbuf[80];			/* Buffer for sprintfs */
extern int                   purse;			/* How much gold the rogue has */
extern int                   quiet;			/* Number of quiet turns */
extern char *                r_guess[MAXRINGS];		/* Players guess at what ring is */
extern int                   r_know[MAXRINGS];		/* Does he know what a ring does */
extern struct magic_item     r_magic[MAXRINGS];		/* Names and chances for rings */
extern char *                r_stones[MAXRINGS];	/* Stone settings of the rings */
extern char *                rainbow[];
extern char *                release;			/* Release number of rogue */
extern struct room           rooms[MAXROOMS];		/* One for each room -- A level */
extern int                   runch;			/* Direction player is running */
extern int                   running;			/* True if player is running */
extern int                   seed;			/* Random number seed */
extern char *                s_guess[MAXSCROLLS];	/* Players guess at what scroll is */
extern int                   s_know[MAXSCROLLS];	/* Does he know what a scroll does */
extern struct magic_item     s_magic[MAXSCROLLS];	/* Names and chances for scrolls */
extern char *                s_names[MAXSCROLLS];	/* Names of the scrolls */
extern FILE *                scoreboard;
extern int                   slow_invent;		/* Inventory one line at a time */
extern char *                stones[];
extern int                   take;			/* Thing the rogue is taking */
extern int                   terse;			/* True if we should be int */
extern struct magic_item     things[NUMTHINGS];		/* Chances for each type of item */
extern int                   total;			/* Total dynamic memory bytes */
extern char *                w_names[MAXWEAPONS];	/* Names of the various weapons */
extern char                  wand_mons[27];
extern int                   waswizard;			/* Was a wizard sometime */
extern char                  whoami[80];		/* Name of player */
extern int                   wizard;			/* True if allows wizard commands */
extern char *                wood[];
extern char *                ws_guess[MAXSTICKS];	/* Players guess at what wand is */
extern int                   ws_know[MAXSTICKS];	/* Does he know what a stick does */
extern char *                ws_made[MAXSTICKS];	/* What sticks are made of */
extern struct magic_item     ws_magic[MAXSTICKS];	/* Names and chances for sticks */
extern char *                ws_type[MAXSTICKS];	/* Is it a wand or a staff */

void                    _attach(struct linked_list **list, struct linked_list *item);
void                    _detach(struct linked_list **list, struct linked_list *item);
void                    _free_list(struct linked_list **ptr);
char *                  _new(size_t size);
int                     add_dam(str_t *str);
void                    add_haste(int potion);
void                    add_pack(struct linked_list *item, int silent);
void                    add_pass(void);
void                    addmsg(char *fmt, ...);
void                    aggravate(void);
int                     attack(struct thing *mp);
int                     author(void);
void                    auto_save(int p);
void                    badcheck(char *name, struct magic_item *magic, int bound);
int                     be_trapped(coord *tc);
void                    bounce(struct object *weap, char *mname);
void                    call(void);
int                     cansee(int y, int x);
char *                  charge_str(struct object *obj);
int                     chase(struct thing *tp, coord *ee);
void                    check_level(void);
void                    checkout(int p);
void                    chg_str(int amt);
void                    chmsg(char *fmt, ...);
void                    command(void);
void                    conn(int r1, int r2);
void                    create_obj(void);
void                    death(int monst);
int                     diag_ok(coord *sp, coord *ep);
void                    discard(struct linked_list *item);
void                    d_level(void);
int                     do_chase(struct thing *th);
void                    do_daemons(int flag);
void                    do_fuses(int flag);
void                    do_motion(struct object *obj, int ydelta, int xdelta);
void                    do_move(int dy, int dx);
void                    do_passages(void);
void                    do_rooms(void);
void                    do_run(int ch);
void                    do_zap(int gotdir);
void                    doadd(char *fmt, va_list ap);
void                    doctor(void);
void                    door(struct room *rm, coord *cp);
void                    drain(int ymin, int ymax, int xmin, int xmax);
void                    draw_room(struct room *rp);
void                    drop(void);
int                     dropcheck(struct object *op);
void                    eat(void);
int	                encerror(void);
void                    encseterr(int err);
int                     encclearerr(void);
size_t                  encread(void *buf, size_t size, FILE *inf);
size_t                  encwrite(const void *buf, size_t size, FILE *outf);
void                    endmsg(void);
void                    extinguish(void (*func)());
void                    fall(struct linked_list *item, int pr);
int                     fallpos(coord *pos, coord *newpos, int passages);
void                    fatal(char *s);
struct linked_list *    find_mons(int y, int x);
struct linked_list *    find_obj(int y, int x);
struct delayed_action * find_slot(void (*func)());
int                     fight(coord *mp, int mn, struct object *weap, int thrown);
void                    fix_stick(struct object *cur);
void                    flush_type(void);
void                    fuse(void (*func)(), int arg, int time, int type);
void                    genocide(void);
int                     get_bool(void *vp, WINDOW *win);
int                     get_dir(void);
struct linked_list *    get_item(char *purpose, int type);
int                     get_str(void *vopt, WINDOW *win);
int                     gethand(void);
void                    help(void);
void                    hit(char *er, char *ee);
int                     hit_monster(int y, int x, struct object *obj);
void                    horiz(int cnt, int is_top);
void                    identify(void);
void                    init_colors(void);
void                    init_materials(void);
void                    init_names(void);
void                    init_player(void);
void                    init_stones(void);
void                    init_things(void);
void                    init_weapon(struct object *weap, int type);
char *                  inv_name(struct object *obj, int drop);
int                     inventory(struct linked_list *list, int type);
int                     is_current(struct object *obj);
int                     is_magic(struct object *obj);
void                    kill_daemon(void (*func)());
void                    killed(struct linked_list *item, int pr);
char *                  killname(int monst);
void                    lengthen(void (*func)(), int xtime);
void                    light(coord *cp);
void                    look(int wakeup);
void                    miss(char *er, char *ee);
void                    missile(int ydelta, int xdelta);
void                    money(void);
void                    msg(char *fmt, ...);
void                    new_level(void);
struct linked_list *    new_item(int size);
void                    new_monster(struct linked_list *item, int type, coord *cp);
struct linked_list *    new_thing(void);
char *                  num(int n1, int n2);
void                    nohaste(void);
void                    open_score(void);
void                    option(void);
int                     pack_char(struct object *obj);
void                    parse_opts(char *str);
int                     passwd(void);
int                     pick_one(struct magic_item *magic, int nitems);
void                    pick_up(int ch);
void                    picky_inven(void);
void                    playit(void);
void                    put_bool(void *b);
void                    put_str(void *str);
void                    put_things(void);
int                     readchar(WINDOW *win);
int                     restore(char *file, char **envp);
int                     roll(int number, int sides);
struct room *           roomin(coord *cp);
int                     step_ok(int ch);
void                    strucpy(char *s1, char *s2, size_t len);
void                    swander(void);
void                    take_off(void);
void                    tstp(int p);
void                    quaff(void);
void                    quit(int p);
void                    raise_level(void);
int                     randmonster(int wander);
void                    read_scroll(void);
void                    remove_monster(coord *mp, struct linked_list *item);
int                     ring_eat(int hand);
void                    ring_off(void);
void                    ring_on(void);
char *                  ring_num(struct object *obj);
int                     rnd(int range);
void                    rnd_pos(struct room *rp, coord *cp);
int                     rnd_room(void);
coord *                 rndmove(struct thing *who);
int                     roll_em(struct stats *att, struct stats *def, struct object *weap, int hurl);
void                    rollwand(void);
int                     rs_save_file(FILE *savef);
int                     rs_restore_file(FILE *inf);
void                    runners(void);
void                    runto(coord *runner, coord *spot);
int                     save(int which);
int                     save_file(FILE *savef);
int                     save_game(void);
int                     save_throw(int which, struct thing *tp);
void                    score(int amount, int flags, int monst);
void                    shell(void);
int                     show(int y, int x);
void                    sight(void);
void                    search(void);
int                     secretdoor(int y, int x);
void                    setup(void);
void                    show_win(WINDOW *scr, char *message);
void                    start_daemon(void (*func)(), int arg, int type);
void                    status(void);
void                    stomach(void);
int                     str_plus(str_t *str);
int                     swing(int at_lvl, int op_arm, int wplus);
int                     teleport(void);
int                     too_much(void);
void                    total_winner(void);
char *                  tr_name(int ch);
struct trap *           trap_at(int y, int x);
void                    thunk(struct object *weap, char *mname);
void                    u_level(void);
void                    unconfuse(void);
void                    unsee(void);
void                    vert(int cnt);
char *                  vowelstr(char *str);
void                    wait_for(WINDOW *win, int ch);
struct linked_list *    wake_monster(int y, int x);
void                    wanderer(void);
void                    waste_time(void);
void                    wear(void);
void                    whatis(void);
void                    wield(void);

extern FILE* g_game_log;
