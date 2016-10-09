/*
    mdport.c - Machine Dependent Code for Porting Unix/Curses games

    Copyright (C) 2005 Nicholas J. Kisseberth
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions
    are met:
    1. Redistributions of source code must retain the above copyright
       notice, this list of conditions and the following disclaimer.
    2. Redistributions in binary form must reproduce the above copyright
       notice, this list of conditions and the following disclaimer in the
       documentation and/or other materials provided with the distribution.
    3. Neither the name(s) of the author(s) nor the names of other contributors
       may be used to endorse or promote products derived from this software
       without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE AUTHOR(S) AND CONTRIBUTORS ``AS IS'' AND
    ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
    IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
    ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR(S) OR CONTRIBUTORS BE LIABLE
    FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
    DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
    OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
    HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
    LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
    OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
    SUCH DAMAGE.
*/

#if defined(_WIN32)
#include <Windows.h>
#include <Lmcons.h>
#include <process.h>
#include <shlobj.h>
#include <sys/types.h>
#include <io.h>
#include <conio.h>
#undef MOUSE_MOVED
#elif defined(__DJGPP__)
#include <process.h>
#else
#include <pwd.h>
#include <sys/utsname.h>
#include <unistd.h>
#include <utmpx.h>
#endif

#ifdef __INTERIX
char *strdup(const char *s);
#endif

#include <stdlib.h>
#include <string.h>

#if defined(_WIN32) && !defined(__MINGW32__)
#define PATH_MAX MAX_PATH
#endif

#include <curses.h>
#if !defined(DJGPP)
#include <term.h>
#endif

#include <stdio.h>
#include <fcntl.h>
#include <limits.h>
#include <sys/stat.h>
#include <signal.h>

#define MOD_MOVE(c) (toupper(c) )

void
md_init()
{
#ifdef __INTERIX
    char *term;

    term = getenv("TERM");

    if (term == NULL)
        setenv("TERM","interix",1);
#endif
#if defined(__DJGPP__) || defined(_WIN32)
    _fmode = _O_BINARY;
#endif
#if defined(__CYGWIN__) || defined(__MSYS__)
    ESCDELAY=250;
#endif
}

int
md_hasclreol()
{
#ifdef CE
    return((CE != NULL) && (*CE != 0));
#elif defined (clr_eol)
    return((clr_eol != NULL) && (*clr_eol != 0));
#elif !defined(__PDCURSES__)
    return(clr_eol != NULL);
#else
    return(TRUE);
#endif
}

void
md_putchar(int c)
{
    putchar(c);
}

static int md_standout_mode = 0;

void
md_raw_standout()
{
#ifdef _WIN32
    CONSOLE_SCREEN_BUFFER_INFO csbiInfo; 
    HANDLE hStdout;
    int fgattr,bgattr;

    if (md_standout_mode == 0)
    {
        hStdout = GetStdHandle(STD_OUTPUT_HANDLE); 
        GetConsoleScreenBufferInfo(hStdout, &csbiInfo);
        fgattr = (csbiInfo.wAttributes & 0xF);
        bgattr = (csbiInfo.wAttributes & 0xF0);
        SetConsoleTextAttribute(hStdout,(fgattr << 4) | (bgattr >> 4));
        md_standout_mode = 1;
    }
#elif defined(SO)
    tputs(SO,0,md_putchar);
    fflush(stdout);
#endif
}

void
md_raw_standend()
{
#ifdef _WIN32
    CONSOLE_SCREEN_BUFFER_INFO csbiInfo; 
    HANDLE hStdout;
    int fgattr,bgattr;

    if (md_standout_mode == 1)
    {
        hStdout = GetStdHandle(STD_OUTPUT_HANDLE); 
        GetConsoleScreenBufferInfo(hStdout, &csbiInfo);
        fgattr = (csbiInfo.wAttributes & 0xF);
        bgattr = (csbiInfo.wAttributes & 0xF0);
        SetConsoleTextAttribute(hStdout,(fgattr << 4) | (bgattr >> 4));
        md_standout_mode = 0;
    }
#elif defined(SE)
    tputs(SE,0,md_putchar);
    fflush(stdout);
#endif
}

int
md_unlink_open_file(char *file, int inf)
{
#ifdef _WIN32
    _close(inf);
    _chmod(file, 0600);
    return( _unlink(file) );
#else
    return(unlink(file));
#endif
}

int
md_unlink(char *file)
{
#ifdef _WIN32
    _chmod(file, 0600);
    return( _unlink(file) );
#else
    return(unlink(file));
#endif
}

int
md_creat(char *file, int mode)
{
    int fd;
#ifdef _WIN32
    mode = _S_IREAD | _S_IWRITE;
    fd = _open(file,O_CREAT | O_EXCL | O_WRONLY, mode);
#else
    fd = open(file,O_CREAT | O_EXCL | O_WRONLY, mode);
#endif

    return(fd);
}


void
md_normaluser()
{
#ifndef _WIN32
    setuid(getuid());
    setgid(getgid());
#endif
}

int
md_getuid()
{
#ifndef _WIN32
    return( getuid() );
#else
    return(42);
#endif
}

char *
md_getusername(int uid)
{
    static char login[80];
    char *l = NULL;
#ifdef _WIN32
    LPSTR mybuffer;
    DWORD size = UNLEN + 1;
    TCHAR buffer[UNLEN + 1];

    mybuffer = buffer;
    if (uid != md_getuid())
	strcpy(mybuffer, "someone");
    else
	GetUserName(mybuffer,&size);
    l = mybuffer;
#endif
#if !defined(_WIN32) && !defined(DJGPP)
    struct passwd *pw;

    pw = getpwuid(getuid());

    l = pw->pw_name;
#endif

    if ((l == NULL) || (*l == '\0'))
        if ( (l = getenv("USERNAME")) == NULL )
            if ( (l = getenv("LOGNAME")) == NULL )
                if ( (l = getenv("USER")) == NULL )
                    l = "nobody";

    strncpy(login,l,80);
    login[79] = 0;

    return(login);
}

