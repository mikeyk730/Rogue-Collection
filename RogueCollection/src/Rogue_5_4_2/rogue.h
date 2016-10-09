/*
 * Rogue definitions and variable declarations
 *
 * @(#)rogue.h	5.42 (Berkeley) 08/06/83
 *
 * Rogue: Exploring the Dungeons of Doom
 * Copyright (C) 1980-1983, 1985, 1999 Michael Toy, Ken Arnold and Glenn Wichman
 * All rights reserved.
 *
 * See the file LICENSE.TXT for full copyright and licensing information.
 */

#include "extern.h"

#undef lines 

#define NOOP(x) (x += 0)
#define CCHAR(x) ( (x) & A_CHARTEXT )

#define MAXDAEMONS 20
#define EMPTY 0

/*
 * Maximum number of different things
 */
#define MAXROOMS	9
#define MAXTHINGS	9
#define MAXOBJ		9
#define MAXPACK		23
#define MAXTRAPS	10
#define AMULETLEVEL	26
#define	NUMTHINGS	7	/* number of types of things */
#define MAXPASS		13	/* upper limit on number of passages */
#define	NUMLINES	24
#define	NUMCOLS		80
#define STATLINE		(NUMLINES - 1)
#define BORE_LEVEL	50

/*
 * return values for get functions
 */
#define	NORM	0	/* normal exit */
#define	QUIT	1	/* quit option setting */
#define	MINUS	2	/* back up one option */

/*
 * inventory types
 */
#define	INV_OVER	0
#define	INV_SLOW	1
#define	INV_CLEAR	2

/*
 * All the fun defines
 */
#define when		break;case
#define otherwise	break;default
#define until(expr)	while(!(expr))
#define next(ptr)	(*ptr).l_next
#define prev(ptr)	(*ptr).l_prev
#define winat(y,x)	(moat(y,x) != NULL ? moat(y,x)->t_disguise : chat(y,x))
#define ce(a,b)		((a).x == (b).x && (a).y == (b).y)
#define hero		player.t_pos
#define pstats		player.t_stats
#define pack		player.t_pack
#define proom		player.t_room
#define max_hp		player.t_stats.s_maxhp
#define attach(a,b)	_attach(&a,b)
#define detach(a,b)	_detach(&a,b)
#define free_list(a)	_free_list(&a)
#undef max
#define max(a,b)	((a) > (b) ? (a) : (b))
#define on(thing,flag)	((((thing).t_flags & (flag)) != 0))
#define GOLDCALC	(rnd(50 + 10 * level) + 2)
#define ISRING(h,r)	(cur_ring[h] != NULL && cur_ring[h]->o_which == r)
#define ISWEARING(r)	(ISRING(LEFT, r) || ISRING(RIGHT, r))
#define ISMULT(type) 	(type == POTION || type == SCROLL || type == FOOD)
#define INDEX(y,x)	(&places[((x) << 5) + (y)])
#define chat(y,x)	(places[((x) << 5) + (y)].p_ch)
#define flat(y,x)	(places[((x) << 5) + (y)].p_flags)
#define moat(y,x)	(places[((x) << 5) + (y)].p_monst)
#define unc(cp)		(cp).y, (cp).x
#ifdef MASTER
#define debug		if (wizard) msg
#endif

/*
 * things that appear on the screens
 */
#define PASSAGE		'#'
#define DOOR		'+'
#define FLOOR		'.'
#define PLAYER		'@'
#define TRAP		'^'
#define STAIRS		'%'
#define GOLD		'*'
#define POTION		'!'
#define SCROLL		'?'
#define MAGIC		'$'
#define FOOD		':'
#define WEAPON		')'
#define ARMOR		']'
#define AMULET		','
#define RING		'='
#define STICK		'/'
#define CALLABLE	-1
#define R_OR_S		-2

/*
 * Various constants
 */
#define BEARTIME	spread(3)
#define SLEEPTIME	spread(5)
#define HOLDTIME	spread(2)
#define WANDERTIME	spread(70)
#define BEFORE		spread(1)
#define AFTER		spread(2)
#define HEALTIME	30
#define HUHDURATION	20
#define SEEDURATION	850
#define HUNGERTIME	1300
#define MORETIME	150
#define STOMACHSIZE	2000
#define STARVETIME	850
#define ESCAPE		27
#define LEFT		0
#define RIGHT		1
#define BOLT_LENGTH	6
#define LAMPDIST	3
#ifdef MASTER
#ifndef PASSWD
#define	PASSWD		"mTBellIQOsLNA"
#endif
#endif

