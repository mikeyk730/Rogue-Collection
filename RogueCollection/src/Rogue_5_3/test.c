#ifndef MDK
#include <stdio.h>
#include <values.h>

main() {
	FILE junk;
	char *istring;
	char ostring[256];
	char *args;
	int fmt;

	istring = "this is a test\n";

	fmt = istring;
	args = (char *)0;
	junk._cnt = MAXINT;
	junk._base = junk._ptr = ostring;
	junk._flag = _IOWRT;
	junk._file = _NFILE;
	_doprnt(fmt, args, &junk);
	printf("ostring=%s", ostring);
	}
#endif