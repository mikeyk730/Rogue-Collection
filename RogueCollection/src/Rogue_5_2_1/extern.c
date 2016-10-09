/*
 * global variable initializaton
 *
 * @(#)extern.c	4.32 (Berkeley) 4/1/82
 *
 * Rogue: Exploring the Dungeons of Doom
 * Copyright (C) 1980, 1981, 1982 Michael Toy, Ken Arnold and Glenn Wichman
 * All rights reserved.
 *
 * See the file LICENSE.TXT for full copyright and licensing information.
 */

#include <curses.h>
#include "rogue.h"

bool after;				/* True if we want after daemons */
bool noscore;				/* Was a wizard sometime */
bool s_know[MAXSCROLLS];		/* Does he know what a scroll does */
bool p_know[MAXPOTIONS];		/* Does he know what a potion does */
bool r_know[MAXRINGS];			/* Does he know what a ring does */
bool ws_know[MAXSTICKS];		/* Does he know what a stick does */
bool amulet = FALSE;			/* He found the amulet */
bool askme = FALSE;			/* Ask about unidentified things */
bool door_stop = FALSE;			/* Stop running when we pass a door */
bool fight_flush = FALSE;		/* True if toilet input */
bool firstmove = FALSE;			/* First move after setting door_stop */
bool in_shell = FALSE;			/* True if executing a shell */
bool jump = FALSE;			/* Show running as series of jumps */
bool passgo = FALSE;			/* Follow passages */
bool playing = TRUE;			/* True until he quits */
bool running = FALSE;			/* True if player is running */
bool save_msg = TRUE;			/* Remember last msg */
bool slow_invent = FALSE;		/* Inventory one line at a time */
bool terse = FALSE;			/* True if we should be short */
#ifdef WIZARD
bool wizard = FALSE;			/* True if allows wizard commands */
#endif

char take;				/* Thing the rogue is taking */
char prbuf[MAXSTR];			/* Buffer for sprintfs */
char outbuf[BUFSIZ];			/* Output buffer for stdout */
char runch;				/* Direction player is running */
char *s_names[MAXSCROLLS];		/* Names of the scrolls */
const char *p_colors[MAXPOTIONS];		/* Colors of the potions */
const char *r_stones[MAXRINGS];		/* Stone settings of the rings */
const char *w_names[MAXWEAPONS + 1] = {	/* Names of the various weapons */
    "mace",
    "long sword",
    "short bow",
    "arrow",
    "dagger",
    "two handed sword",
    "dart",
    "crossbow",
    "crossbow bolt",
    "spear",
    NULL				/* fake entry for dragon's breath */
};
const char *a_names[MAXARMORS] = {		/* Names of armor types */
    "leather armor",
    "ring mail",
    "studded leather armor",
    "scale mail",
    "chain mail",
    "splint mail",
    "banded mail",
    "plate mail",
};
const char *ws_made[MAXSTICKS];		/* What sticks are made of */
char *release;				/* Release number of rogue */
char whoami[MAXSTR];			/* Name of player */
char fruit[MAXSTR];			/* Favorite fruit */
char huh[MAXSTR];			/* The last message printed */
char *s_guess[MAXSCROLLS];		/* Players guess at what scroll is */
char *p_guess[MAXPOTIONS];		/* Players guess at what potion is */
char *r_guess[MAXRINGS];		/* Players guess at what ring is */
char *ws_guess[MAXSTICKS];		/* Players guess at what wand is */
char *ws_type[MAXSTICKS];		/* Is it a wand or a staff */
char file_name[MAXSTR];			/* Save file name */
char home[MAXSTR];			/* User's home directory */
char _level[MAXLINES*MAXCOLS];		/* Level map */
char _flags[MAXLINES*MAXCOLS];		/* Flags for each space on the map */

int max_level;				/* Deepest player has gone */
int ntraps;				/* Number of traps on this level */
int dnum;				/* Dungeon number */
int level = 1;				/* What level rogue is on */
int purse = 0;				/* How much gold the rogue has */
int mpos = 0;				/* Where cursor is on top line */
int no_move = 0;			/* Number of turns held in place */
int no_command = 0;			/* Number of turns asleep */
int inpack = 0;				/* Number of things in pack */
int total = 0;				/* Total dynamic memory bytes */
int lastscore = -1;			/* Score before this turn */
int no_food = 0;			/* Number of levels without food */
int count = 0;				/* Number of times to repeat command */
int fung_hit = 0;			/* Number of time fungi has hit */
int quiet = 0;				/* Number of quiet turns */
int food_left;				/* Amount of food in hero's stomach */
int group = 2;				/* Current group number */
int hungry_state = 0;			/* How hungry is he */
int fd;					/* File descriptor for score file */
int a_chances[MAXARMORS] = {		/* Chance for each armor type */
    20,
    35,
    50,
    63,
    75,
    85,
    95,
    100
};
int a_class[MAXARMORS] = {		/* Armor class for each armor type */
    8,
    7,
    7,
    6,
    5,
    4,
    4,
    3,
};

long seed;				/* Random number seed */