/*
 * Save against things
 */
#define VS_POISON	00
#define VS_PARALYZATION	00
#define VS_DEATH	00
#define VS_BREATH	02
#define VS_MAGIC	03

/*
 * Various flag bits
 */
/* flags for rooms */
#define ISDARK	0000001		/* room is dark */
#define ISGONE	0000002		/* room is gone (a corridor) */
#define ISMAZE	0000004		/* room is gone (a corridor) */

/* flags for objects */
#define ISCURSED 000001		/* object is cursed */
#define ISKNOW	0000002		/* player knows details about the object */
#define ISMISL	0000004		/* object is a missile type */
#define ISMANY	0000010		/* object comes in groups */
/*	ISFOUND 0000020		...is used for both objects and creatures */
#define	ISPROT	0000040		/* armor is permanently protected */

/* flags for creatures */
#define CANHUH	0000001		/* creature can confuse */
#define CANSEE	0000002		/* creature can see invisible creatures */
#define ISBLIND	0000004		/* creature is blind */
#define ISCANC	0000010		/* creature has special qualities cancelled */
#define ISLEVIT	0000010		/* hero is levitating */
#define ISFOUND	0000020		/* creature has been seen (used for objects) */
#define ISGREED	0000040		/* creature runs to protect gold */
#define ISHASTE	0000100		/* creature has been hastened */
#define ISTARGET 000200		/* creature is the target of an 'f' command */
#define ISHELD	0000400		/* creature has been held */
#define ISHUH	0001000		/* creature is confused */
#define ISINVIS	0002000		/* creature is invisible */
#define ISMEAN	0004000		/* creature can wake when player enters room */
#define ISHALU	0004000		/* hero is on acid trip */
#define ISREGEN	0010000		/* creature can regenerate */
#define ISRUN	0020000		/* creature is running at the player */
#define SEEMONST 040000		/* hero can detect unseen monsters */
#define ISFLY	0040000		/* creature can fly */
#define ISSLOW	0100000		/* creature has been slowed */

/*
 * Flags for level map
 */
#define F_PASS		0x80		/* is a passageway */
#define F_SEEN		0x40		/* have seen this spot before */
#define F_DROPPED	0x20		/* object was dropped here */
#define F_LOCKED	0x20		/* door is locked */
#define F_REAL		0x10		/* what you see is what you get */
#define F_PNUM		0x0f		/* passage number mask */
#define F_TMASK		0x07		/* trap number mask */

/*
 * Trap types
 */
#define T_DOOR	00
#define T_ARROW	01
#define T_SLEEP	02
#define T_BEAR	03
#define T_TELEP	04
#define T_DART	05
#define T_RUST	06
#define T_MYST  07
#define NTRAPS	8

/*
 * Potion types
 */
#define P_CONFUSE	0
#define P_LSD		1
#define P_POISON	2
#define P_STRENGTH	3
#define P_SEEINVIS	4
#define P_HEALING	5
#define P_MFIND		6
#define	P_TFIND 	7
#define	P_RAISE		8
#define P_XHEAL		9
#define P_HASTE		10
#define P_RESTORE	11
#define P_BLIND		12
#define P_LEVIT		13
#define MAXPOTIONS	14

/*
 * Scroll types
 */
#define S_CONFUSE	0
#define S_MAP		1
#define S_HOLD		2
#define S_SLEEP		3
#define S_ARMOR		4
#define S_ID_POTION	5
#define S_ID_SCROLL	6
#define S_ID_WEAPON	7
#define S_ID_ARMOR	8
#define S_ID_R_OR_S	9
#define S_SCARE		10
#define S_FDET		11
#define S_TELEP		12
#define S_ENCH		13
#define S_CREATE	14
#define S_REMOVE	15
#define S_AGGR		16
#define S_PROTECT	17
#define MAXSCROLLS	18

/*
 * Weapon types
 */
