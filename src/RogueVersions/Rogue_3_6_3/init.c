/*
 * global variable initializaton
 *
 * @(#)init.c	3.33 (Berkeley) 6/15/81
 *
 * Rogue: Exploring the Dungeons of Doom
 * Copyright (C) 1980, 1981 Michael Toy, Ken Arnold and Glenn Wichman
 * All rights reserved.
 *
 * See the file LICENSE.TXT for full copyright and licensing information.
 */

#include "curses.h"
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include "machdep.h"
#include "rogue.h"

int playing = TRUE, running = FALSE, wizard = FALSE;
int notify = TRUE, fight_flush = FALSE, terse = FALSE, door_stop = FALSE;
int jump = FALSE, slow_invent = FALSE, firstmove = FALSE, askme = FALSE, showac = TRUE;
int amulet = FALSE;
int in_shell = FALSE;
struct linked_list *lvl_obj = NULL, *mlist = NULL;
struct object *cur_weapon = NULL;
int mpos = 0, no_move = 0, no_command = 0, level = 1, purse = 0, inpack = 0;
int total = 0, no_food = 0, count = 0, fung_hit = 0, quiet = 0;
int food_left = HUNGERTIME, group = 1, hungry_state = 0;
int lastscore = -1;

struct thing player;
struct room rooms[MAXROOMS];
struct room *oldrp;
struct stats max_stats;
struct object *cur_armor;
struct object *cur_ring[2];
int after;
int waswizard;
coord oldpos;                            /* Position before last look() call */
coord delta;                             /* Change indicated to get_dir()    */

int s_know[MAXSCROLLS];         /* Does he know what a scroll does */
int p_know[MAXPOTIONS];         /* Does he know what a potion does */
int r_know[MAXRINGS];                   /* Does he know what a ring does
 */
int ws_know[MAXSTICKS];         /* Does he know what a stick does */

int  take;                               /* Thing the rogue is taking */
int  runch;                              /* Direction player is running */
char whoami[80];                 /* Name of player */
char fruit[80];                          /* Favorite fruit */
char huh[80];                            /* The last message printed */
int dnum;                                /* Dungeon number */
char *s_names[MAXSCROLLS];               /* Names of the scrolls */
char *p_colors[MAXPOTIONS];              /* Colors of the potions */
char *r_stones[MAXRINGS];                /* Stone settings of the rings */
char *a_names[MAXARMORS];                /* Names of armor types */
char *ws_made[MAXSTICKS];                /* What sticks are made of */
char *s_guess[MAXSCROLLS];               /* Players guess at what scroll is */
char *p_guess[MAXPOTIONS];               /* Players guess at what potion is */
char *r_guess[MAXRINGS];         /* Players guess at what ring is */
char *ws_guess[MAXSTICKS];               /* Players guess at what wand is */
char *ws_type[MAXSTICKS];                /* Is it a wand or a staff */
char file_name[80];                      /* Save file name */
char home[PATH_MAX];                     /* User's home directory */
unsigned char prbuf[80];                 /* Buffer for sprintfs */
int max_hp;                              /* Player's max hit points */
int ntraps;                              /* Number of traps on this level */
int max_level;                           /* Deepest player has gone */
int seed;                                /* Random number seed */

struct trap  traps[MAXTRAPS];


