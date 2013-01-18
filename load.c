//This file contains the code to display the rogue picture files
//load.c      1.42    (A.I. Design)   2/12/84

#include "rogue.h"
#include "load.h"

#define MODESAVE  0x65
#define MODEREG  0x3d8
#define BWENABLE  0x4
#define COLREG  0x3d9
#define HIGHENABLE  0x10
#define PALETTEBIT  0x20

static char *store;
static int blksize = 0x4000, lfd;

epyx_yuck()
{
//  extern unsigned int tick;
//  int type = get_mode();
//
//  if (type==7 || (lfd = open("rogue.pic", 0))<0) return;
//  while ((int)(store = sbrk(blksize))==-1) blksize /= 2;
//  video_mode(4);
//  scr_load();
//  tick = 0;
//#ifdef LOGFILE
//  while (tick<18*10) ;
//#else
//  while (no_char() && tick<18*60*5) ;
//  if (!no_char()) readchar();
//#endif
//  video_mode(type);
//  brk(store);
//  tick = 0;
}

scr_load()
{
  //int palette, background;
  //int mode, burst;

  //bload(0xb800);
  //palette = peekb(8012, 0xB800);
  //background = peekb(8013, 0xB800);
  //if (palette>=3) background |= HIGHENABLE;
  //burst = 0;
  //switch (palette)
  //{
  //  case 2: case 5: burst = 1;
  //  case 0: case 3: palette = 1; break;
  //  case 1: case 4: palette = 0; break;
  //}
  //out(COLREG, background);
  //mode = peekb(MODESAVE, 0x40)&(~BWENABLE);
  //if (burst==1) mode = mode|BWENABLE;
  //pokeb(MODESAVE, 0x40, mode);
  //out(MODEREG, mode);
}

bload(unsigned int segment)
{
  //unsigned int offset = 0, rdcnt;

  //if (read(lfd, store, 7)<=0) lseek(lfd, 7L, 0); //Ignore first seven bytes
  //while ((rdcnt = read(lfd, store, blksize))>0)
  //{
  //  dmaout(store, rdcnt/2, segment, offset);
  //  if ((offset += rdcnt)>=16384) break;
  //}
}

find_drive()
{
  //int drive = bdos(0x19);
  //char spec = s_drive[0];
  //char filename[30];

  //if (isalpha(spec))
  //{
  //  if (isupper(spec)) drive = spec-'A';
  //  else drive = spec-'a';
  //}
  //strcpy(filename, "a:jatgnas.8ys");
  //filename[0] += (char)drive;
  //access(filename);
  //return drive;
}