#define MACE		0
#define SWORD		1
#define BOW		2
#define ARROW		3
#define DAGGER		4
#define TWOSWORD	5
#define DART		6
#define SHIRAKEN	7
#define SPEAR		8
#define FLAME		9	/* fake entry for dragon breath (ick) */
#define MAXWEAPONS	9	/* this should equal FLAME */

/*
 * Armor types
 */
#define LEATHER		0
#define RING_MAIL	1
#define STUDDED_LEATHER	2
#define SCALE_MAIL	3
#define CHAIN_MAIL	4
#define SPLINT_MAIL	5
#define BANDED_MAIL	6
#define PLATE_MAIL	7
#define MAXARMORS	8

/*
 * Ring types
 */
#define R_PROTECT	0
#define R_ADDSTR	1
#define R_SUSTSTR	2
#define R_SEARCH	3
#define R_SEEINVIS	4
#define R_NOP		5
#define R_AGGR		6
#define R_ADDHIT	7
#define R_ADDDAM	8
#define R_REGEN		9
#define R_DIGEST	10
#define R_TELEPORT	11
#define R_STEALTH	12
#define R_SUSTARM	13
#define MAXRINGS	14

/*
 * Rod/Wand/Staff types
 */
#define WS_LIGHT	0
#define WS_INVIS	1
#define WS_ELECT	2
#define WS_FIRE		3
#define WS_COLD		4
#define WS_POLYMORPH	5
#define WS_MISSILE	6
#define WS_HASTE_M	7
#define WS_SLOW_M	8
#define WS_DRAIN	9
#define WS_NOP		10
#define WS_TELAWAY	11
#define WS_TELTO	12
#define WS_CANCEL	13
#define MAXSTICKS	14

/*
 * Now we define the structures and types
 */

/*
 * Help list
 */
struct h_list {
    int h_ch;
    const char *h_desc;
    int h_print;
};

/*
 * Coordinate data type
 */
typedef struct coord {
    int x;
    int y;
} coord;

/*
 * Stuff about objects
 */
struct obj_info {
    const char *oi_name;
    int oi_prob;
    int oi_worth;
    char *oi_guess;
    int oi_know;
};

/*
 * Room structure
 */
struct room {
    coord r_pos;			/* Upper left corner */
    coord r_max;			/* Size of room */
    coord r_gold;			/* Where the gold is */
    int r_goldval;			/* How much the gold is worth */
    int r_flags;			/* info about the room */
    int r_nexits;			/* Number of exits */
    coord r_exit[12];			/* Where the exits are */
};

/*
 * Structure describing a fighting being
 */
struct stats {
    int s_str;			/* Strength */
    int s_exp;				/* Experience */
    int s_lvl;				/* level of mastery */
    int s_arm;				/* Armor class */
    int s_hpt;			/* Hit points */
    char s_dmg[13];			/* String describing damage done */
    int  s_maxhp;			/* Max hit points */
};

/*
 * Structure for monsters and player
 */
union thing {
    struct {
	union thing *_l_next, *_l_prev;	/* Next pointer in link */
	coord _t_pos;			/* Position */
	int _t_turn;			/* If slowed, is it a turn to move */
	int _t_type;			/* What it is */
	int _t_disguise;		/* What mimic looks like */
	int _t_oldch;			/* Character that was where it was */
	const coord *_t_dest;		/* Where it is running to */
	int _t_flags;			/* State word */
	struct stats _t_stats;		/* Physical description */
	struct room *_t_room;		/* Current room for thing */
	union thing *_t_pack;		/* What the thing is carrying */
        int _t_reserved;
    } _t;
    struct {
	union thing *_l_next, *_l_prev;	/* Next pointer in link */
	int _o_type;			/* What kind of object it is */
	coord _o_pos;			/* Where it lives on the screen */
	char *_o_text;			/* What it says if you read it */
	int  _o_launch;			/* What you need to launch it */
	int _o_packch;			/* What character it is in the pack */
	char _o_damage[8];		/* Damage if used like sword */
	char _o_hurldmg[8];		/* Damage if thrown */
	int _o_count;			/* count for plural objects */
	int _o_which;			/* Which object of a type it is */
	int _o_hplus;			/* Plusses to hit */
	int _o_dplus;			/* Plusses to damage */
	int _o_arm;			/* Armor protection */
	int _o_flags;			/* information about objects */
	int _o_group;			/* group number for this object */
	char *_o_label;			/* Label for object */
    } _o;
};

