extern char ctp_[];

isalpha(int x)
{
  return x>128?0:(ctp_[(x)+1]&0x03);
}

isupper(int x)
{
  return x>128?0:(ctp_[(x)+1]&0x01);
}

islower(int x)
{
  return x>128?0:(ctp_[(x)+1]&0x02);
}

isdigit(int x)
{
  return x>128?0:(ctp_[(x)+1]&0x04);
}

isspace(int x)
{
  return x>128?0:(ctp_[(x)+1]&0x10);
}

isprint(int x)
{
  return x>128?0:(ctp_[(x)+1]&0xc7);
}

toascii(int x)
{
  return (x&127);
}

toupper(char chr)
{
  return (islower(chr)?((chr)-('a'-'A')):(chr));
}

tolower(char chr)
{
  return (isupper(chr)?((chr)+('a'-'A')):(chr));
}

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
  while (*str && !index(brk,*str)) str++;
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
