/*
 * Structure and defines for the swint call
 */

struct sw_regs {
	int	ax;
	int	bx;
	int	cx;
	int	dx;
	int	si;
	int	di;
	int ds;
	int	es;
};

#define	SW_DOS	0x21
#define	SW_KEY	0x16
#define	SW_SCR	0x10
#define SW_DSK	0x13

#define LOW(x) ((x) & 0xff)
#define HI(x) ((x) >> 8)

#define SETHILO(reg,hi,lo) (reg) = ((hi) << 8) | lo)

extern struct sw_regs *regs;

/* processor flags */
#define CF	0x001
#define PF	0x004
#define AF	0x010
#define ZF	0x040
#define SF	0x080
#define TF	0x100
#define IF	0x200
#define DF	0x400
#define OF	0x800