typedef union thing THING;

#define l_next		_t._l_next
#define l_prev		_t._l_prev
#define t_pos		_t._t_pos
#define t_turn		_t._t_turn
#define t_type		_t._t_type
#define t_disguise	_t._t_disguise
#define t_oldch		_t._t_oldch
#define t_dest		_t._t_dest
#define t_flags		_t._t_flags
#define t_stats		_t._t_stats
#define t_pack		_t._t_pack
#define t_room		_t._t_room
#define t_reserved      _t._t_reserved
#define o_type		_o._o_type
#define o_pos		_o._o_pos
#define o_text		_o._o_text
#define o_launch	_o._o_launch
#define o_packch	_o._o_packch
#define o_damage	_o._o_damage
#define o_hurldmg	_o._o_hurldmg
#define o_count		_o._o_count
#define o_which		_o._o_which
#define o_hplus		_o._o_hplus
#define o_dplus		_o._o_dplus
#define o_arm		_o._o_arm
#define o_charges	o_arm
#define o_goldval	o_arm
#define o_flags		_o._o_flags
#define o_group		_o._o_group
#define o_label		_o._o_label

/*
 * describe a place on the level map
 */
typedef struct PLACE {
    int p_ch;
    int p_flags;
    THING *p_monst;
} PLACE;

/*
 * Array containing information on all the various types of monsters
 */
struct monster {
    const char *m_name;			/* What to call the monster */
    int m_carry;			/* Probability of carrying something */
    int m_flags;			/* things about the monster */
    struct stats m_stats;		/* Initial stats */
};

struct delayed_action {
    int d_type;
    void (*d_func)();
    int d_arg;
    int d_time;
};

struct STONE {
    char	*st_name;
    int		st_value;
};

typedef struct STONE STONE;

/*
 * External variables
 */

extern int after, again, allscore, door_stop, fight_flush,
	   firstmove, has_hit, inv_describe, jump, kamikaze,
	   lower_msg, move_on, msg_esc, pack_used[],
	   passgo, playing, q_comm, running, save_msg, see_floor,
	   seenstairs, stat_msg, terse, to_death, tombstone,
           amulet, count, dir_ch, food_left, hungry_state, inpack,
	   inv_type, lastscore, level, max_hit, max_level, mpos, take,
	   n_objs, no_command, no_food, no_move, noscore, ntraps, purse,
	   quiet, vf_hit, runch, last_comm, l_last_comm, last_dir, l_last_dir,
	   numscores, total, between, group, cNWOOD, cNMETAL, cNSTONES,
	   cNCOLORS;

extern char file_name[], home[], huh[], *Numname, outbuf[], 
	    *ws_type[], *s_names[];

extern const char *ws_made[], *inv_t_name[], *p_colors[], *r_stones[], 
                  *release, *tr_name[], *rainbow[], *wood[], *metal[],
		  encstr[], statlist[], version[];

extern const int a_class[], e_levels[];

extern unsigned int dnum, seed;

extern WINDOW *hw;

extern coord delta, oldpos, stairs;

extern PLACE places[];

extern THING *cur_armor, *cur_ring[], *cur_weapon, *l_last_pick,
	     *last_pick, *lvl_obj, *mlist, player;

extern const struct h_list helpstr[];

extern struct room *oldrp, passages[], rooms[];

extern struct stats max_stats;

extern struct monster monsters[];

extern struct obj_info arm_info[], pot_info[], ring_info[],
			scr_info[], things[], ws_info[], weap_info[];

extern struct delayed_action d_list[MAXDAEMONS];

extern const STONE    stones[];

/*
 * Function types
 */

