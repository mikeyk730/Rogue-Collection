//setenv: read in environment from a file
//envfile - name of file that contains data to be put in the environment
//STATUS - setenv return -1 if the file does not exist or if there is not enough memory to expand the environment
int setenv(char *envfile);
