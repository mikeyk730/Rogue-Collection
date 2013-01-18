//msg: Display a message at the top of the screen.
void ifterse(char *tfmt, char *fmt, ...);

void msg(char *fmt, ...);

//addmsg: Add things to the current message
void addmsg(char *fmt, ...);

//endmsg: Display a new msg (giving him a chance to see the previous one if it is up there with the -More-)
void endmsg();

//More: tag the end of a line and wait for a space
void more(char *msg);

//doadd: Perform an add onto the message buffer
void doadd(char *fmt, int a1, int a2, int a3, int a4, int a5);

//putmsg: put a msg on the line, make sure that it will fit, if it won't scroll msg sideways until he has read it all
void putmsg(int msgline, char *msg);

//scrl: scroll a message across the line
void scrl(int msgline, char *str1, char *str2);

//unctrl: Print a readable version of a certain character
char *unctrl(unsigned char ch);

//status: Display the important stats line.  Keep the cursor where it was.
void status();

//wait_for: Sit around until the guy types the right key
void wait_for(char ch);

//show_win: Function used to display a window and wait before returning
void show_win(int *scr, char *message);

//This routine reads information from the keyboard. It should do all the strange processing that is needed to retrieve sensible data from the user
int getinfo(char *str, int size);

void backspace();

//str_attr: format a string with attributes.
//
//    formats:
//        %i - the following character is turned inverse vidio
//        %I - All characters upto %$ or null are turned inverse vidio
//        %u - the following character is underlined
//        %U - All characters upto %$ or null are underlined
//        %$ - Turn off all attributes
//
//     Attributes do not nest, therefore turning on an attribute while
//     a different one is in effect simply changes the attribute.
//
//     "No attribute" is the default and is set on leaving this routine
//
//     Eventually this routine will contain colors and character intensity
//     attributes.  And I'm not sure how I'm going to interface this with
//     printf certainly '%' isn't a good choice of characters.  jll.

void str_attr(char *str);

//key_state:
void SIG2();

//Replacement printf
//Michael Toy, AI Design, January 1984

char *my_stccpy(char* a, char* b, int c);

char *noterse(char *str);
