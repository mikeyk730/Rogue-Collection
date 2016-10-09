/*
 *  Cursor motion header for Monochrome display
 */

#define abs(x) ((x)<0?-(x):(x))
#define NULL	0
#define TRUE 	1
#define	FALSE	0

#define refresh	stub

#define	curscr	NULL
#define	stdscr	NULL
#define hw	NULL

#define	BUFSIZ	128

#define wmove(a,b,c)	move(b,c)

#define	wclear	clear
#define fputs(a,b)	addstr(a)
#define puts(s)	addstr(s)

#define wrefresh	stub
#define	clearok		stub
#define leaveok		stub
#define	endwin		wclose
#define touchwin	stub

#define gets	cgets

#define waddstr(w,s)	addstr(s)
#define mvwaddstr(w,a,b,c)	mvaddstr(a,b,c)
#define mvwaddch(w,a,b,c)	mvaddch(a,b,c)

#define getyx(a,b,c)	getxy(&b,&c)
#define getxy	getrc

#define	inch()	(0xff&curch())

#define is_color (scr_type!=7)
#define is_bw (scr_type==0 || scr_type==2)

#define standend() set_attr(0)
#define green() set_attr(1)
#define cyan() set_attr(2)
#define red() set_attr(3)
#define magenta() set_attr(4)
#define brown() set_attr(5)
#define dgrey() set_attr(6)
#define lblue() set_attr(7)
#define lgrey() set_attr(8)
#define lred() set_attr(9)
#define lmagenta() set_attr(10)
#define yellow() set_attr(11)
#define uline() set_attr(12)
#define blue() set_attr(13)
#define standout() set_attr(14)
#define high() set_attr(15)
#define bold() set_attr(16)

extern int scr_ds, old_page_no, no_check;

#define BX_UL	0
#define BX_UR	1
#define BX_LL	2
#define BX_LR	3
#define BX_VW	4
#define BX_HT	5
#define BX_HB	6
#define BX_SIZE	7

extern byte dbl_box[BX_SIZE], sng_box[BX_SIZE], fat_box[BX_SIZE];