char *
md_gethomedir()
{
    static char homedir[PATH_MAX];
    char *h = NULL;
    size_t len;
#if defined(_WIN32)
    TCHAR szPath[PATH_MAX];
#endif
#if defined(_WIN32) || defined(DJGPP)
        char slash = '\\';
#else
    char slash = '/';
    struct passwd *pw;
    pw = getpwuid(getuid());

    h = pw->pw_dir;

    if (strcmp(h,"/") == 0)
        h = NULL;
#endif
    homedir[0] = 0;
#ifdef _WIN32
    if(SUCCEEDED(SHGetFolderPath(NULL, CSIDL_PERSONAL, NULL, 0, szPath)))
        h = szPath;
#endif

    if ( (h == NULL) || (*h == '\0') )
        if ( (h = getenv("HOME")) == NULL )
                h = "";

    strncpy(homedir,h,PATH_MAX-1);
    len = strlen(homedir);

    if ((len > 0) && (homedir[len-1] == slash))
	homedir[len-1] = 0;

    return(homedir);
}

void
md_sleep(int s)
{
#ifdef _WIN32
    Sleep(s);
#else
    sleep(s);
#endif
}

char *
md_getshell()
{
    static char shell[PATH_MAX];
    char *s = NULL;
#ifdef _WIN32
    char *def = "C:\\WINDOWS\\SYSTEM32\\CMD.EXE";
#elif defined(__DJGPP__)
    char *def = "C:\\COMMAND.COM";
#else
    char *def = "/bin/sh";
    struct passwd *pw;
    pw = getpwuid(getuid());
    s = pw->pw_shell;
#endif
    if ((s == NULL) || (*s == '\0'))
        if ( (s = getenv("COMSPEC")) == NULL)
            if ( (s = getenv("SHELL")) == NULL)
                if ( (s = getenv("SystemRoot")) == NULL)
                    s = def;

    strncpy(shell,s,PATH_MAX);
    shell[PATH_MAX-1] = 0;

    return(shell);
}

int
md_shellescape()
{
#if (!defined(_WIN32) && !defined(__DJGPP__))
    int ret_status;
    int pid;
    void (*myquit)(int);
    void (*myend)(int);
#endif
    char *sh;

    sh = md_getshell();

#if defined(_WIN32)
    return((int)_spawnl(_P_WAIT,sh,"shell",NULL,0));
#elif defined(__DJGPP__)
    return ( spawnl(P_WAIT,sh,"shell",NULL,0) );
#else
    while((pid = fork()) < 0)
        sleep(1);

    if (pid == 0) /* Shell Process */
    {
        /*
         * Set back to original user, just in case
         */
        setuid(getuid());
        setgid(getgid());
        execl(sh == NULL ? "/bin/sh" : sh, "shell", "-i", 0);
        perror("No shelly");
        _exit(-1);
    }
    else /* Application */
    {
	myend = signal(SIGINT, SIG_IGN);
#ifdef SIGQUIT
        myquit = signal(SIGQUIT, SIG_IGN);
#endif  
        while (wait(&ret_status) != pid)
            continue;
	    
        signal(SIGINT, myquit);
#ifdef SIGQUIT
        signal(SIGQUIT, myend);
#endif
    }

    return(ret_status);
#endif
}

int
directory_exists(char *dirname)
{
    struct stat sb;

    if (stat(dirname, &sb) == 0) /* path exists */
        return (sb.st_mode & S_IFDIR);

    return(0);
}

char *
md_getroguedir()
{
    static char path[1024];
    char *end,*home;

    if ( (home = getenv("ROGUEHOME")) != NULL)
    {
        if (*home)
        {
            strncpy(path, home, PATH_MAX - 20);

            end = &path[strlen(path)-1];

            while( (end >= path) && ((*end == '/') || (*end == '\\')))
                *end-- = '\0';

            if (directory_exists(path))
                return(path);
        }
    }

    if (directory_exists("/var/games/roguelike"))
        return("/var/games/roguelike");
    if (directory_exists("/var/lib/roguelike"))
        return("/var/lib/roguelike");
    if (directory_exists("/var/roguelike"))
        return("/var/roguelike");
    if (directory_exists("/usr/games/lib"))
        return("/usr/games/lib");
    if (directory_exists("/games/roguelik"))
        return("/games/roguelik");
    if (directory_exists(md_gethomedir()))
	return(md_gethomedir());
    return("");
}

char *
md_getrealname(int uid)
{
    static char uidstr[20];
#if !defined(_WIN32) && !defined(DJGPP)
    struct passwd *pp;

	if ((pp = getpwuid(uid)) == NULL)
    {
        sprintf(uidstr,"%d", uid);
        return(uidstr);
    }
	else
	    return(pp->pw_name);
#else
   sprintf(uidstr,"%d", uid);
   return(uidstr);
#endif
}

extern char *xcrypt(char *key, char *salt);

char *
md_crypt(char *key, char *salt)
{
    return( xcrypt(key,salt) );
}

char *
md_getpass(char *prompt)
{
#ifdef _WIN32
    static char password_buffer[9];
    char *p = password_buffer;
    int c, count = 0;
    int max_length = 9;

    fflush(stdout);
    /* If we can't prompt, abort */
    if (fputs(prompt, stderr) < 0)
    {
        *p = '\0';
        return NULL;
    }

    for(;;)
    {
        /* Get a character with no echo */
        c = _getch();

        /* Exit on interrupt (^c or ^break) */
        if (c == '\003' || c == 0x100)
            exit(1);

        /* Terminate on end of line or file (^j, ^m, ^d, ^z) */
        if (c == '\r' || c == '\n' || c == '\004' || c == '\032')
            break;

        /* Back up on backspace */
        if (c == '\b')
        {
            if (count)
                count--;
            else if (p > password_buffer)
                p--;
            continue;
        }

        /* Ignore DOS extended characters */
        if ((c & 0xff) != c)
            continue;

        /* Add to password if it isn't full */
        if (p < password_buffer + max_length - 1)
            *p++ = c;
        else
            count++;
    }
   *p = '\0';

   fputc('\n', stderr);

   return password_buffer;
#else
   return( (char *) getpass(prompt) );
#endif
}


int md_endian = 0x01020304;