coord oldpos;				/* Position before last look() call */
coord delta;				/* Change indicated to get_dir() */

THING player;				/* The rogue */
THING *cur_armor;			/* What a well dresssed rogue wears */
THING *cur_weapon;			/* Which weapon he is weilding */
THING *cur_ring[2];			/* Which rings are being worn */
THING *lvl_obj = NULL;			/* List of objects on this level */
THING *mlist = NULL;			/* List of monsters on the level */
THING *_monst[MAXLINES*MAXCOLS];	/* Pointers for monsters at each spot */

WINDOW *hw;				/* Used as a scratch window */

#define INIT_STATS { 16, 0, 1, 10, 12, "1d4", 12 }

struct stats max_stats = INIT_STATS;	/* The maximum for the player */

struct room *oldrp;			/* Roomin(&oldpos) */
struct room rooms[MAXROOMS];		/* One for each room -- A level */
struct room passages[MAXPASS] =		/* One for each passage */
{
    { {0, 0}, {0, 0}, {0, 0}, 0, ISGONE|ISDARK, 0, 0 },
    { {0, 0}, {0, 0}, {0, 0}, 0, ISGONE|ISDARK, 0, 0 },
    { {0, 0}, {0, 0}, {0, 0}, 0, ISGONE|ISDARK, 0, 0 },
    { {0, 0}, {0, 0}, {0, 0}, 0, ISGONE|ISDARK, 0, 0 },
    { {0, 0}, {0, 0}, {0, 0}, 0, ISGONE|ISDARK, 0, 0 },
    { {0, 0}, {0, 0}, {0, 0}, 0, ISGONE|ISDARK, 0, 0 },
    { {0, 0}, {0, 0}, {0, 0}, 0, ISGONE|ISDARK, 0, 0 },
    { {0, 0}, {0, 0}, {0, 0}, 0, ISGONE|ISDARK, 0, 0 },
    { {0, 0}, {0, 0}, {0, 0}, 0, ISGONE|ISDARK, 0, 0 },
    { {0, 0}, {0, 0}, {0, 0}, 0, ISGONE|ISDARK, 0, 0 },
    { {0, 0}, {0, 0}, {0, 0}, 0, ISGONE|ISDARK, 0, 0 },
    { {0, 0}, {0, 0}, {0, 0}, 0, ISGONE|ISDARK, 0, 0 }
};

#define ___ 1
#define XX 10
struct monster monsters[26] =
{
	/* Name		 CARRY	FLAG    str, exp, lvl, amr, hpt, dmg */
	{ "giant ant",	 0,	ISMEAN,	{ XX,  9,   2,   3, ___, "1d6" } },
	{ "bat",	 0,	0,	{ XX,  1,   1,   3, ___, "1d2" } },
	{ "centaur",	 15,	0,	{ XX, 15,   4,   4, ___, "1d6/1d6" } },
	{ "dragon",	 100,	ISMEAN,	{ XX,6800, 10,  -1, ___, "1d8/1d8/3d10" } },
	{ "floating eye",0,	0,	{ XX,  5,   1,   9, ___, "0d0" } },
		/* NOTE: the damage is %%% so that xstr won't merge this */
		/* string with others, since it is written on in the program */
	{ "violet fungi",0,	ISMEAN,	{ XX, 80,   8,   3, ___, "%%%d0" } },
	{ "gnome",	 10,	0,	{ XX,  7,   1,   5, ___, "1d6" } },
	{ "hobgoblin",	 0,	ISMEAN,	{ XX,  3,   1,   5, ___, "1d8" } },
	{ "invisible stalker",0,ISINVIS,{ XX,120,   8,   3, ___, "4d4" } },
	{ "jackal",	 0,	ISMEAN,	{ XX,  2,   1,   7, ___, "1d2" } },
	{ "kobold",	 0,	ISMEAN,	{ XX,  1,   1,   7, ___, "1d4" } },
	{ "leprechaun",	 0,	0,	{ XX, 10,   3,   8, ___, "1d1" } },
	{ "mimic",	 30,	0,	{ XX,100,   7,   7, ___, "3d4" } },
	{ "nymph",	 100,	0,	{ XX, 37,   3,   9, ___, "0d0" } },
	{ "orc",	 15,	ISGREED,{ XX,  5,   1,   6, ___, "1d8" } },
	{ "purple worm", 70,	0,	{ XX,4000, 15,   6, ___, "2d12/2d4" } },
	{ "quasit",	 30,	ISMEAN,	{ XX, 32,   3,   2, ___, "1d2/1d2/1d4" } },
	{ "rust monster",0,	ISMEAN,	{ XX, 20,   5,   2, ___, "0d0/0d0" } },
	{ "snake",	 0,	ISMEAN,	{ XX,  2,   1,   5, ___, "1d3" } },
	{ "troll",	 50,	ISREGEN|ISMEAN,{ XX, 120, 6, 4, ___, "1d8/1d8/2d6" } },
	{ "umber hulk",	 40,	ISMEAN,	{ XX,200,   8,   2, ___, "3d4/3d4/2d5" } },
	{ "vampire",	 20,	ISREGEN|ISMEAN,{ XX,350,   8,   1, ___, "1d10" } },
	{ "wraith",	 0,	0,	{ XX, 55,   5,   4, ___, "1d6" } },
	{ "xorn",	 0,	ISMEAN,	{ XX,190,   7,  -2, ___, "1d3/1d3/1d3/4d6" } },
	{ "yeti",	 30,	0,	{ XX, 50,   4,   6, ___, "1d6/1d6" } },
	{ "zombie",	 0,	ISMEAN,	{ XX,  6,   2,   8, ___, "1d8" } }
};
#undef ___
#undef XX

