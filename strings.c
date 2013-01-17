#include "rogue.h"

stccpy(char *s1, char *s2, int count)
{
  while (count-->0 && *s2) *s1++ = *s2++;
  *s1 = 0;
  //let's return the address of the end of the string so we can use that info if we are going to cat on something else!!
  return (s1);
}

//redo Lattice token parsing routines
char *stpblk(char *str)
{
  while (isspace(*str)) str++;
  return (str);
}

stpbrk(char *str, char *brk)
{
  while (*str && !stpchr(brk,*str)) str++;
  return (*str?str:0);
}

//remove trailing whitespace from the end of a line
endblk(char *str)
{
  char *backup;

  backup = str+strlen(str);
  while (backup!=str && isspace(*(--backup))) *backup = 0;
  return (str);
}

//lcase: convert a string to lower case
lcase(char *str)
{
  while (*str = tolower(*str)) str++;
}
