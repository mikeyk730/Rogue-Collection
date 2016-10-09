/*
 * global variable initializaton
 *
 * @(#)init.c	4.16 (Berkeley) 3/30/82
 *
 * Rogue: Exploring the Dungeons of Doom
 * Copyright (C) 1980, 1981, 1982 Michael Toy, Ken Arnold and Glenn Wichman
 * All rights reserved.
 *
 * See the file LICENSE.TXT for full copyright and licensing information.
 */

#include <curses.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include "rogue.h"

/*
 * init_player:
 *	Roll up the rogue
 */
init_player()
{
    register THING *obj;

    pstats = max_stats;
    food_left = HUNGERTIME;
    /*
     * Give the rogue his weaponry.  First a mace.
     */
    obj = new_item();
    obj->o_type = WEAPON;
    obj->o_which = MACE;
    init_weapon(obj, MACE);
    obj->o_hplus = 1;
    obj->o_dplus = 1;
    obj->o_flags |= ISKNOW;
    obj->o_count = 1;
    obj->o_group = 0;
    add_pack(obj, TRUE);
    cur_weapon = obj;
    /*
     * Now a +1 bow
     */
    obj = new_item();
    obj->o_type = WEAPON;
    obj->o_which = BOW;
    init_weapon(obj, BOW);
    obj->o_hplus = 1;
    obj->o_dplus = 0;
    obj->o_count = 1;
    obj->o_group = 0;
    obj->o_flags |= ISKNOW;
    add_pack(obj, TRUE);
    /*
     * Now some arrows
     */
    obj = new_item();
    obj->o_type = WEAPON;
    obj->o_which = ARROW;
    init_weapon(obj, ARROW);
    obj->o_count = rnd(15) + 25;
    obj->o_hplus = obj->o_dplus = 0;
    obj->o_flags |= ISKNOW;
    add_pack(obj, TRUE);
    /*
     * And his suit of armor
     */
    obj = new_item();
    obj->o_type = ARMOR;
    obj->o_which = RING_MAIL;
    obj->o_ac = a_class[RING_MAIL] - 1;
    obj->o_flags |= ISKNOW;
    obj->o_count = 1;
    obj->o_group = 0;
    cur_armor = obj;
    add_pack(obj, TRUE);
    /*
     * Give him some food too
     */
    obj = new_item();
    obj->o_type = FOOD;
    obj->o_count = 1;
    obj->o_which = 0;
    obj->o_group = 0;
    add_pack(obj, TRUE);
}

/*
 * Contains defintions and functions for dealing with things like
 * potions and scrolls
 */

const char *rainbow[NCOLORS] = {
    "amber",
    "aquamarine",
    "black",
    "blue",
    "brown",
    "clear",
    "crimson",
    "cyan",
    "ecru",
    "gold",
    "green",
    "grey",
    "magenta",
    "orange",
    "pink",
    "plaid",
    "purple",
    "red",
    "silver",
    "tan",
    "tangerine",
    "topaz",
    "turquoise",
    "vermilion",
    "violet",
    "white",
    "yellow",
};

const char *sylls[NSYLLS] = {
    "a",   "ab",  "ag",  "aks", "ala", "an",  "ankh","app", "arg", "arze",
    "ash", "ban", "bar", "bat", "bek", "bie", "bin", "bit", "bjor",
    "blu", "bot", "bu",  "byt", "comp","con", "cos", "cre", "dalf",
    "dan", "den", "do",  "e",   "eep", "el",  "eng", "er",  "ere", "erk",
    "esh", "evs", "fa",  "fid", "for", "fri", "fu",  "gan", "gar",
    "glen","gop", "gre", "ha",  "he",  "hyd", "i",   "ing", "ion", "ip",
    "ish", "it",  "ite", "iv",  "jo",  "kho", "kli", "klis","la",  "lech",
    "man", "mar", "me",  "mi",  "mic", "mik", "mon", "mung","mur",
    "nej", "nelg","nep", "ner", "nes", "nes", "nih", "nin", "o",   "od",
    "ood", "org", "orn", "ox",  "oxy", "pay", "pet", "ple", "plu", "po",
    "pot", "prok","re",  "rea", "rhov","ri",  "ro",  "rog", "rok", "rol",
    "sa",  "san", "sat", "see", "sef", "seh", "shu", "ski", "sna",
    "sne", "snik","sno", "so",  "sol", "sri", "sta", "sun", "ta",
    "tab", "tem", "ther","ti",  "tox", "trol","tue", "turs","u",
    "ulk", "um",  "un",  "uni", "ur",  "val", "viv", "vly", "vom", "wah",
    "wed", "werg","wex", "whon","wun", "xo",  "y",   "yot", "yu",
    "zant","zap", "zeb", "zim", "zok", "zon", "zum",
};

const STONE stones[NSTONES] = {
    { "agate",		 25},
    { "alexandrite",	 40},
    { "amethyst",	 50},
    { "carnelian",	 40},
    { "diamond",	300},
    { "emerald",	300},
    { "germanium",	225},
    { "granite",	  5},
    { "garnet",		 50},
    { "jade",		150},
    { "kryptonite",	300},
    { "lapis lazuli",	 50},
    { "moonstone",	 50},
    { "obsidian",	 15},
    { "onyx",		 60},
    { "opal",		200},
    { "pearl",		220},
    { "peridot",	 63},
    { "ruby",		350},
    { "saphire",	285},
    { "stibotantalite",	200},
    { "tiger eye",	 50},
    { "topaz",		 60},
    { "turquoise",	 70},
    { "taaffeite",	300},
    { "zircon",	 	 80},
};

