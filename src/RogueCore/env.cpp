//Env.c: routines to set up environment
//Jon Lane  -  10/31/83

#include <io.h>
#include <ctype.h>
#include <string.h>

#include "rogue.h"
#include "game_state.h"
#include "env.h"
#include "strings.h"
#include "main.h"

#define MAXEP  9

static int fd;
static int ch;
static int pstate;
static char blabel[256], bstring[256], barg[36];
static char *plabel, *pstring;

//Putenv: Put something into the environment. label - label of thing in environment. string - string associated with the label
//No meaningful return codes to save data space. Just ignores strange labels
void putenv2(char *label, char *string)
{
    game->set_environment(label, string);
}

//Peekc: Return the next char associated with efd (environment file descriptor)
//This routine has some knowledge of the file parsing state so that it knows if there has been a premature eof.  This way I can avoid checking for premature eof every time a character is read.
int peekc()
{
    ch = 0;
    //we make sure that the strings never get filled past the end, this way we only have to check for these things once
    if (plabel > &blabel[255]) 
        plabel = &blabel[255];
    if (pstring > &bstring[255]) 
        pstring = &bstring[255];
    if (_read(fd, &ch, 1) < 1 && pstate != 0)
    {
        //When looking for the end of the string, Let the eof look like newlines
        if (pstate >= 2) return('\n');
        fatal("rogue.opt: incorrect file format\n");
    }
    if (ch == 26) ch = '\n';
    return (ch);
}

//setenv: read in environment from a file
//envfile - name of file that contains data to be put in the environment
//STATUS - setenv return -1 if the file does not exist or if there is not enough memory to expand the environment
int setenv(const char *envfile)
{
    char pc;

    if ((fd = _open(envfile, 0)) < 0)
        return(-1);

    while (1)
    {
        //Look for another label
        pstate = 0;
        plabel = blabel;
        pstring = bstring;
        //Skip white space, this is the only state (pstate == 0) where eof will not be aborted
        while (isspace(peekc()));
        if (ch == 0) { _close(fd); break; }
        pstate = 3;
        //Skip comments.
        if (ch == '#' || ch == ';')
        {
            while (peekc() != '\n');
            continue;
        }
        pstate = 1;
        //start of label found
        *plabel = ch;
        while ((pc = peekc()) != '=' && pc != '-') if (!isspace(*plabel) || !isspace(ch)) *(++plabel) = ch;
        if (!isspace(*plabel)) plabel++;
        *plabel = 0;
        //Looking for corresponding string
        while (isspace(peekc()));
        //Start of string found
        pstate = 2;
        *pstring = ch;
        while (peekc() != '\n') if (!isspace(*pstring) || !isspace(ch)) *(++pstring) = ch;
        if (!isspace(*pstring)) pstring++;
        *pstring = 0;
        lcase(blabel);
        putenv2(blabel, bstring);
    }

    game->process_environment();
    return 0;
}