unsigned long int
md_ntohl(unsigned long int x)
{
#ifdef _WIN32
    if ( *((char *)&md_endian) == 0x01 )
        return(x);
    else
        return( ((x & 0x000000ffU) << 24) |
                ((x & 0x0000ff00U) <<  8) |
                ((x & 0x00ff0000U) >>  8) |
                ((x & 0xff000000U) >> 24) );
#else
    return( ntohl(x) );
#endif
}

unsigned long int
md_htonl(unsigned long int x)
{
#ifdef _WIN32
    if ( *((char *)&md_endian) == 0x01 )
        return(x);
    else
        return( ((x & 0x000000ffU) << 24) |
                ((x & 0x0000ff00U) <<  8) |
                ((x & 0x00ff0000U) >>  8) |
                ((x & 0xff000000U) >> 24) );
#else
    return( htonl(x) );
#endif
}

int
md_ucount()
{
#ifdef __DJGPP__
    return(1);
#elif defined(_WIN32)
    return(1);
#else
    struct utmpx *up=NULL;
    int count=0;

    setutxent();    
    do
    {
	up = getutxent();
	if (up && up->ut_type == USER_PROCESS)
	    count++;
    } while(up != NULL);
   
   endutxent();

   return(count);
#endif
}

int
md_getloadavg(double *avg)
{
#if defined(__GLIBC__) || defined(_BSD)
    if (getloadavg(avg, 3) == -1)
#endif
    {
	avg[0] = avg[1] = avg[2] = 0.0;
	return -1;
    }
}

long
md_random()
{
#ifdef _WIN32
    return(rand());
#else
    return( random() );
#endif
}

void
md_srandom(unsigned x)
{
#ifdef _WIN32
    srand(x);
#else
    srandom(x);
#endif
}

int
md_rand()
{
#ifdef _WIN32
    return(rand());
#else
    return(lrand48() & 0x7fffffff);
#endif
}

void
md_srand(int seed)
{
#ifdef _WIN32
    srand(seed);
#else
    srand48(seed);
#endif
}

char *
md_strdup(const char *s)
{
#ifdef _WIN32
    return( _strdup(s) );
#else
    return(strdup(s));
#endif
}

long
md_memused()
{
#ifdef _WIN32
    MEMORYSTATUS stat;

    GlobalMemoryStatus(&stat);

    return((long)stat.dwTotalPageFile);
#else
    return( (long)sbrk(0) );
#endif
}

char *
md_gethostname()
{
    static char nodename[80];
    char *n = NULL;
#if !defined(_WIN32) && !defined(__DJGPP__)
    struct utsname ourname;

    if (uname(&ourname) == 0)
        n = ourname.nodename;
#endif
    if ((n == NULL) || (*n == '\0'))
        if ( (n = getenv("COMPUTERNAME")) == NULL)
            if ( (n = getenv("HOSTNAME")) == NULL)
                n = "localhost";

    strncpy(nodename, n, 80);
    nodename[79] = 0;

    return(nodename);
}

int
md_erasechar()
{
#ifdef BSD
    return(_tty.sg_erase); /* process erase character */
#elif defined(USG5_0)
    return(_tty.c_cc[VERASE]); /* process erase character */
#else /* USG5_2 .... curses */
    return( erasechar() ); /* process erase character */
#endif
}

int
md_killchar()
{
#ifdef BSD
    return(_tty.sg_kill);
#elif defined(USG5_0)
    return(_tty.c_cc[VKILL]);
#else /* USG5_2 ..... curses */
    return( killchar() );
#endif
}

/*
 * unctrl:
 *	Print a readable version of a certain character
 */

char *
md_unctrl(char ch)
{
#if USG5_0
    extern char *_unctrl[];		/* Defined in curses library */

    return _unctrl[ch&0177];
#else
    return( unctrl(ch) );
#endif
}

void
md_flushinp()
{
#ifdef BSD
    ioctl(0, TIOCFLUSH);
#elif defined(USG5_0)
    ioctl(_tty_ch,TCFLSH,0)
#else /* USG5_2.... curses */
    flushinp();
#endif
}

