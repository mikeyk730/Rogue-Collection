#pragma once
#include <stdio.h>
#include "types.h"

char *findentry_getfakename(char* string, stuff item_type);
char *findentry_getrealname(char* string, stuff item_type);
char *getname();
char *itemstr(register int i);
char *md_gethomedir();
char *md_gethostname();
char *md_getpass(char *prompt);
char *md_getrealname(int uid);
char *md_getshell();
char *md_getusername(int uid);
char *md_strdup(const char *s);
char *md_unctrl(char ch);
char *monname(char m);
char *realname(char* codename);
char *statusline();
char *strdup(const char *s);
char commandarg(char* cmd, int n);
char functionchar(char* cmd);
char getroguechar();
char getroguetoken();
char *getname();
char getroguetoken(void);
const char *getLockFile();
const char *getRgmDir();
double mean(register statistic *s);
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
