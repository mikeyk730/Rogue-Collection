/*
 * Read and execute the user comamnds
 *
 * command.c	1.44	(A.I. Design)	2/14/85
 */

#include	"rogue.h"
#include	"curses.h"

static int lastcount;
static byte lastch, do_take, lasttake;

command()
{
	register int ntimes;

	if (on(player, ISHASTE))
		ntimes = rnd(2) + 2;
	else
		ntimes = 1;
	while (ntimes--) {
		status();
		SIG2();
#ifdef WIZARD
		if (wizard)
			noscore = TRUE;
#endif
		if (no_command) {
			if (--no_command <= 0) {
				msg("you can move again");
				no_command = 0;
			}
		} else
			execcom();
		do_fuses();
		do_daemons();
		for (ntimes = LEFT; ntimes <= RIGHT; ntimes++)
			if (cur_ring[ntimes])
				switch (cur_ring[ntimes]->o_which) {
				when R_SEARCH:
					search();
				when R_TELEPORT:
					if (rnd(50) == 17)
						teleport();
				}
	}
}

com_char()
{
	register int same, ch;

	same = (fastmode == faststate);
	ch = readchar();
	if (same)
		fastmode = faststate;
	else
		fastmode = !faststate;
	switch (ch) {
		when '\b': ch = 'h';
		when '+': ch = 't';
		when '-': ch = 'z';
	}
	if (mpos && !running)
		msg("");
	return ch;
}

/*
 * Read a command, setting thing up according to prefix like devices
 * Return the command character to be executed.
 */

get_prefix()
{
	register int retch, ch, junk;

	after = TRUE;
	fastmode = faststate;
	look(TRUE);
	if (!running)
		door_stop = FALSE;
	do_take = TRUE;
	again = FALSE;
	if (--count > 0) {
		do_take = lasttake;
		retch = lastch;
		fastmode = FALSE;
	} else {
		count = 0;
		if (running) {
			retch = runch;
			do_take = lasttake;
		} else {
			for (retch = 0; retch == 0; ) {
				switch (ch = com_char()) {
					case '0': case '1': case '2': case '3': case '4':
					case '5': case '6': case '7': case '8': case '9':
						junk = count * 10;
						if ((junk += ch - '0') > 0 && junk < 10000)
							count = junk;
						show_count();
					when 'f':
						fastmode = !fastmode;
					when 'g':
						do_take = FALSE;
					when 'a':
						retch = lastch;
						count = lastcount;
						do_take = lasttake;
						again = TRUE;
					when ' ':	/* Spaces are ignored */
					when ESCAPE:
						door_stop = FALSE;
						count = 0;
						show_count();
					otherwise:
						retch = ch;
				}
			}
		}
	}
	if (count)
		fastmode = FALSE;
	switch (retch) {
	case 'h': case 'j': case 'k': case 'l':
	case 'y': case 'u': case 'b': case 'n':
		if (fastmode && !running ) {
			if (!on(player, ISBLIND)) {
				door_stop = TRUE;
				firstmove = TRUE;
			}
			retch = toupper(retch);
		}
	case 'H': case 'J': case 'K': case 'L':
	case 'Y': case 'U': case 'B': case 'N':
	case 'q': case 'r': case 's': case 'z':
	case 't': case '.':
#ifdef WIZARD
	case CTRL(D): case 'C':
#endif WIZARD
		break;
	default:
		count = 0;
	}
	if (count || lastcount)
		show_count();
	lastch = retch;
	lastcount = count;
	lasttake = do_take;
	return retch;
}

show_count()
{
	move(LINES-2, COLS-4);
	if (count)
		printw("%-4d", count);
	else
		addstr("    ");
}

execcom()
{
	coord mv;
	register int ch;

	do {
		switch (ch = get_prefix()) {
		when 'h': case 'j': case 'k': case 'l':
		case 'y': case 'u': case 'b': case 'n':
			find_dir(ch, &mv);
			do_move(mv.y, mv.x);
		when 'H': case 'J': case 'K': case 'L':
		case 'Y': case 'U': case 'B': case 'N':
			do_run(tolower(ch));
		when 't':
			if (get_dir())
				missile(delta.y, delta.x);
			else
				after = FALSE;
		when 'Q': after = FALSE; quit();
		when 'i': after = FALSE; inventory(pack, 0, "");
		when 'd': drop();
		when 'q': quaff();
		when 'r': read_scroll();
		when 'e': eat();
		when 'w': wield();
		when 'W': wear();
		when 'T': take_off();
		when 'P': ring_on();
		when 'R': ring_off();
		when 'c': after = FALSE; call();
		when '>': after = FALSE; d_level();
		when '<': after = FALSE; u_level();
		when '/': after = FALSE; help(helpobjs);
		when '?': after = FALSE; help(helpcoms);
		when '!': after = FALSE; fakedos();
		when 's': search();
		when 'z':
			if (get_dir())
				do_zap();
			else
				after = FALSE;
		when 'D': after = FALSE; discovered();
		when CTRL(T):
			after = FALSE;
			msg((expert ^= 1)
				? "Ok, I'll be brief"
				: "Goodie, I can use big words again!");
		when 'F': after = FALSE; do_macro(macro, MACROSZ);
		when CTRL(F): after = FALSE; typeahead = macro;
		when CTRL(R): after = FALSE; msg(huh);
		when 'v':
			after = FALSE;
			if (strcmp(whoami,"The Grand Beeking") == 0)
				addmsg("(%d)",csum());
			msg("Rogue version %d.%d (Mr. Mctesq was here)", revno, verno);
		when 'S': after = FALSE; save_game();
		when '.': doctor();
		when '^':
			after = FALSE;
			if (get_dir()) {
				coord lookat;

				lookat.y = hero.y + delta.y;
				lookat.x = hero.x + delta.x;
				if (chat(lookat.y, lookat.x) != TRAP)
					msg("no trap there.");
				else
					msg("you found %s",
						tr_name(flat(lookat.y, lookat.x) & F_TMASK));
			}
		when 'o': after = FALSE; msg("i don't have any options, oh my!");
		when CTRL(L):
			after = FALSE;
			msg("the screen looks fine to me (jll was here)");
#ifdef WIZARD
		when 'C': after = FALSE; create_obj();
#endif
		otherwise:
			after = FALSE;
			save_msg = FALSE;
			msg("illegal command '%s'", unctrl(ch));
			count = 0;
			save_msg = TRUE;
		}
		if (take && do_take)
			pick_up(take);
		take = 0;
		if (!running)
			door_stop = FALSE;
	} while (after == FALSE);
}
