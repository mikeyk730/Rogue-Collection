//save and restore routines
//save.c      1.32    (A.I. Design)   12/13/84

//routines for saving a program in any given state. This is the first pass of this, so I have no idea how it is really going to work.
//The two basic functions here will be "save" and "restor".

#include "rogue.h"

#define MIDSIZE  10
//BLKSZ: size of block read/written on each io operation. Has to be less than 4096 and a factor of 4096 so the screen can be read in exactly.
#define BLKSZ  512
#define printf  printw

//we need to know location of screen being saved
extern char *savewin;
extern int errno;
extern char _lowmem; //Address of first save-able memory
extern char _Uend; //Address of end of user data space
extern char do_force;
extern int bwflag;

char *msaveid = "AI Design";

//save_game: Implement the "save game" command
save_game()
{
//#ifndef DEMO
//
//  int retcode;
//  char savename[20];
//
//  msg("");
//  mpos = 0;
//  if (terse) addstr("Save file ? ");
//  else printw("Save file (press enter (\x11\xd9) to default to \"%s\") ? ", s_save);
//  retcode = getinfo(savename, 19);
//  if (*savename==0) strcpy(savename, s_save);
//  msg("");
//  mpos = 0;
//  if (retcode!=ESCAPE)
//  {
//    if ((retcode = save_ds(savename))==-1)
//    {
//      if (unlink(savename)==0) ifterse1("out of space?", "out of space, can not write %s", savename);
//      msg("Sorry, you can't save the game just now");
//      is_saved = FALSE;
//    }
//    else if (retcode>0) fatal("\nGame saved as %s.", savename);
//  }
//
//#endif
}

#ifndef DEMO

//Save: Determine the entire data area that needs to be saved, Open save file, first write in to save file a header that dimensions the data area that will be saved, and then dump data area determined previous to opening file.
save_ds(char *savename)
{
//  int sfd;
//  char answer;
//
//  if ((sfd = open(savename, 0))>=0)
//  {
//    close(sfd);
//    msg("%s %sexists, overwrite (y/n) ?", savename, noterse("already "));
//    answer = readchar();
//    msg("");
//    if ((answer!='y') && (answer!='Y')) return (-2);
//  }
//  if ((sfd = creat(savename, 0666))<=0) {msg("Could not creat %s", savename); return (-2);}
//  is_saved = TRUE;
//  mpos = 0;
//  errno = 1;
//  if (write(sfd, msaveid, MIDSIZE)!=MIDSIZE || write(sfd, &_lowmem, &_Uend-&_lowmem)!=&_Uend-&_lowmem || write(sfd, end_sb, startmem-end_sb)!=startmem-end_sb) goto wr_err;
//  //save the screen (have to bring it into current data segment first)
//  wdump();
//  if (write(sfd, savewin, 4000)==4000) errno = 0;
//  wrestor();
//
//wr_err:
//
//  close(sfd);
//  switch (errno)
//  {
//    default: msg("Could not write savefile to disk!"); return -1;
//    case 0: move(24, 0); clrtoeol(); move(23, 0); return 1;
//  }
}

#endif DEMO

//Restore: Open saved data file, read in header, and determine how much data area is going to be restored.
//Close save data file, Allocate enough data space so that open data file information will be stored outside the data area that will be restored.
//Now reopen data save file, skip header, dump into memory all saved data.

restore(char *savefile)
{
#ifndef DEMO

//  int oldrev, oldver, old_check;
//  int oldcols, fd;
//  char errbuf[11], save_name[MAXSTR];
//  char *read_error = "Read Error";
//  struct sw_regs *oregs;
//  unsigned int nbytes;
//  char idbuf[MIDSIZE];
//
//  oregs = regs;
//  winit();
//  if (bwflag) forcebw();
//  if (no_check==0) no_check = do_force;
//  old_check = no_check;
//  strcpy(errbuf, read_error);
//  //save things that will be bombed on when the restor takes place
//  oldrev = revno;
//  oldver = verno;
//  if (!strcmp(s_drive, "?"))
//  {
//    int ot = scr_type;
//
//    printw("Press space to restart game");
//    scr_type = -1;
//    wait_for(' ');
//    scr_type = ot;
//    addstr("\n");
//  }
//  if ((fd = open(savefile, 0))<0) fatal("%s not found\n", savefile);
//  else printf("Restoring %s", savefile);
//  strcpy(save_name, savefile);
//  nbytes = &_Uend-&_lowmem;
//  if (read(fd, idbuf, MIDSIZE)!=MIDSIZE || strcmp(idbuf, msaveid)) addstr("\nNot a savefile\n");
//  else
//  {
//    if (read(fd, &_lowmem, nbytes)==nbytes) if (read(fd, end_sb, nbytes = startmem-end_sb)==nbytes) goto rok;
//    addstr(errbuf);
//  }
//  close(fd);
//  exit();
//
//rok:
//
//  regs = oregs;
//  if (revno!=oldrev || verno!=oldver)
//  {
//    close(fd);
//    exit();
//  }
//  oldcols = COLS;
//  brk(end_sb); //Restore heap to empty state
//  init_ds();
//  wclose();
//  winit();
//  if (oldcols!=COLS) {close(fd); fatal("Restore Error: new screen size\n");}
//  wdump();
//  if (read(fd, savewin, 4000)!=4000) {close(fd); fatal("Serious restore error");}
//  wrestor();
//  close(fd);
//  no_check = old_check;
//  mpos = 0;
//  ifterse1("%s, Welcome back!", "Hello %s, Welcome back to the Dungeons of Doom!", whoami);
//  dnum = srand(); //make it a little tougher on cheaters
//  unlink(save_name);

#endif DEMO
}