#define ___ 1
#define _x {1,1}
struct monster monsters[26] = {
	/* Name		 CARRY	FLAG    str, exp, lvl, amr, hpt, dmg */
	{ "giant ant",	 0,	ISMEAN,	{ _x, 10,   2,   3, ___, "1d6" } },
	{ "bat",	 0,	0,	{ _x,  1,   1,   3, ___, "1d2" } },
	{ "centaur",	 15,	0,	{ _x, 15,   4,   4, ___, "1d6/1d6" } },
	{ "dragon",	 100,	ISGREED,{ _x,9000, 10,  -1, ___, "1d8/1d8/3d10" } },
	{ "floating eye",0,	0,	{ _x,  5,   1,   9, ___, "0d0" } },
	{ "violet fungi",0,	ISMEAN,	{ _x, 85,   8,   3, ___, "000d0" } },
	{ "gnome",	 10,	0,	{ _x,  8,   1,   5, ___, "1d6" } },
	{ "hobgoblin",	 0,	ISMEAN,	{ _x,  3,   1,   5, ___, "1d8" } },
	{ "invisible stalker",0,ISINVIS,{ _x,120,   8,   3, ___, "4d4" } },
	{ "jackal",	 0,	ISMEAN,	{ _x,  2,   1,   7, ___, "1d2" } },
	{ "kobold",	 0,	ISMEAN,	{ _x,  1,   1,   7, ___, "1d4" } },
	{ "leprechaun",	 0,	0,	{ _x, 10,   3,   8, ___, "1d1" } },
	{ "mimic",	 30,	0,	{ _x,140,   7,   7, ___, "3d4" } },
	{ "nymph",	 100,	0,	{ _x, 40,   3,   9, ___, "0d0" } },
	{ "orc",	 15,	ISBLOCK,{ _x,  5,   1,   6, ___, "1d8" } },
	{ "purple worm", 70,	0,	{ _x,7000, 15,   6, ___, "2d12/2d4" } },
	{ "quasit",	 30,	ISMEAN,	{ _x, 35,   3,   2, ___, "1d2/1d2/1d4" } },
	{ "rust monster",0,	ISMEAN,	{ _x, 25,   5,   2, ___, "0d0/0d0" } },
	{ "snake",	 0,	ISMEAN,	{ _x,  3,   1,   5, ___, "1d3" } },
	{ "troll",	 50,	ISREGEN|ISMEAN,{ _x, 55,   6,   4, ___, "1d8/1d8/2d6" } },
	{ "umber hulk",	 40,	ISMEAN,	{ _x,130,   8,   2, ___, "3d4/3d4/2d5" } },
	{ "vampire",	 20,	ISREGEN|ISMEAN,{ _x,380,   8,   1, ___, "1d10" } },
	{ "wraith",	 0,	0,	{ _x, 55,   5,   4, ___, "1d6" } },
	{ "xorn",	 0,	ISMEAN,	{ _x,120,   7,  -2, ___, "1d3/1d3/1d3/4d6" } },
	{ "yeti",	 30,	0,	{ _x, 50,   4,   6, ___, "1d6/1d6" } },
	{ "zombie",	 0,	ISMEAN,	{ _x,  7,   2,   8, ___, "1d8" } }
};
#undef ___

/*
 * init_player:
 *	roll up the rogue
 */

void
init_player()
{
    pstats.s_lvl = 1;
    pstats.s_exp = 0L;
    max_hp = pstats.s_hpt = 12;
    if (rnd(100) == 7)
    {
	pstats.s_str.st_str = 18;
	pstats.s_str.st_add = rnd(100) + 1;
    }
    else
    {
	pstats.s_str.st_str = 16;
	pstats.s_str.st_add = 0;
    }
    strcpy(pstats.s_dmg,"1d4");
    pstats.s_arm = 10;
    max_stats = pstats;
    pack = NULL;
}

/*
 * Contains defintions and functions for dealing with things like
 * potions and scrolls
 */

char *rainbow[] = {
    "red",
    "blue",
    "green",
    "yellow",
    "black",
    "brown",
    "orange",
    "pink",
    "purple",
    "grey",
    "white",
    "silver",
    "gold",
    "violet",
    "clear",
    "vermilion",
    "ecru",
    "turquoise",
    "magenta",
    "amber",
    "topaz",
    "plaid",
    "tan",
    "tangerine"
};

#define NCOLORS (sizeof rainbow / sizeof (char *))
int cNCOLORS = NCOLORS;

char *sylls[] = {
    "a", "ab", "ag", "aks", "ala", "an", "ankh", "app", "arg", "arze",
    "ash", "ban", "bar", "bat", "bek", "bie", "bin", "bit", "bjor",
    "blu", "bot", "bu", "byt", "comp", "con", "cos", "cre", "dalf",
    "dan", "den", "do", "e", "eep", "el", "eng", "er", "ere", "erk",
    "esh", "evs", "fa", "fid", "for", "fri", "fu", "gan", "gar",
    "glen", "gop", "gre", "ha", "he", "hyd", "i", "ing", "ion", "ip",
    "ish", "it", "ite", "iv", "jo", "kho", "kli", "klis", "la", "lech",
    "man", "mar", "me", "mi", "mic", "mik", "mon", "mung", "mur",
    "nej", "nelg", "nep", "ner", "nes", "nes", "nih", "nin", "o", "od",
    "ood", "org", "orn", "ox", "oxy", "pay", "pet", "ple", "plu", "po",
    "pot", "prok", "re", "rea", "rhov", "ri", "ro", "rog", "rok", "rol",
    "sa", "san", "sat", "see", "sef", "seh", "shu", "ski", "sna",
    "sne", "snik", "sno", "so", "sol", "sri", "sta", "sun", "ta",
    "tab", "tem", "ther", "ti", "tox", "trol", "tue", "turs", "u",
    "ulk", "um", "un", "uni", "ur", "val", "viv", "vly", "vom", "wah",
    "wed", "werg", "wex", "whon", "wun", "xo", "y", "yot", "yu",
    "zant", "zap", "zeb", "zim", "zok", "zon", "zum",
};

