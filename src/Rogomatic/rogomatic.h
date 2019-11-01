#pragma once
#include <stdio.h>
#include "types.h"

char *findentry_getfakename(char* string, stuff item_type);
char *findentry_getrealname(char* string, stuff item_type);
char *getname(void);
char *itemstr(register int i);
char *md_gethomedir(void);
char *md_gethostname(void);
char *md_getpass(char *prompt);
char *md_getrealname(int uid);
char *md_getshell(void);
char *md_getusername(int uid);
char *md_strdup(const char *s);
char *md_unctrl(char ch);
char *monname(char m);
char *realname(char* codename);
char *statusline(void);
char commandarg(char* cmd, int n);
char functionchar(char* cmd);
char getroguechar(void);
char getroguetoken(void);
char *getname(void);
char getroguetoken(void);
const char *getLockFile(void);
const char *getRgmDir(void);
double mean(const register statistic *s);
double prob(register probability *p);
double stdev(register statistic *s);
FILE *md_fdopen(int fd,char *mode);
FILE *rogo_openlog(char *genelog);
FILE *rogo_openlog(register char *genelog);
FILE *wopen(char* fname, char* mode);
void saynow(char* f, ...);
void say(char* f, ...);
void command(int tmode, char* f, ...);
void sendnow(char* f, ...);
void say_impl(char* f, va_list args);
void add_to_screen(int row, int col, char ch);
char get_from_screen(int row, int col);
void bumpsearchcount(void);
void adjustpack(char *cmd);
void clearcommand(void);
void usemsg(char *str, int obj);
void showcommand(char *cmd);
void promptforflags(void);
void dumpflags(int r, int c);
void toggledebug(void);
void avoidmonsters(void);
void caddycorner(int r, int c, int d1, int d2, char ch);
void pinavoid(void);
int avoid(void);
int dwait(int msgtype, char* f, ...);
void clearscreen(void);
void quitrogue(char *reason, int gld, int terminationtype);
void dumpinv(register FILE *f);
int wielding(stuff otype);
void terpbot(void);
void animate(char *movie[]);
void add_score(char *new_line, char *vers, int ntrm);
void nametrap(int traptype, int standingonit);
void getrogue(char *waitstr, int onat);
void at(int r, int c);
void debuglog(const char *fmt, ...);
void deletestuff(int row, int col);
void dosnapshot(void);
void waitforspace(void);
void evalknobs(int gid, int score, int level);
void fillstruct(FILE *f, struct levstruct *lev);
void foundnew(void);
int wanttowake(char c);
void infer(char *objname, stuff item_type);
int runvalue(int r, int c, int depth, int *val, int *avd, int *cont);
void readident(char *name);
void rampage(void);
void curseditem(void);
washit(char *monster);
void wasmissed(char *monster);
void didhit(void);
void didmiss(void);
void killed(register char *monster);
void sendcnow(char c);
int stlmatch(char *, char *);
int unknown(stuff otype);
int used(char *codename);
void analyzeltm(void);
void clearstat(register  statistic * s);
int rogo_randint(register int max);
void markchokepts(void);
void connectdoors(register int r1, register int c1, register int r2, register int c2);
void unmarkexplored(int row, int col);
void markexplored(int row, int col);
void updatepos(register char ch, register int row, register int col);
void deadrogue(void);
void redrawscreen(void);
void toggleecho(void);
void readltm(void);
void parsemonster(char *monster);
void clearltm(register ltmrec *ltmarr);
void initseed(void);
void endlesson(void);
void startlesson(void);
void mshit(char *monster);
void msmiss(char *monster);
void countgold(register char *amount);
void parsemsg(register char *mess, register char *mend);
void deletemonster(int r, int c);
void clearpack(int pos);
void doresetinv(void);
void countpack(void);
void rollpackup(register int pos);
void rollpackdown(register int pos);
void teleport(void);
void saveltm(int score);
void positionreplay(void);
void inferhall(register int r, register int c);
void updateat(void);
void setnewgoal(void);
int takeoff(void);
void summary(FILE *f, char sep);
int throw (int obj, int dir);
void critical(void);
void uncritical(void);
void unlock_file(const char *lokfil);
void unrest(void);
int movetorest(void);
int reads(int obj);
int armorclass(int i);
int dropjunk(void);
int gotocorner(void);
int light(void);
int shootindark(void);
int dinnertime(void);
int trywand(void);
int eat(void);
int gotowardsgoal(void);
int restup(void);
int goupstairs(int running);
int waitaround(void);
int plunge(void);
int godownstairs(register int running);
int checkcango(register int dir, register int turns);
int grope(register int turns);
int findring(char *name);
int handlering(void);
int readscroll(void);
int quaffpotion(void);
int handleweapon(void);
int handlearmor(void);
int haveweapon(int k, int print);
int havebow(int k, int print);
int havearmor(int k, int print, int rustproof);
int havenamed(stuff otype, char *name);
int havemult(stuff otype, char *name, int count);
int havefood(int n);
int have(stuff otype);
int makemove(int movetype, evalinit_ptr evalinit, evaluate_ptr evaluate, int reevaluate);
int findmove(int movetype, evalinit_ptr evalinit, evaluate_ptr evaluate, int reevaluate);
int validatemap(int movetype, evalinit_ptr evalinit, evaluate_ptr evaluate);
int searchto(int row, int col, evaluate_ptr evaluate, char dir[24][80], int *trow, int *tcol);
int searchfrom(int row, int col, evaluate_ptr evaluate, char dir[24][80], int *trow, int *tcol);
int secretvalue(int r, int c, int depth, int *val, int *avd, int *cont);
int expunpinvalue(int r, int c, int depth, int *val, int *avd, int *cont);
int expvalue(int r, int c, int depth, int *val, int *avd, int *cont);
int gotovalue(int r, int c, int depth, int *val, int *avd, int *cont);
int archeryvalue(int r, int c, int depth, int *val, int *avd, int *cont);
int restvalue(int r, int c, int depth, int *val, int *avd, int *cont);
int downvalue(int r, int c, int depth, int *val, int *avd, int *cont);
int rundoorvalue(int r, int c, int depth, int *val, int *avd, int *cont);
int sleepvalue(int r, int c, int depth, int *val, int *avd, int *cont);
int safevalue(int r, int c, int depth, int *val, int *avd, int *cont);
int exprunvalue(int r, int c, int depth, int *val, int *avd, int *cont);
int zigzagvalue(int r, int c, int depth, int *val, int *avd, int *cont);
int archeryinit(void);
int runinit(void);
int restinit(void);
int expruninit(void);
int rundoorinit(void);
int unpininit(void);
int expunpininit(void);
int genericinit(void);
int gotoinit(void);
int followmap(register int movetype);
int darkroom(void);
int aftermelee(void);
int archery(void);
int callitpending(void);
int canrun(void);
int charsavail(void);
void check_frogue_sync(void);
void clearsendqueue(void);
void close_frogue_debuglog(void);
void copyltm(void);
void currentrectangle(void);
void debuglog_close(void);
void debuglog_open(const char *log);
int doorexplore(void);
int drop(int obj);
void dumpdatabase(void);
void dumpmazedoor(void);
void dumpmonster(void);
void dumpmonstertable(void);
void dumpstuff(void);
void dumpwalls(void);
int expinit(void);
int exploreroom(void);
int fainting(void);
int fightinvisible(void);
int fightmonster(void);
int findarrow(void);
int findroom(void);
int findsafe(void);
void finishcallit(void);
int getlogtoken(void);
void getoldcommand(register char *s);
void getrogver(void);
void givehelp(void);
void halftimeshow(int level);
int has_typeahead(void);
int havearrow(void);
int haveminus(void);
int havemissile(void);
int haveuseless(void);
void holdmonsters(void);
int hungry(void);
void initstufflist(void);
int lightroom(void);
void mapinfer(void);
void markmissingrooms(void);
void newlevel(void);
void newmonsterlevel(void);
void open_frogue_debuglog(const char *file);
void open_frogue_fd(int frogue_fd);
void pauserogue(void);
int pickgenotype(void);
int pickident(void);
int pickupafter(void);
void printexplored(void);
void quit(int code, char* fmt, ...);
int quitforhonors(void);
int replaycommand(void);
int resend(void);
void reset_int(void);
void resetinv(void);
void restoreltm(void);
int rogo_baudrate(void);
void rogo_closelog(void);
void rogo_send(char* f, ...);
void rogue_log_close(void);
void rogue_log_write_command(char c);
int runaway(void);
void sleepmonster(void);
int strategize(void);
void terpmes(void);
int tomonster(void);
int tostuff(void);
int unpin(void);
void versiondep(void);
int wear(int obj);
int wield(int obj);
void setbonuses(void);
void rmove(int count, int d, int mode);
void fmove(int d);
void move1(int d);
void mmove(int d, int mode);
void infername(char *codename, char *name, stuff item_type);
void useobj(char *string);
void addobj(char *codename, int pack_index, stuff item_type);
void dumpdatabase(void);
void timehistory(FILE *f, char sep);
void display(char *s);
int findmonster(char *monster);
int addmonhist(char *monster);
void addstat(register statistic *s, register int datum);
void addprob(register probability *p, register int success);
void addmonster(char ch, int r, int c, int quiescence);
void addstuff(char ch, int row, int col);
int bowclass(int i);
int inventory(char *msgstart, char *msgend);
int md_unlink(const char *file);
int archmonster(register int m, register int trns);
int getfilelength(char *f);
int lock_file(const char *lokfil, int maxtime);
void deleteinv(int pos);
int weaponclass(int i);
void findstairs(int notr, int notc);
int gotowards(int r, int c, int running);
int markcycles(int print);
int wearing(char *name);
int willrust(int obj);
void wakemonster(int dir);
void writestat(register FILE *f, register statistic *s);
int unidentified(stuff otype);
int havering(int k, int print);
void initpool(int k, int m);
int isexplored(int row, int col);
int damagebonus(int strength);
int seemonster(char *monster);
int seeawakemonster(char *monster);
int worth(int obj);
int quaff(int obj);
void waitfor(char *mess);
void removeinv(int pos);
int ringclass(int i);
int puton(int obj);
void printsnap(FILE *f);
int rogue_log_open(const char *filename);
void rogo_srand(int seed);
int point(int obj, int dir);
void parsestat(register char *buf, register statistic *s);
void parseprob(register char *buf, register probability *p);
void writeprob(register FILE *f, register probability *p);
void clearprob(register  probability *p);
int prepareident(int obj, int iscroll);
int fexists(char *fn);
int findmatch(FILE *f, char *s);
int commandcount(char *cmd);
int hitbonus(int strength);
void md_sleep(int s);
void md_usleep(int us);
int getscrpos(char *msg, int *r, int *c);
int useless(int i);
int isholder(register char *monster);
int nextto(register int type, register int r, register int c);
void setknobs(int *newid, int *knb, int *best, int *avg);
int battlestations(int m, char *monster, int mbad, int danger, int mdir, int mdist, int alert, int adj);
int havewand(char *name);
int backtodoor(int dist);
int canbedoor(int deadr, int deadc);
void cancelmove(int movetype);
int findlevel(FILE *f, struct levstruct *lvpos, int *nmlev, int maxnum);
void int_exit(void(*exitproc)(int));
int mazedoor(int row, int col);
int whichroom(register int r, register int c);
int nexttowall(register int r, register int c);
void startreplay(FILE **logf, char *logfname);
int setpsd(int print);
int readgenes(register char *genepool);
void writegenes(register char *genepool);
int know(char *name);
int getmonhist(char *monster, int hitormiss);
int monsternum(char *monster);
int smatch(register char *dat, register char *pat, register char **res);
void mapcharacter(char ch, char *str);
int removering(int obj);
int haveother(stuff otype, int other);
int haveexplored(int n);
int downright(int *drow, int *dcol);

#include <curses.h>
int md_readchar(WINDOW *win);

int version_has_arrow_bug();
int striking_takes_2_charges();
int can_step_on_scare_monster_if_inv_full();
int version_supports_move_without_pickup();
int version_has_invisible_stalker();
int version_has_double_haste_bug();
int version_has_wands();
int version_has_new_monsters();
int get_zap_key();
int version_has_hallucination();
int leather_is_rustproof();
int get_repeat_message_key();
int get_redraw_key();
int new_weapon_protocol();