struct magic_item things[NUMTHINGS] = {
    { 0,			27 },	/* potion */
    { 0,			30 },	/* scroll */
    { 0,			17 },	/* food */
    { 0,			 8 },	/* weapon */
    { 0,			 8 },	/* armor */
    { 0,			 5 },	/* ring */
    { 0,			 5 },	/* stick */
};

struct magic_item s_magic[MAXSCROLLS] = {
    { "monster confusion",	 8, 140 },
    { "magic mapping",		 5, 150 },
    { "hold monster",		 3, 180 },
    { "sleep",			 5,   5 },
    { "enchant armor",		 8, 160 },
    { "identify",		27, 100 },
    { "scare monster",		 4, 200 },
    { "gold detection",		 4,  50 },
    { "teleportation",		 7, 165 },
    { "enchant weapon",		10, 150 },
    { "create monster",		 5,  75 },
    { "remove curse",		 8, 105 },
    { "aggravate monsters",	 4,  20 },
    { "blank paper",		 1,   5 },
    { "genocide",		 1, 300 },
};

struct magic_item p_magic[MAXPOTIONS] = {
    { "confusion",		 8,   5 },
    { "paralysis",		10,   5 },
    { "poison",			 8,   5 },
    { "gain strength",		15, 150 },
    { "see invisible",		 2, 100 },
    { "healing",		15, 130 },
    { "monster detection",	 6, 130 },
    { "magic detection",	 6, 105 },
    { "raise level",		 2, 250 },
    { "extra healing",		 5, 200 },
    { "haste self",		 4, 190 },
    { "restore strength",	14, 130 },
    { "blindness",		 4,   5 },
    { "thirst quenching",	 1,   5 },
};

struct magic_item r_magic[MAXRINGS] = {
    { "protection",		 9, 400 },
    { "add strength",		 9, 400 },
    { "sustain strength",	 5, 280 },
    { "searching",		10, 420 },
    { "see invisible",		10, 310 },
    { "adornment",		 1,  10 },
    { "aggravate monster",	10,  10 },
    { "dexterity",		 8, 440 },
    { "increase damage",	 8, 400 },
    { "regeneration",		 4, 460 },
    { "slow digestion",		 9, 240 },
    { "teleportation",		 5,  30 },
    { "stealth",		 7, 470 },
    { "maintain armor",		 5, 380 },
};

struct magic_item ws_magic[MAXSTICKS] = {
    { "light",			12, 250 },
    { "striking",		 9,  75 },
    { "lightning",		 3, 330 },
    { "fire",			 3, 330 },
    { "cold",			 3, 330 },
    { "polymorph",		15, 310 },
    { "magic missile",		10, 170 },
    { "haste monster",		 9,   5 },
    { "slow monster",		11, 350 },
    { "drain life",		 9, 300 },
    { "nothing",		 1,   5 },
    { "teleport away",		 5, 340 },
    { "teleport to",		 5,  50 },
    { "cancellation",		 5, 280 },
};

struct h_list helpstr[] = {
    '?',	"	prints help",
    '/',	"	identify object",
    'h',	"	left",
    'j',	"	down",
    'k',	"	up",
    'l',	"	right",
    'y',	"	up & left",
    'u',	"	up & right",
    'b',	"	down & left",
    'n',	"	down & right",
    'H',	"	run left",
    'J',	"	run down",
    'K',	"	run up",
    'L',	"	run right",
    'Y',	"	run up & left",
    'U',	"	run up & right",
    'B',	"	run down & left",
    'N',	"	run down & right",
    't',	"<dir>	throw something",
    'f',	"<dir>	forward until find something",
    'z',	"<dir>	zap a wand in a direction",
    '^',	"<dir>	identify trap type",
    's',	"	search for trap/secret door",
    '>',	"	go down a staircase",
    '<',	"	go up a staircase",
    '.',	"	rest for a while",
    'i',	"	inventory",
    'I',	"	inventory single item",
    'q',	"	quaff potion",
    'r',	"	read paper",
    'e',	"	eat food",
    'w',	"	wield a weapon",
    'W',	"	wear armor",
    'T',	"	take armor off",
    'P',	"	put on ring",
    'R',	"	remove ring",
    'd',	"	drop object",
    'c',	"	call object",
    'D',	"	recall what's been discovered",
    'o',	"	examine/set options",
    CTRL('L'),	"	redraw screen",
    CTRL('R'),	"	repeat last message",
    ESCAPE,	"	cancel command",
    '!',	"	shell escape",
    'S',	"	save game",
    'Q',	"	quit",
    0, 0
};
