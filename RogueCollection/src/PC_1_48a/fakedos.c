/*
 * routines for writing a fake dos
 */

#include	"rogue.h"
#include	"curses.h"

char *stpblk();

fakedos()
{
    char comline[132];
    register char *savedir = "a:", *comhead;

	wdump();
	clear();
	move (0,0);
	cursor(TRUE);
	*savedir = bdos(0x19,0) + 'A';
	do {
		setmem(comline, sizeof comline, 0);
	    printw("\n%c>",bdos(0x19,0)+'A');
        getinfo(comline,130);
        comhead = stpblk(comline);
        endblk(comhead);
    } while (dodos(comhead));
	dodos(savedir);    
    cursor(FALSE);
    wrestor();
}

/*
 * execute a dos like command
 */
dodos(com)
    char *com;
{
	if ((*com & 0x80) || (strcmp(com, "rogue") == 0))
		return 0;
    if (com[1] == ':' && com[2] == 0) {
    	int drv = (*com & 0x1f) - 1;

		printw("\n");
		if ((!isalpha(*com)) || drv >= bdos(0x0e, drv))
		    printw("Invalid drive specification\n");
    } else  if (com[0])
	    printw("\nBad command or file name\n");
	return 1;
}