/*
    Cursor/Keypad Support

    Sadly Cursor/Keypad support is less straightforward than it should be.

    The various terminal emulators/consoles choose to differentiate the
    cursor and keypad keys (with modifiers) in different ways (if at all!).
    Furthermore they use different code set sequences for each key only
    a subset of which the various curses libraries recognize. Partly due
    to incomplete termcap/terminfo entries and partly due to inherent
    limitations of those terminal capability databases.

    I give curses first crack at decoding the sequences. If it fails to decode
    it we check for common ESC-prefixed sequences.

    All cursor/keypad results are translated into standard rogue movement
    commands.

    Unmodified keys are translated to walk commands: hjklyubn
    Modified (shift,control,alt) are translated to run commands: HJKLYUBN

    Console and supported (differentiated) keys
    Interix:  Cursor Keys, Keypad, Ctl-Keypad
    Cygwin:   Cursor Keys, Keypad, Alt-Cursor Keys
    MSYS:     Cursor Keys, Keypad, Ctl-Cursor Keys, Ctl-Keypad
    Win32:    Cursor Keys, Keypad, Ctl/Shift/Alt-Cursor Keys, Ctl/Alt-Keypad
    DJGPP:    Cursor Keys, Keypad, Ctl/Shift/Alt-Cursor Keys, Ctl/Alt-Keypad

    Interix Console (raw, ncurses)
    ==============================
    normal      shift           ctrl        alt
    ESC [D,     ESC F^,         ESC [D,     ESC [D          /# Left         #/
    ESC [C,     ESC F$,         ESC [C,     ESC [C          /# Right        #/
    ESC [A,     ESC F-,         local win,  ESC [A          /# Up           #/
    ESC [B,     ESC F+,         local win,  ESC [B          /# Down         #/
    ESC [H,     ESC [H,         ESC [H,     ESC [H          /# Home         #/
    ESC [S,     local win,      ESC [S,     ESC [S          /# Page Up      #/
    ESC [T,     local win,      ESC [T,     ESC [T          /# Page Down    #/
    ESC [U,     ESC [U,         ESC [U,     ESC [U          /# End          #/
    ESC [D,     ESC F^,         ESC [D,     O               /# Keypad Left  #/
    ESC [C,     ESC F$,         ESC [C,     O               /# Keypad Right #/
    ESC [A,     ESC [A,         ESC [-1,    O               /# Keypad Up    #/
    ESC [B,     ESC [B,         ESC [-2,    O               /# Keypad Down  #/
    ESC [H,     ESC [H,         ESC [-263,  O               /# Keypad Home  #/
    ESC [S,     ESC [S,         ESC [-19,   O               /# Keypad PgUp  #/
    ESC [T,     ESC [T,         ESC [-20,   O               /# Keypad PgDn  #/
    ESC [U,     ESC [U,         ESC [-21,   O               /# Keypad End   #/
    nothing,    nothing,        nothing,    O               /# Kaypad 5     #/

    Interix Console (term=interix, ncurses)
    ==============================
    KEY_LEFT,   ESC F^,         KEY_LEFT,   KEY_LEFT        /# Left         #/
    KEY_RIGHT,  ESC F$,         KEY_RIGHT,  KEY_RIGHT       /# Right        #/
    KEY_UP,     0x146,          local win,  KEY_UP          /# Up           #/
    KEY_DOWN,   0x145,          local win,  KEY_DOWN        /# Down         #/
    ESC [H,     ESC [H,         ESC [H,     ESC [H          /# Home         #/
    KEY_PPAGE,  local win,      KEY_PPAGE,  KEY_PPAGE       /# Page Up      #/
    KEY_NPAGE,  local win,      KEY_NPAGE,  KEY_NPAGE       /# Page Down    #/
    KEY_LL,     KEY_LL,         KEY_LL,     KEY_LL          /# End          #/
    KEY_LEFT,   ESC F^,         ESC [-4,    O               /# Keypad Left  #/
    KEY_RIGHT,  ESC F$,         ESC [-3,    O               /# Keypad Right #/
    KEY_UP,     KEY_UP,         ESC [-1,    O               /# Keypad Up    #/
    KEY_DOWN,   KEY_DOWN,       ESC [-2,    O               /# Keypad Down  #/
    ESC [H,     ESC [H,         ESC [-263,  O               /# Keypad Home  #/
    KEY_PPAGE,  KEY_PPAGE,      ESC [-19,   O               /# Keypad PgUp  #/
    KEY_NPAGE,  KEY_NPAGE,      ESC [-20,   O               /# Keypad PgDn  #/
    KEY_LL,     KEY_LL,         ESC [-21,   O               /# Keypad End   #/
    nothing,    nothing,        nothing,    O               /# Keypad 5     #/

    Cygwin Console (raw, ncurses)
    ==============================
    normal      shift           ctrl        alt
    ESC [D,     ESC [D,         ESC [D,     ESC ESC [D      /# Left         #/
    ESC [C,     ESC [C,         ESC [C,     ESC ESC [C      /# Rght         #/
    ESC [A,     ESC [A,         ESC [A,     ESC ESC [A      /# Up           #/
    ESC [B,     ESC [B,         ESC [B,     ESC ESC [B      /# Down         #/
    ESC [1~,    ESC [1~,        ESC [1~,    ESC ESC [1~     /# Home         #/
    ESC [5~,    ESC [5~,        ESC [5~,    ESC ESC [5~     /# Page Up      #/
    ESC [6~,    ESC [6~,        ESC [6~,    ESC ESC [6~     /# Page Down    #/
    ESC [4~,    ESC [4~,        ESC [4~,    ESC ESC [4~     /# End          #/
    ESC [D,     ESC [D,         ESC [D,     ESC ESC [D,O    /# Keypad Left  #/
    ESC [C,     ESC [C,         ESC [C,     ESC ESC [C,O    /# Keypad Right #/
    ESC [A,     ESC [A,         ESC [A,     ESC ESC [A,O    /# Keypad Up    #/
    ESC [B,     ESC [B,         ESC [B,     ESC ESC [B,O    /# Keypad Down  #/
    ESC [1~,    ESC [1~,        ESC [1~,    ESC ESC [1~,O   /# Keypad Home  #/
    ESC [5~,    ESC [5~,        ESC [5~,    ESC ESC [5~,O   /# Keypad PgUp  #/
    ESC [6~,    ESC [6~,        ESC [6~,    ESC ESC [6~,O   /# Keypad PgDn  #/
    ESC [4~,    ESC [4~,        ESC [4~,    ESC ESC [4~,O   /# Keypad End   #/
    ESC [-71,   nothing,        nothing,    O               /# Keypad 5     #/

    Cygwin Console (term=cygwin, ncurses)
    ==============================
    KEY_LEFT,   KEY_LEFT,       KEY_LEFT,   ESC-260         /# Left         #/
    KEY_RIGHT,  KEY_RIGHT,      KEY_RIGHT,  ESC-261         /# Rght         #/
    KEY_UP,     KEY_UP,         KEY_UP,     ESC-259         /# Up           #/
    KEY_DOWN,   KEY_DOWN,       KEY_DOWN,   ESC-258         /# Down         #/
    KEY_HOME,   KEY_HOME,       KEY_HOME,   ESC-262         /# Home         #/
    KEY_PPAGE,  KEY_PPAGE,      KEY_PPAGE,  ESC-339         /# Page Up      #/
    KEY_NPAGE,  KEY_NPAGE,      KEY_NPAGE,  ESC-338         /# Page Down    #/
    KEY_END,    KEY_END,        KEY_END,    ESC-360         /# End          #/
    KEY_LEFT,   KEY_LEFT,       KEY_LEFT,   ESC-260,O       /# Keypad Left  #/
    KEY_RIGHT,  KEY_RIGHT,      KEY_RIGHT,  ESC-261,O       /# Keypad Right #/
    KEY_UP,     KEY_UP,         KEY_UP,     ESC-259,O       /# Keypad Up    #/
    KEY_DOWN,   KEY_DOWN,       KEY_DOWN,   ESC-258,O       /# Keypad Down  #/
    KEY_HOME,   KEY_HOME,       KEY_HOME,   ESC-262,O       /# Keypad Home  #/
    KEY_PPAGE,  KEY_PPAGE,      KEY_PPAGE,  ESC-339,O       /# Keypad PgUp  #/
    KEY_NPAGE,  KEY_NPAGE,      KEY_NPAGE,  ESC-338,O       /# Keypad PgDn  #/
    KEY_END,    KEY_END,        KEY_END,    ESC-360,O       /# Keypad End   #/
    ESC [G,     nothing,        nothing,    O               /# Keypad 5     #/

    MSYS Console (raw, ncurses)
    ==============================
    normal      shift           ctrl        alt
    ESC OD,     ESC [d,         ESC Od      nothing         /# Left         #/
    ESC OE,     ESC [e,         ESC Oe,     nothing         /# Right        #/
    ESC OA,     ESC [a,         ESC Oa,     nothing         /# Up           #/
    ESC OB,     ESC [b,         ESC Ob,     nothing         /# Down         #/
    ESC [7~,    ESC [7$,        ESC [7^,    nothing         /# Home         #/
    ESC [5~,    local window,   ESC [5^,    nothing         /# Page Up      #/
    ESC [6~,    local window,   ESC [6^,    nothing         /# Page Down    #/
    ESC [8~,    ESC [8$,        ESC [8^,    nothing         /# End          #/
    ESC OD,     ESC [d,         ESC Od      O               /# Keypad Left  #/
    ESC OE,     ESC [c,         ESC Oc,     O               /# Keypad Right #/
    ESC OA,     ESC [a,         ESC Oa,     O               /# Keypad Up    #/
    ESC OB,     ESC [b,         ESC Ob,     O               /# Keypad Down  #/
    ESC [7~,    ESC [7$,        ESC [7^,    O               /# Keypad Home  #/
    ESC [5~,    local window,   ESC [5^,    O               /# Keypad PgUp  #/
    ESC [6~,    local window,   ESC [6^,    O               /# Keypad PgDn  #/
    ESC [8~,    ESC [8$,        ESC [8^,    O               /# Keypad End   #/
    11,         11,             11,         O               /# Keypad 5     #/

    MSYS Console (term=rxvt, ncurses)
    ==============================
    normal      shift           ctrl        alt
    KEY_LEFT,   KEY_SLEFT,      514         nothing         /# Left         #/
    KEY_RIGHT,  KEY_SRIGHT,     516,        nothing         /# Right        #/
    KEY_UP,     518,            519,        nothing         /# Up           #/
    KEY_DOWN,   511,            512,        nothing         /# Down         #/
    KEY_HOME,   KEY_SHOME,      ESC [7^,    nothing         /# Home         #/
    KEY_PPAGE,  local window,   ESC [5^,    nothing         /# Page Up      #/
    KEY_NPAGE,  local window,   ESC [6^,    nothing         /# Page Down    #/
    KEY_END,    KEY_SEND,       KEY_EOL,    nothing         /# End          #/
    KEY_LEFT,   KEY_SLEFT,      514         O               /# Keypad Left  #/
    KEY_RIGHT,  KEY_SRIGHT,     516,        O               /# Keypad Right #/
    KEY_UP,     518,            519,        O               /# Keypad Up    #/
    KEY_DOWN,   511,            512,        O               /# Keypad Down  #/
    KEY_HOME,   KEY_SHOME,      ESC [7^,    O               /# Keypad Home  #/
    KEY_PPAGE,  local window,   ESC [5^,    O               /# Keypad PgUp  #/
    KEY_NPAGE,  local window,   ESC [6^,    O               /# Keypad PgDn  #/
    KEY_END,    KEY_SEND,       KEY_EOL,    O               /# Keypad End   #/
    11,         11,             11,         O               /# Keypad 5     #/

    Win32 Console (raw, pdcurses)
   DJGPP Console (raw, pdcurses)
   ==============================
   normal      shift           ctrl        alt
   260,        391,            443,        493             /# Left         #/
   261,        400,            444,        492             /# Right        #/
   259,        547,            480,        490             /# Up           #/
   258,        548,            481,        491             /# Down         #/
   262,        388,            447,        524             /# Home         #/
   339,        396,            445,        526             /# Page Up      #/
   338,        394,            446,        520             /# Page Down    #/
   358,        384,            448,        518             /# End          #/
   452,        52('4'),        511,        521             /# Keypad Left  #/
   454,        54('6'),        513,        523             /# Keypad Right #/
   450,        56('8'),        515,        525             /# Keypad Up    #/
   456,        50('2'),        509,        519             /# Keypad Down  #/
   449,        55('7'),        514,        524             /# Keypad Home  #/
   451,        57('9'),        516,        526             /# Keypad PgUp  #/
   457,        51('3'),        510,        520             /# Keypad PgDn  #/
   455,        49('1'),        508,        518             /# Keypad End   #/
   453,        53('5'),        512,        522             /# Keypad 5     #/

   Win32 Console (pdcurses, MSVC/MingW32)
   DJGPP Console (pdcurses)
   ==============================
   normal      shift           ctrl        alt
   KEY_LEFT,   KEY_SLEFT,      CTL_LEFT,   ALT_LEFT        /# Left         #/
   KEY_RIGHT,  KEY_SRIGHT,     CTL_RIGHT,  ALT_RIGHT       /# Right        #/
   KEY_UP,     KEY_SUP,        CTL_UP,     ALT_UP          /# Up           #/
   KEY_DOWN,   KEY_SDOWN,      CTL_DOWN,   ALT_DOWN        /# Down         #/
   KEY_HOME,   KEY_SHOME,      CTL_HOME,   ALT_HOME        /# Home         #/
   KEY_PPAGE,  KEY_SPREVIOUS,  CTL_PGUP,   ALT_PGUP        /# Page Up      #/
   KEY_NPAGE,  KEY_SNEXTE,     CTL_PGDN,   ALT_PGDN        /# Page Down    #/
   KEY_END,    KEY_SEND,       CTL_END,    ALT_END         /# End          #/
   KEY_B1,     52('4'),        CTL_PAD4,   ALT_PAD4        /# Keypad Left  #/
   KEY_B3,     54('6'),        CTL_PAD6,   ALT_PAD6        /# Keypad Right #/
   KEY_A2,     56('8'),        CTL_PAD8,   ALT_PAD8        /# Keypad Up    #/
   KEY_C2,     50('2'),        CTL_PAD2,   ALT_PAD2        /# Keypad Down  #/
   KEY_A1,     55('7'),        CTL_PAD7,   ALT_PAD7        /# Keypad Home  #/
   KEY_A3,     57('9'),        CTL_PAD9,   ALT_PAD9        /# Keypad PgUp  #/
   KEY_C3,     51('3'),        CTL_PAD3,   ALT_PAD3        /# Keypad PgDn  #/
   KEY_C1,     49('1'),        CTL_PAD1,   ALT_PAD1        /# Keypad End   #/
   KEY_B2,     53('5'),        CTL_PAD5,   ALT_PAD5        /# Keypad 5     #/

   Windows Telnet (raw)
   ==============================
   normal      shift           ctrl        alt
   ESC [D,     ESC [D,         ESC [D,     ESC [D          /# Left         #/
   ESC [C,     ESC [C,         ESC [C,     ESC [C          /# Right        #/
   ESC [A,     ESC [A,         ESC [A,     ESC [A          /# Up           #/
   ESC [B,     ESC [B,         ESC [B,     ESC [B          /# Down         #/
   ESC [1~,    ESC [1~,        ESC [1~,    ESC [1~         /# Home         #/
   ESC [5~,    ESC [5~,        ESC [5~,    ESC [5~         /# Page Up      #/
   ESC [6~,    ESC [6~,        ESC [6~,    ESC [6~         /# Page Down    #/
   ESC [4~,    ESC [4~,        ESC [4~,    ESC [4~         /# End          #/
   ESC [D,     ESC [D,         ESC [D,     ESC [D          /# Keypad Left  #/
   ESC [C,     ESC [C,         ESC [C,     ESC [C          /# Keypad Right #/
   ESC [A,     ESC [A,         ESC [A,     ESC [A          /# Keypad Up    #/
   ESC [B,     ESC [B,         ESC [B,     ESC [B          /# Keypad Down  #/
   ESC [1~,    ESC [1~,        ESC [1~,    ESC [1~         /# Keypad Home  #/
   ESC [5~,    ESC [5~,        ESC [5~,    ESC [5~         /# Keypad PgUp  #/
   ESC [6~,    ESC [6~,        ESC [6~,    ESC [6~         /# Keypad PgDn  #/
   ESC [4~,    ESC [4~,        ESC [4~,    ESC [4~         /# Keypad End   #/
   nothing,    nothing,        nothing,    nothing         /# Keypad 5     #/

   Windows Telnet (term=xterm)
   ==============================
   normal      shift           ctrl        alt
   KEY_LEFT,   KEY_LEFT,       KEY_LEFT,   KEY_LEFT        /# Left         #/
   KEY_RIGHT,  KEY_RIGHT,      KEY_RIGHT,  KEY_RIGHT       /# Right        #/
   KEY_UP,     KEY_UP,         KEY_UP,     KEY_UP          /# Up           #/
   KEY_DOWN,   KEY_DOWN,       KEY_DOWN,   KEY_DOWN        /# Down         #/
   ESC [1~,    ESC [1~,        ESC [1~,    ESC [1~         /# Home         #/
   KEY_PPAGE,  KEY_PPAGE,      KEY_PPAGE,  KEY_PPAGE       /# Page Up      #/
   KEY_NPAGE,  KEY_NPAGE,      KEY_NPAGE,  KEY_NPAGE       /# Page Down    #/
   ESC [4~,    ESC [4~,        ESC [4~,    ESC [4~         /# End          #/
    KEY_LEFT,   KEY_LEFT,       KEY_LEFT,   O               /# Keypad Left  #/
    KEY_RIGHT,  KEY_RIGHT,      KEY_RIGHT,  O               /# Keypad Right #/
    KEY_UP,     KEY_UP,         KEY_UP,     O               /# Keypad Up    #/
    KEY_DOWN,   KEY_DOWN,       KEY_DOWN,   O               /# Keypad Down  #/
    ESC [1~,    ESC [1~,        ESC [1~,    ESC [1~         /# Keypad Home  #/
    KEY_PPAGE,  KEY_PPAGE,      KEY_PPAGE,  KEY_PPAGE       /# Keypad PgUp  #/
    KEY_NPAGE,  KEY_NPAGE,      KEY_NPAGE,  KEY_NPAGE       /# Keypad PgDn  #/
    ESC [4~,    ESC [4~,        ESC [4~,    O               /# Keypad End   #/
    ESC [-71,   nothing,        nothing,    O               /# Keypad 5     #/

    PuTTY
    ==============================
    normal      shift           ctrl        alt
    ESC [D,     ESC [D,         ESC OD,     ESC [D          /# Left         #/
    ESC [C,     ESC [C,         ESC OC,     ESC [C          /# Right        #/
    ESC [A,     ESC [A,         ESC OA,     ESC [A          /# Up           #/
    ESC [B,     ESC [B,         ESC OB,     ESC [B          /# Down         #/
    ESC [1~,    ESC [1~,        local win,  ESC [1~         /# Home         #/
    ESC [5~,    local win,      local win,  ESC [5~         /# Page Up      #/
    ESC [6~,    local win,      local win,  ESC [6~         /# Page Down    #/
    ESC [4~,    ESC [4~,        local win,  ESC [4~         /# End          #/
    ESC [D,     ESC [D,         ESC [D,     O               /# Keypad Left  #/
    ESC [C,     ESC [C,         ESC [C,     O               /# Keypad Right #/
    ESC [A,     ESC [A,         ESC [A,     O               /# Keypad Up    #/
    ESC [B,     ESC [B,         ESC [B,     O               /# Keypad Down  #/
    ESC [1~,    ESC [1~,        ESC [1~,    O               /# Keypad Home  #/
    ESC [5~,    ESC [5~,        ESC [5~,    O               /# Keypad PgUp  #/
    ESC [6~,    ESC [6~,        ESC [6~,    O               /# Keypad PgDn  #/
    ESC [4~,    ESC [4~,        ESC [4~,    O               /# Keypad End   #/
    nothing,    nothing,        nothing,    O               /# Keypad 5     #/

    PuTTY
    ==============================
    normal      shift           ctrl        alt
    KEY_LEFT,   KEY_LEFT,       ESC OD,     ESC KEY_LEFT    /# Left         #/
    KEY_RIGHT   KEY_RIGHT,      ESC OC,     ESC KEY_RIGHT   /# Right        #/
    KEY_UP,     KEY_UP,         ESC OA,     ESC KEY_UP      /# Up           #/
    KEY_DOWN,   KEY_DOWN,       ESC OB,     ESC KEY_DOWN    /# Down         #/
    ESC [1~,    ESC [1~,        local win,  ESC ESC [1~     /# Home         #/
    KEY_PPAGE   local win,      local win,  ESC KEY_PPAGE   /# Page Up      #/
    KEY_NPAGE   local win,      local win,  ESC KEY_NPAGE   /# Page Down    #/
    ESC [4~,    ESC [4~,        local win,  ESC ESC [4~     /# End          #/
    ESC Ot,     ESC Ot,         ESC Ot,     O               /# Keypad Left  #/
    ESC Ov,     ESC Ov,         ESC Ov,     O               /# Keypad Right #/
    ESC Ox,     ESC Ox,         ESC Ox,     O               /# Keypad Up    #/
    ESC Or,     ESC Or,         ESC Or,     O               /# Keypad Down  #/
    ESC Ow,     ESC Ow,         ESC Ow,     O               /# Keypad Home  #/
    ESC Oy,     ESC Oy,         ESC Oy,     O               /# Keypad PgUp  #/
    ESC Os,     ESC Os,         ESC Os,     O               /# Keypad PgDn  #/
    ESC Oq,     ESC Oq,         ESC Oq,     O               /# Keypad End   #/
    ESC Ou,     ESC Ou,         ESC Ou,     O               /# Keypad 5     #/
*/