char *stones[] = {
    "agate",
    "alexandrite",
    "amethyst",
    "carnelian",
    "diamond",
    "emerald",
    "granite",
    "jade",
    "kryptonite",
    "lapus lazuli",
    "moonstone",
    "obsidian",
    "onyx",
    "opal",
    "pearl",
    "ruby",
    "saphire",
    "tiger eye",
    "topaz",
    "turquoise",
};

#define NSTONES (sizeof stones / sizeof (char *))
int cNSTONES = NSTONES;

char *wood[] = {
    "avocado wood",
    "balsa",
    "banyan",
    "birch",
    "cedar",
    "cherry",
    "cinnibar",
    "driftwood",
    "ebony",
    "eucalyptus",
    "hemlock",
    "ironwood",
    "mahogany",
    "manzanita",
    "maple",
    "oak",
    "persimmon wood",
    "redwood",
    "rosewood",
    "teak",
    "walnut",
    "zebra wood",
};

#define NWOOD (sizeof wood / sizeof (char *))
int cNWOOD = NWOOD;

char *metal[] = {
    "aluminium",
    "bone",
    "brass",
    "bronze",
    "copper",
    "iron",
    "lead",
    "pewter",
    "steel",
    "tin",
    "zinc",
};

#define NMETAL (sizeof metal / sizeof (char *))
int cNMETAL = NMETAL;

struct magic_item things[NUMTHINGS] = {
    { "",			27 },	/* potion */
    { "",			27 },	/* scroll */
    { "",			18 },	/* food */
    { "",			 9 },	/* weapon */
    { "",			 9 },	/* armor */
    { "",			 5 },	/* ring */
    { "",			 5 },	/* stick */
};

struct magic_item s_magic[MAXSCROLLS] = {
    { "monster confusion",	 8, 170 },
    { "magic mapping",		 5, 180 },
    { "light",			10, 100 },
    { "hold monster",		 2, 200 },
    { "sleep",			 5,  50 },
    { "enchant armor",		 8, 130 },
    { "identify",		21, 100 },
    { "scare monster",		 4, 180 },
    { "gold detection",		 4, 110 },
    { "teleportation",		 7, 175 },
    { "enchant weapon",		10, 150 },
    { "create monster",		 5,  75 },
    { "remove curse",		 8, 105 },
    { "aggravate monsters",	 1,  60 },
    { "blank paper",		 1,  50 },
    { "genocide",		 1, 200 },
};

struct magic_item p_magic[MAXPOTIONS] = {
    { "confusion",		 8,  50 },
    { "paralysis",		10,  50 },
    { "poison",			 8,  50 },
    { "gain strength",		15, 150 },
    { "see invisible",		 2, 170 },
    { "healing",		15, 130 },
    { "monster detection",	 6, 120 },
    { "magic detection",	 6, 105 },
    { "raise level",		 2, 220 },
    { "extra healing",		 5, 180 },
    { "haste self",		 4, 200 },
    { "restore strength",	14, 120 },
    { "blindness",		 4,  50 },
    { "thirst quenching",	 1,  50 },
};

struct magic_item r_magic[MAXRINGS] = {
    { "protection",		 9, 200 },
    { "add strength",		 9, 200 },
    { "sustain strength",	 5, 180 },
    { "searching",		10, 200 },
    { "see invisible",		10, 175 },
    { "adornment",		 1, 100 },
    { "aggravate monster",	11, 100 },
    { "dexterity",		 8, 220 },
    { "increase damage",	 8, 220 },
    { "regeneration",		 4, 260 },
    { "slow digestion",		 9, 240 },
    { "teleportation",		 9, 100 },
    { "stealth",		 7, 100 },
};

struct magic_item ws_magic[MAXSTICKS] = {
    { "light",			12, 120 },
    { "striking",		 9, 115 },
    { "lightning",		 3, 200 },
    { "fire",			 3, 200 },
    { "cold",			 3, 200 },
    { "polymorph",		15, 210 },
    { "magic missile",		10, 170 },
    { "haste monster",		 9,  50 },
    { "slow monster",		11, 220 },
    { "drain life",		 9, 210 },
    { "nothing",		 1,  70 },
    { "teleport away",		 5, 140 },
    { "teleport to",		 5,  60 },
    { "cancellation",		 5, 130 },
};

int a_class[MAXARMORS] = {
    8,
    7,
    7,
    6,
    5,
    4,
    4,
    3,
};

char *a_names[MAXARMORS] = {
    "leather armor",
    "ring mail",
    "studded leather armor",
    "scale mail",
    "chain mail",
    "splint mail",
    "banded mail",
    "plate mail",
};

int a_chances[MAXARMORS] = {
    20,
    35,
    50,
    63,
    75,
    85,
    95,
    100
};

#define MAX3(a,b,c)     (a > b ? (a > c ? a : c) : (b > c ? b : c))
static int used[MAX3(NCOLORS, NSTONES, NWOOD)];

/*
 * init_things
 *	Initialize the probabilities for types of things
 */
