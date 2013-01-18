//Peekc: Return the next char associated with efd (environment file descriptor)
//This routine has some knowledge of the file parsing state so that it knows if there has been a premature eof.  This way I can avoid checking for premature eof every time a character is read.
int peekc();

//setenv: read in environment from a file
//envfile - name of file that contains data to be put in the environment
//STATUS - setenv return -1 if the file does not exist or if there is not enough memory to expand the environment
int setenv(char *envfile);

int is_set(char *label, char *string);