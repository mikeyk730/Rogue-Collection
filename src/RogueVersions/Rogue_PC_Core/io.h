#include <iostream>

bool short_msgs();

void set_small_screen_mode(bool enable);
bool in_small_screen_mode();

void set_brief_mode(bool enable);
bool in_brief_mode();

//msg: Display a message at the top of the screen.
void msg(const char *fmt, ...);
void debug(const char *fmt, ...);
//unsaved_msg: Same as above, but is replaced by a new msg instead of showing -More-
void unsaved_msg(const char *fmt, ...);

void reset_msg_position();
void clear_msg();

//addmsg: Add things to the current message
void addmsg(const char *fmt, ...);

//endmsg: Display a new msg (giving him a chance to see the previous one if it is up there with the -More-)
void endmsg();

//More: tag the end of a line and wait for a space
void more();
void more(const char *msg);

//doadd: Perform an add onto the message buffer
void doadd(const char *fmt, ...);

//putmsg: put a msg on the line, make sure that it will fit, if it won't scroll msg sideways until he has read it all
void putmsg(int msgline, const char *msg);

//scrl: scroll a message across the line
void scrl(int msgline, const char *str1, const char *str2);

//unctrl: Print a readable version of a certain character
char *unctrl(unsigned char ch);

//update_status_bar: Display the important stats line.  Keep the cursor where it was.
void update_status_bar();

//wait_for: Sit around until the guy types the right key
void wait_for(char ch);

//show_win: Function used to display a window and wait before returning
void show_win(const char *message);

//readchar: Return the next input character, from the macro or from the keyboard.
int readchar();

//This routine reads information from the keyboard. It should do all the strange processing that is needed to retrieve sensible data from the user
int getinfo(char *str, int size);

//clear_typeahead_buffer: Flush typeahead for traps, etc.
void clear_typeahead_buffer();

void tick_pause();
void pause(int ms);
void alert();

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

void str_attr(const char *str);

//key_state:
void handle_key_state();

template <typename T>
std::ostream& write(std::ostream& out, T t) {
    out.write((char*)&t, sizeof(T));
    return out;
}

template <typename T>
std::istream& read(std::istream& in, T* t) {
    in.read((char*)t, sizeof(T));
    return in;
}

std::ostream& write_string(std::ostream& out, const std::string& s);
std::istream& read_string(std::istream& in, std::string* s);