void
init_things()
{
    struct magic_item *mp;

    for (mp = &things[1]; mp <= &things[NUMTHINGS-1]; mp++)
	mp->mi_prob += (mp-1)->mi_prob;
    badcheck("things", things, NUMTHINGS);
}

/*
 * init_colors:
 *	Initialize the potion color scheme for this time
 */

void
init_colors()
{
    int i, j;

    for (i = 0; i < NCOLORS; i++)
	used[i] = 0;
    for (i = 0; i < MAXPOTIONS; i++)
    {
	do
	    j = rnd(NCOLORS);
	until (!used[j]);
	used[j] = TRUE;
	p_colors[i] = rainbow[j];
	p_know[i] = FALSE;
	p_guess[i] = NULL;
	if (i > 0)
		p_magic[i].mi_prob += p_magic[i-1].mi_prob;
    }
    badcheck("potions", p_magic, MAXPOTIONS);
}

/*
 * init_names:
 *	Generate the names of the various scrolls
 */

void
init_names()
{
    int nsyl;
    char *cp, *sp;
    int i, nwords;

    for (i = 0; i < MAXSCROLLS; i++)
    {
	cp = prbuf;
	nwords = rnd(4)+2;
	while(nwords--)
	{
	    nsyl = rnd(3)+1;
	    while(nsyl--)
	    {
		sp = sylls[rnd((sizeof sylls) / (sizeof (char *)))];
		while(*sp)
		    *cp++ = *sp++;
	    }
	    *cp++ = ' ';
	}
	*--cp = '\0';
	s_names[i] = (char *) _new(strlen(prbuf)+1);
	s_know[i] = FALSE;
	s_guess[i] = NULL;
	strcpy(s_names[i], prbuf);
	if (i > 0)
		s_magic[i].mi_prob += s_magic[i-1].mi_prob;
    }
    badcheck("scrolls", s_magic, MAXSCROLLS);
}

/*
 * init_stones:
 *	Initialize the ring stone setting scheme for this time
 */

void
init_stones()
{
    int i, j;

    for (i = 0; i < NSTONES; i++)
	used[i] = FALSE;
    for (i = 0; i < MAXRINGS; i++)
    {
	do
	    j = rnd(NSTONES);
	until (!used[j]);
	used[j] = TRUE;
	r_stones[i] = stones[j];
	r_know[i] = FALSE;
	r_guess[i] = NULL;
	if (i > 0)
		r_magic[i].mi_prob += r_magic[i-1].mi_prob;
    }
    badcheck("rings", r_magic, MAXRINGS);
}

/*
 * init_materials:
 *	Initialize the construction materials for wands and staffs
 */

void
init_materials()
{
    int i, j;
    static int metused[NMETAL];

    for (i = 0; i < NWOOD; i++)
	used[i] = FALSE;
    for (i = 0; i < NMETAL; i++)
	metused[i] = FALSE;

    for (i = 0; i < MAXSTICKS; i++)
    {
	for (;;)
	    if (rnd(100) > 50)
	    {
		j = rnd(NMETAL);
		if (!metused[j])
		{
		    metused[j] = TRUE;
		    ws_made[i] = metal[j];
		    ws_type[i] = "wand";
		    break;
		}
	    }
	    else
	    {
		j = rnd(NWOOD);
		if (!used[j])
		{
		    used[j] = TRUE;
		    ws_made[i] = wood[j];
		    ws_type[i] = "staff";
		    break;
		}
	    }

	ws_know[i] = FALSE;
	ws_guess[i] = NULL;
	if (i > 0)
		ws_magic[i].mi_prob += ws_magic[i-1].mi_prob;
    }
    badcheck("sticks", ws_magic, MAXSTICKS);
}

void
badcheck(char *name, struct magic_item *magic, int bound)
{
    struct magic_item *end;

    if (magic[bound - 1].mi_prob == 100)
	return;
    printf("\nBad percentages for %s:\n", name);
    for (end = &magic[bound]; magic < end; magic++)
	printf("%3d%% %s\n", magic->mi_prob, magic->mi_name);
    printf("[hit RETURN to continue]");
    fflush(stdout);
    while (getchar() != '\n')
	continue;
}

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
    'p',	"<dir>	zap a wand in a direction",
    'z',	"	zap a wand or staff",
    '>',	"	go down a staircase",
    's',	"	search for trap/secret door",
    REST_COMMAND,	"	"REST_DESC"rest for a while",
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
    'o',	"	examine/set options",
    CTRL('E'),	"	redraw screen",
    CTRL('R'),	"	repeat last message",
    ESCAPE,	"	cancel command",
    'v',	"	print program version number",
    '!',	"	shell escape",
    'S',	"	save game",
    'Q',	"	quit",
    0, 0
};

FILE* g_game_log = NULL;