const char *wood[NWOOD] = {
    "avocado wood",
    "balsa",
    "bamboo",
    "banyan",
    "birch",
    "cedar",
    "cherry",
    "cinnibar",
    "cypress",
    "dogwood",
    "driftwood",
    "ebony",
    "elm",
    "eucalyptus",
    "fall",
    "hemlock",
    "holly",
    "ironwood",
    "kukui wood",
    "mahogany",
    "manzanita",
    "maple",
    "oaken",
    "persimmon wood",
    "pecan",
    "pine",
    "poplar",
    "redwood",
    "rosewood",
    "spruce",
    "teak",
    "walnut",
    "zebrawood",
};

const char *metal[NMETAL] = {
    "aluminum",
    "beryllium",
    "bone",
    "brass",
    "bronze",
    "copper",
    "electrum",
    "gold",
    "iron",
    "lead",
    "magnesium",
    "mercury",
    "nickel",
    "pewter",
    "platinum",
    "steel",
    "silver",
    "silicon",
    "tin",
    "titanium",
    "tungsten",
    "zinc",
};

/*
 * init_things
 *	Initialize the probabilities for types of things
 */
init_things()
{
    register struct magic_item *mp;

    for (mp = &things[1]; mp <= &things[NUMTHINGS-1]; mp++)
	mp->mi_prob += (mp-1)->mi_prob;
#ifdef WIZARD
    badcheck("things", things, NUMTHINGS);
#endif
}

/*
 * init_colors:
 *	Initialize the potion color scheme for this time
 */
init_colors()
{
    register int i, j;
    bool used[NCOLORS];

    for (i = 0; i < NCOLORS; i++)
	used[i] = FALSE;
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
#ifdef WIZARD
    badcheck("potions", p_magic, MAXPOTIONS);
#endif
}

/*
 * init_names:
 *	Generate the names of the various scrolls
 */
#define MAXNAME	40	/* Max number of characters in a name */

init_names()
{
    register int nsyl;
    register char *cp;
    const char *sp;
    register int i, nwords;

    for (i = 0; i < MAXSCROLLS; i++)
    {
	cp = prbuf;
	nwords = rnd(4) + 2;
	while (nwords--)
	{
	    nsyl = rnd(3) + 1;
	    while (nsyl--)
	    {
		sp = sylls[rnd((sizeof sylls) / (sizeof (char *)))];
		if (&cp[strlen(sp)] > &prbuf[MAXNAME])
			break;
		while (*sp)
		    *cp++ = *sp++;
	    }
	    *cp++ = ' ';
	}
	*--cp = '\0';
	s_names[i] = (char *) malloc((unsigned) strlen(prbuf)+1);
	s_know[i] = FALSE;
	s_guess[i] = NULL;
	strcpy(s_names[i], prbuf);
	if (i > 0)
	    s_magic[i].mi_prob += s_magic[i-1].mi_prob;
    }
#ifdef WIZARD
    badcheck("scrolls", s_magic, MAXSCROLLS);
#endif
}

/*
 * init_stones:
 *	Initialize the ring stone setting scheme for this time
 */
init_stones()
{
    register int i, j;
    bool used[NSTONES];

    for (i = 0; i < NSTONES; i++)
	used[i] = FALSE;
    for (i = 0; i < MAXRINGS; i++)
    {
	do
	    j = rnd(NSTONES);
	until (!used[j]);
	used[j] = TRUE;
	r_stones[i] = stones[j].st_name;
	r_know[i] = FALSE;
	r_guess[i] = NULL;
	if (i > 0)
	    r_magic[i].mi_prob += r_magic[i-1].mi_prob;
	r_magic[i].mi_worth += stones[j].st_value;
    }
#ifdef WIZARD
    badcheck("rings", r_magic, MAXRINGS);
#endif
}

/*
 * init_materials:
 *	Initialize the construction materials for wands and staffs
 */
init_materials()
{
    register int i, j;
    register const char *str;
    bool metused[NMETAL], woodused[NWOOD];

    for (i = 0; i < NWOOD; i++)
	woodused[i] = FALSE;
    for (i = 0; i < NMETAL; i++)
	metused[i] = FALSE;
    for (i = 0; i < MAXSTICKS; i++)
    {
	for (;;)
	    if (rnd(2) == 0)
	    {
		j = rnd(NMETAL);
		if (!metused[j])
		{
		    ws_type[i] = "wand";
		    str = metal[j];
		    metused[j] = TRUE;
		    break;
		}
	    }
	    else
	    {
		j = rnd(NWOOD);
		if (!woodused[j])
		{
		    ws_type[i] = "staff";
		    str = wood[j];
		    woodused[j] = TRUE;
		    break;
		}
	    }
	ws_made[i] = str;
	ws_know[i] = FALSE;
	ws_guess[i] = NULL;
	if (i > 0)
	    ws_magic[i].mi_prob += ws_magic[i-1].mi_prob;
    }
#ifdef WIZARD
    badcheck("sticks", ws_magic, MAXSTICKS);
#endif
}

#ifdef WIZARD
/*
 * badcheck:
 *	Check to see if a series of probabilities sums to 100
 */
badcheck(name, magic, bound)
char *name;
register struct magic_item *magic;
register int bound;
{
    register struct magic_item *end;

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
#endif