void	_attach(THING **list, THING *item);
void	_detach(THING **list, THING *item);
void	_free_list(THING **ptr);
void	addmsg(const char *fmt, ...);
int 	add_haste(int potion);
int	add_line(const char *fmt, const char *arg);
void	add_pack(THING *obj, int silent);
void	add_pass(void);
void	add_str(int *sp, int amt);
void	accnt_maze(int y, int x, int ny, int nx);
void	aggravate(void);
int	attack(THING *mp);
void    auto_save(int);
void	badcheck(const char *name, const struct obj_info *info, int bound);
int 	be_trapped(const coord *tc);
void	bounce(const THING *weap, const char *mname, int noend);
void	call(void);
void	call_it(struct obj_info *info);
int 	cansee(int y, int x);
int	center(const char *str);
int	chase(THING *tp, const coord *ee);
int	checkout(void);
const char *charge_str(const THING *obj);
void	chg_str(int amt);
void	check_level(void);
const char *choose_str(const char *ts, const char *ns);
void	conn(int r1, int r2);
void	come_down(void);
void	command(void);
void	create_obj(void);
void	current(const THING *cur, const char *how, const char *where);
void	d_level(void);
void	death(int monst);
int 	death_monst(void);
int	diag_ok(const coord *sp, const coord *ep);
void	dig(int y, int x);
void	discard(THING *item);
void	discovered(void);
int	dist(int y1, int x1, int y2, int x2);
int	dist_cp(const coord *c1, const coord *c2);
int	do_chase(THING *th);
void	do_daemons(int flag);
void	do_fuses(int flag);
void	do_maze(const struct room *rp);
void	do_motion(THING *obj, int ydelta, int xdelta);
void	do_move(int dy, int dx);
void	do_passages(void);
void	do_pot(int type, int knowit);
void	do_rooms(void);
void	do_run(int ch);
void	do_zap(void);
void	doadd(const char *fmt, va_list args);
void	doctor(void);
void	door(struct room *rm, const coord *cp);
void	door_open(const struct room *rp);
void	drain(void);
void	draw_room(const struct room *rp);
void	drop(void);
int 	dropcheck(const THING *obj);
void	eat(void);
int     encclearerr();
int     encerror();
void    encseterr();
size_t  encread(char *start, size_t size, FILE *inf);
size_t	encwrite(const char *start, size_t size, FILE *outf);
void    end_line(void);
void    endit(int sig);
int	endmsg(void);
void	enter_room(const coord *cp);
void	erase_lamp(const coord *pos, const struct room *rp);
int	exp_add(const THING *tp);
void	extinguish(void (*func)());
void	fall(THING *obj, int pr);
int	fallpos(const coord *pos, coord *newpos);
void	fatal(const char *s);
void	fire_bolt(const coord *start, coord *dir, const char *name);
int 	floor_at(void);
int	floor_ch(void);
void	flush_type(void);
const coord *find_dest(const THING *tp);
int 	find_floor(const struct room *rp, coord *cp, int limit, int monst);
THING   *find_obj(int y, int x);
int	fight(const coord *mp, const THING *weap, int thrown);
void	fix_stick(THING *cur);
void	fuse(void (*func)(), int arg, int time, int type);
int	get_bool(void *vp, WINDOW *win);
int	get_dir(void);
int	get_inv_t(void *vp, WINDOW *win);
THING   *get_item(const char *purpose, int type);
int	get_num(void *vp, WINDOW *win);
int	get_sf(void *vp, WINDOW *win);
int	get_str(void *vopt, WINDOW *win);
int	gethand(void);
void	getltchars(void);
void	give_pack(THING *tp);
void	help(void);
void	hit(const char *er, const char *ee, int noend);
void	horiz(const struct room *rp, int starty);
void	leave_room(const coord *cp);
void	lengthen(void (*func)(), int xtime);
void	look(int wakeup);
int	hit_monster(int y, int x, const THING *obj);
void	identify(void);
void	illcom(int ch);
void	init_check(void);
void	init_colors(void);
void	init_materials(void);
void	init_names(void);
void	init_player(void);
void	init_probs(void);
void	init_stones(void);
void	init_weapon(THING *weap, int which);
char	*inv_name(const THING *obj, int drop);
int	inventory(const THING *list, int type);
void	invis_on(void);
int	is_current(const THING *obj);
int 	is_magic(const THING *obj);
int     is_symlink(const char *sp); 
void	kill_daemon(void (*func)());
void	killed(THING *tp, int pr);
const char *killname(int monst, int doart);
void	land(void);
void    leave(int);
THING   *leave_pack(THING *obj, int newobj, int all);
int 	levit_check(void);
int	lock_sc(void);
void	miss(const char *er, const char *ee, int noend);
void	missile(int ydelta, int xdelta);
void	money(int value);
int	move_monst(THING *tp);
void	move_msg(const THING *obj);
int	msg(const char *fmt, ...);
void	my_exit(int sig);
void	nameit(const THING *obj, const char *type, const char *which, const struct obj_info *op, const char *(*prfunc)(const THING *));
THING   *new_item(void);
void	new_level(void);
void	new_monster(THING *tp, int type, const coord *cp);
THING   *new_thing(void);
void	nohaste(void);
const char *nothing(int type);
const char *nullstr(const THING *ignored);
const char *num(int n1, int n2, int type);
void	numpass(int y, int x);
void	option(void);
void	open_score(void);
int	pack_char(void);
int	pack_room(int from_floor, THING *obj);
void	parse_opts(char *str);
void 	passnum(void);
int	passwd(void);
const char *pick_color(const char *col);
int	pick_one(const struct obj_info *info, int nitems);
void	pick_up(int ch);
void	picky_inven(void);
void	playit(void);
void    playltchars(void);
void	pr_spec(const struct obj_info *info, int nitems);
void	pr_list(void);
void	print_disc(int);
void	put_bool(void *b);
void	put_inv_t(void *ip);
void	put_str(void *str);
void	put_things(void);
void	putpass(const coord *cp);
void	quaff(void);
void    quit(int);
void	raise_level(void);
int 	randmonster(int wander);
void	read_scroll(void);
int	readchar(void);
void    relocate(THING *th, const coord *new_loc);
void	remove_mon(const coord *mp, THING *tp, int waskill);
void	reset_last(void);
void    resetltchars(void);
int     restore(const char *file);
int	ring_eat(int hand);
void	ring_on(void);
void	ring_off(void);
const char *ring_num(const THING *obj);
int	rnd(int range);
int	rnd_room(void);
int	rnd_thing(void);
coord	rndmove(const THING *who);
int	roll(int number, int sides);
int	roll_em(const THING *thatt, THING *thdef, const THING *weap, int hurl);
void	rollwand(void);
struct room *roomin(const coord *cp);
int	rs_save_file(FILE *savef);
int	rs_restore_file(FILE *inf);
void	runners(void);
void	runto(const coord *runner);
void	rust_armor(THING *arm);
int	save(int which);
void	save_file(FILE *savef);
void	save_game(void);
int	save_throw(int which, const THING *tp);
void	score(int amount, int flags, int monst);
void	search(void);
int	see_monst(const THING *mp);
int	seen_stairs(void);
void	set_know(THING *obj, struct obj_info *info);
const char *set_mname(const THING *tp);
void	set_oldch(THING *tp, const coord *cp);
void    set_order(int *order, int numthings);
void	setup(void);
void	shell(void);
int 	show_floor(void);
void	show_map(void);
void	show_win(const char *message);
void	sight(void);
int	sign(int nm);
int	spread(int nm);
void	start_daemon(void (*func)(), int arg, int type);
void	start_score(void);
void	status(void);
int	step_ok(int ch);
void	stomach(void);
void	strucpy(char *s1, const char *s2, size_t len);
void	swander(void);
int	swing(int at_lvl, int op_arm, int wplus);
void	take_off(void);
void	teleport(void);
void	total_winner(void);
void	thunk(const THING *weap, const char *mname, int noend);
void	treas_room(void);
int	trip_ch(int y, int x, int ch);
void	tstp(int ignored);
int     turn_ok(int y, int x);
int	turn_see(int turn_off);
void	turnref(void);
const char *type_name(int type);
void	u_level(void);
void	unconfuse(void);
void	uncurse(THING *obj);
void	unlock_sc(void);
void	unsee(void);
void	vert(const struct room *rp, int startx);
void	visuals(void);
char	*vowelstr(const char *str);
void	wait_for(WINDOW *win, int ch);
const THING *wake_monster(int y, int x);
void	wanderer(void);
void	waste_time(void);
void	wear(void);
void	whatis(int insist, int type);
void	wield(void);
int	wreadchar(WINDOW *win);

