#include <windows.h> 
#include "rogue.h"

int tick = 0;

void move(int y, int x) 
{ 
   HANDLE hStdout = GetStdHandle(STD_OUTPUT_HANDLE) ; 
   COORD position = { x, y } ; 
   SetConsoleCursorPosition( hStdout, position ) ; 
}

void beep()
{}

int csum()
{
   return CSUM;
}

//TODO: return char at position
char curch()
{
   return 'a';
}

void _halt(){}

int no_char(){ return 0; }

void dmain(char* d, int a, int b, int c)
{}

void dmaout(char* d, int a, int b, int c)
{}


