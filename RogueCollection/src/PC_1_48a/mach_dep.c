/*
 * Various installation dependent routines
 *
 * mach_dep.c	1.4 (A.I. Design) 12/1/84
 */

#include	"rogue.h"
#include	"curses.h"
#include	"keypad.h"

#define ULINE() if(is_color) lmagenta();else uline();
#define TICK_ADDR 0x70
static int clk_vec[2];
static int ocb;


/*
 * setup:
 *	Get starting setup for all games
 */
setup()
{
	terse = FALSE;
	maxrow = 23;
	if (COLS == 40) {
		maxrow = 22;
		terse = TRUE;
	}
	expert = terse;
	/*
	 * Vector CTRL-BREAK to call quit()
	 */
	COFF();
	ocb = set_ctrlb(0);
}

clock_on()
{
	extern int _csval, clock(), (*cls_)(), no_clock();
	int new_vec[2];

	new_vec[0] = clock;
	new_vec[1] = _csval;
	dmain(clk_vec, 2, 0, TICK_ADDR); 
	dmaout(new_vec, 2, 0, TICK_ADDR);
	cls_ = no_clock;
}

no_clock()
{
	dmaout(clk_vec, 2, 0, TICK_ADDR);
}

/*
 * returns a seed for a random number generator
 */
srand()
{
#ifdef DEBUG
	return ++dnum;
#else
	/*
	 * Get Time
	 */
	bdos(0x2C);
	return(regs->cx + regs->dx);
#endif
}


/*
 * flush_type:
 *	Flush typeahead for traps, etc.
 */
flush_type()
{
#ifdef CRASH_MACHINE
	regs->ax = 0xc06;		/* clear keyboard input */
	regs->dx = 0xff;		/* set input flag */
	swint(SW_DOS, regs);
#endif CRASH_MACHINE
	typeahead = "";
}

credits()
{
	int i;
	char tname[25];

	cursor(FALSE);
	clear();
	if (is_color)
	    brown();
	box(0,0,LINES-1,COLS-1);
	bold();
	center(2,"ROGUE:  The Adventure Game");
	ULINE();
	center(4,"The game of Rogue was designed by:");
	high();
	center(6,"Michael Toy and Glenn Wichman");
	ULINE();
	center(9,"Various implementations by:");
	high();
	center(11,"Ken Arnold, Jon Lane and Michael Toy");
	ULINE();
#ifdef INTL
	center(14,"International Versions by:");
#else
	center(14,"Adapted for the IBM PC by:");
#endif
	high();
#ifdef INTL
	center(16,"Mel Sibony");
#else
	center(16,"A.I. Design");
#endif
	ULINE();
	if (is_color)
	    yellow();
	center(19,"(C)Copyright 1985");
	high();
#ifdef INTL
	center(20,"AI Design");
#else
	center(20,"Epyx Incorporated");
#endif
    standend();
	if (is_color)
	    yellow();
	center(21,"All Rights Reserved");
	if (is_color)
		brown();
	for(i=1;i<(COLS-1);i++) {
		move(22,i);
		putchr(205);
	}
	mvaddch(22,0,204);
	mvaddch(22,COLS-1,185);
	standend();
	mvaddstr(23,2,"Rogue's Name? ");
	is_saved = TRUE;		/*  status line hack  */
	high();
	getinfo(tname,23);
	if (*tname && *tname != ESCAPE) 
		strcpy(whoami, tname);
	is_saved = FALSE;
	blot_out(23,0,24,COLS-1);
	if (is_color)
	    brown();
	mvaddch(22,0,0xc8);
	mvaddch(22,COLS-1,0xbc);
	standend();
}

/*
 * Table for IBM extended key translation
 */
static struct xlate {
	byte keycode, keyis;
} xtab[] = {
	C_HOME, 'y', C_UP,	'k', C_PGUP,'u', C_LEFT,	'h', C_RIGHT,	'l',
	C_END,	'b', C_DOWN,'j', C_PGDN,'n', C_INS,		'>', C_DEL,		's',
	C_F1,	'?', C_F2,	'/', C_F3,	'a', C_F4,	CTRL(R), C_F5,		'c',
	C_F6,	'D', C_F7,	'i', C_F8,	'^', C_F9,	CTRL(F), C_F10,		'!',
	ALT_F9,	'F'
};

/*
 * readchar:
 *	Return the next input character, from the macro or from the keyboard.
 */
readchar()
{
	register struct xlate *x;
	register byte ch;
		
    if (*typeahead) {
        SIG2();
        return(*typeahead++);
    }
    /*
     * while there are no characters in the type ahead buffer
     * update the status line at the bottom of the screen
     */
    do
        SIG2();				/* Rogue spends a lot of time here */
    while (no_char());
	/*
	 * Now read a character and translate it if it appears in the
	 * translation table
	 */
	for (ch = getch(), x = xtab; x < xtab + (sizeof xtab) / sizeof *xtab; x++)
		if (ch == x->keycode) {
			ch = x->keyis;
			break;
		}
    if (ch == ESCAPE)
        count = 0;
    return ch;
}

bdos(fnum, dxval)
	int fnum, dxval;
{
	register struct sw_regs *saveptr;

	regs->ax = fnum << 8;
	regs->bx = regs->cx = 0;
	regs->dx = dxval;
	saveptr = regs;
	swint(SW_DOS,regs);
	regs = saveptr;
	return(0xff & regs->ax);
}

/*
 *  newmem - memory allocater
 *         - motto: allocate or die trying
 */
newmem(nbytes,clrflag)
	unsigned nbytes;
	int clrflag;
{
	register char *newaddr;

	newaddr = sbrk(nbytes);
	if (newaddr == -1)
		fatal("No Memory");
	end_mem = newaddr + nbytes;
	if ((unsigned)end_mem & 1)
		end_mem = sbrk(1);
	return(newaddr);
}

#define PC	0xff
#define XT  0xfe
#define JR  0xfd
#define AT	0xfc

isjr()
{
	static int machine = 0;

	if (machine == 0) {
		dmain(&machine,1,0xf000,0xfffe);
		machine &= 0xff;
	}
	return machine == JR;
}

swint(intno, rp)
int intno;
struct sw_regs *rp;
{
	extern int _dsval;

	rp->ds = rp->es = _dsval;
	sysint(intno, rp, rp);
	return rp->ax;
}

set_ctrlb(state)
{
	struct sw_regs rg;
	int retcode;

	rg.ax = 0x3300;
	swint(SW_DOS,&rg);
	retcode = rg.dx &0xFF;

	rg.ax = 0x3300;
	rg.dx = (state) ? 1 : 0;
	swint(SW_DOS,&rg);

	return retcode;
}

unsetup()
{
	set_ctrlb(ocb);
}

one_tick()
{
	extern int tick;
	int otick = tick;
	int i=0,j=0;

	while(i++)
		while (j++)
			if (otick != tick)
				return;
			else if (i > 2)
				_halt();
}