#define M_NORMAL 0
#define M_ESC    1
#define M_KEYPAD 2
#define M_TRAIL  3

int
md_readchar(WINDOW *win)
{
    int ch = 0;
    int lastch = 0;
    int mode = M_NORMAL;
    int mode2 = M_NORMAL;

    for(;;)
    {
        ch = wgetch(win);

        if (ch == ERR)      /* timed out waiting for valid sequence */
        {                   /* flush input so far and start over    */
            mode = M_NORMAL;
            nocbreak();
            raw();
            ch = 27;
            break;
        }

        if (mode == M_TRAIL)
        {
            if (ch == '^')              /* msys console  : 7,5,6,8: modified*/
                ch = MOD_MOVE( toupper(lastch) );

            if (ch == '~')              /* cygwin console: 1,5,6,4: normal  */
                ch = tolower(lastch);   /* windows telnet: 1,5,6,4: normal  */
                                        /* msys console  : 7,5,6,8: normal  */

            if (mode2 == M_ESC)         /* cygwin console: 1,5,6,4: modified*/
                ch = MOD_MOVE( toupper(ch) );

            break;
        }

        if (mode == M_ESC)
        {
            if (ch == 27)
            {
                mode2 = M_ESC;
                continue;
            }

            if ((ch == 'F') || (ch == 'O') || (ch == '['))
            {
                mode = M_KEYPAD;
                continue;
            }


            switch(ch)
            {
                /* Cygwin Console   */
                /* PuTTY            */
                case KEY_LEFT : ch = MOD_MOVE('H'); break;
                case KEY_RIGHT: ch = MOD_MOVE('L'); break;
                case KEY_UP   : ch = MOD_MOVE('K'); break;
                case KEY_DOWN : ch = MOD_MOVE('J'); break;
                case KEY_HOME : ch = MOD_MOVE('Y'); break;
                case KEY_PPAGE: ch = MOD_MOVE('U'); break;
                case KEY_NPAGE: ch = MOD_MOVE('N'); break;
                case KEY_END  : ch = MOD_MOVE('B'); break;

                default: break;
            }

            break;
        }

        if (mode == M_KEYPAD)
        {
            switch(ch)
            {
                /* ESC F - Interix Console codes */
                case   '^': ch = MOD_MOVE('H'); break;      /* Shift-Left       */
                case   '$': ch = MOD_MOVE('L'); break;      /* Shift-Right      */

                /* ESC [ - Interix Console codes */
                case   'H': ch = 'y'; break;            /* Home             */
                case     1: ch = MOD_MOVE('K'); break;      /* Ctl-Keypad Up    */
                case     2: ch = MOD_MOVE('J'); break;      /* Ctl-Keypad Down  */
                case     3: ch = MOD_MOVE('L'); break;      /* Ctl-Keypad Right */
                case     4: ch = MOD_MOVE('H'); break;      /* Ctl-Keypad Left  */
                case   263: ch = MOD_MOVE('Y'); break;      /* Ctl-Keypad Home  */
                case    19: ch = MOD_MOVE('U'); break;      /* Ctl-Keypad PgUp  */
                case    20: ch = MOD_MOVE('N'); break;      /* Ctl-Keypad PgDn  */
                case    21: ch = MOD_MOVE('B'); break;      /* Ctl-Keypad End   */

                /* ESC [ - Cygwin Console codes */
                case   'G': ch = '.'; break;            /* Keypad 5         */
                case   '7': lastch = 'Y'; mode=M_TRAIL; break;  /* Ctl-Home */
                case   '5': lastch = 'U'; mode=M_TRAIL; break;  /* Ctl-PgUp */
                case   '6': lastch = 'N'; mode=M_TRAIL; break;  /* Ctl-PgDn */

                /* ESC [ - Win32 Telnet, PuTTY */
                case   '1': lastch = 'y'; mode=M_TRAIL; break;  /* Home     */
                case   '4': lastch = 'b'; mode=M_TRAIL; break;  /* End      */

                /* ESC O - PuTTY */
                case   'D': ch = MOD_MOVE('H'); break;
                case   'C': ch = MOD_MOVE('L'); break;
                case   'A': ch = MOD_MOVE('K'); break;
                case   'B': ch = MOD_MOVE('J'); break;
                case   't': ch = 'h'; break;
                case   'v': ch = 'l'; break;
                case   'x': ch = 'k'; break;
                case   'r': ch = 'j'; break;
                case   'w': ch = 'y'; break;
                case   'y': ch = 'u'; break;
                case   's': ch = 'n'; break;
                case   'q': ch = 'b'; break;
                case   'u': ch = '.'; break;
            }

            if (mode != M_KEYPAD)
                continue;
        }

        if (ch == 27)
        {
            halfdelay(1);
            mode = M_ESC;
            continue;
        }

        switch(ch)
        {
            case KEY_LEFT   : ch = 'h'; break;
            case KEY_DOWN   : ch = 'j'; break;
            case KEY_UP     : ch = 'k'; break;
            case KEY_RIGHT  : ch = 'l'; break;
            case KEY_HOME   : ch = 'y'; break;
            case KEY_PPAGE  : ch = 'u'; break;
            case KEY_END    : ch = 'b'; break;
#ifdef KEY_LL
            case KEY_LL     : ch = 'b'; break;
#endif
            case KEY_NPAGE  : ch = 'n'; break;

#ifdef KEY_B1
            case KEY_B1     : ch = 'h'; break;
            case KEY_C2     : ch = 'j'; break;
            case KEY_A2     : ch = 'k'; break;
            case KEY_B3     : ch = 'l'; break;
#endif
            case KEY_A1     : ch = 'y'; break;
            case KEY_A3     : ch = 'u'; break;
            case KEY_C1     : ch = 'b'; break;
            case KEY_C3     : ch = 'n'; break;
            /* next should be '.', but for problem with putty/linux */
            case KEY_B2     : ch = 'u'; break;

#ifdef KEY_SLEFT
            case KEY_SRIGHT  : ch = MOD_MOVE('L'); break;
            case KEY_SLEFT   : ch = MOD_MOVE('H'); break;
#ifdef KEY_SUP
            case KEY_SUP     : ch = MOD_MOVE('K'); break;
            case KEY_SDOWN   : ch = MOD_MOVE('J'); break;
#endif
            case KEY_SHOME   : ch = MOD_MOVE('Y'); break;
            case KEY_SPREVIOUS:ch = MOD_MOVE('U'); break;
            case KEY_SEND    : ch = MOD_MOVE('B'); break;
            case KEY_SNEXT   : ch = MOD_MOVE('N'); break;
#endif
            case 0x146       : ch = MOD_MOVE('K'); break;   /* Shift-Up     */
            case 0x145       : ch = MOD_MOVE('J'); break;   /* Shift-Down   */

#ifdef CTL_RIGHT
            case CTL_RIGHT   : ch = MOD_MOVE('L'); break;
            case CTL_LEFT    : ch = MOD_MOVE('H'); break;
            case CTL_UP      : ch = MOD_MOVE('K'); break;
            case CTL_DOWN    : ch = MOD_MOVE('J'); break;
            case CTL_HOME    : ch = MOD_MOVE('Y'); break;
            case CTL_PGUP    : ch = MOD_MOVE('U'); break;
            case CTL_END     : ch = MOD_MOVE('B'); break;
            case CTL_PGDN    : ch = MOD_MOVE('N'); break;
#endif
#ifdef KEY_EOL
            case KEY_EOL     : ch = MOD_MOVE('B'); break;
#endif

#ifndef CTL_PAD1
            /* MSYS rxvt console */
            case 511         : ch = MOD_MOVE('J'); break; /* Shift Dn */
            case 512         : ch = MOD_MOVE('J'); break; /* Ctl Down */
            case 514         : ch = MOD_MOVE('H'); break; /* Ctl Left */
            case 516         : ch = MOD_MOVE('L'); break; /* Ctl Right*/
            case 518         : ch = MOD_MOVE('K'); break; /* Shift Up */
            case 519         : ch = MOD_MOVE('K'); break; /* Ctl Up   */
#endif

#ifdef CTL_PAD1
            case CTL_PAD1   : ch = MOD_MOVE('B'); break;
            case CTL_PAD2   : ch = MOD_MOVE('J'); break;
            case CTL_PAD3   : ch = MOD_MOVE('N'); break;
            case CTL_PAD4   : ch = MOD_MOVE('H'); break;
            case CTL_PAD5   : ch = '.'; break;
            case CTL_PAD6   : ch = MOD_MOVE('L'); break;
            case CTL_PAD7   : ch = MOD_MOVE('Y'); break;
            case CTL_PAD8   : ch = MOD_MOVE('K'); break;
            case CTL_PAD9   : ch = MOD_MOVE('U'); break;
#endif

#ifdef ALT_RIGHT
            case ALT_RIGHT  : ch = MOD_MOVE('L'); break;
            case ALT_LEFT   : ch = MOD_MOVE('H'); break;
            case ALT_DOWN   : ch = MOD_MOVE('J'); break;
            case ALT_HOME   : ch = MOD_MOVE('Y'); break;
            case ALT_PGUP   : ch = MOD_MOVE('U'); break;
            case ALT_END    : ch = MOD_MOVE('B'); break;
            case ALT_PGDN   : ch = MOD_MOVE('N'); break;
#endif

#ifdef ALT_PAD1
            case ALT_PAD1   : ch = MOD_MOVE('B'); break;
            case ALT_PAD2   : ch = MOD_MOVE('J'); break;
            case ALT_PAD3   : ch = MOD_MOVE('N'); break;
            case ALT_PAD4   : ch = MOD_MOVE('H'); break;
            case ALT_PAD5   : ch = '.'; break;
            case ALT_PAD6   : ch = MOD_MOVE('L'); break;
            case ALT_PAD7   : ch = MOD_MOVE('Y'); break;
            case ALT_PAD8   : ch = MOD_MOVE('K'); break;
            case ALT_PAD9   : ch = MOD_MOVE('U'); break;
#endif
        }

        break;
    }

    nocbreak();     /* disable halfdelay mode if on */
    raw();

    return(ch & 0x7F);
}
