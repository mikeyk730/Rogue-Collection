//setup: Get starting setup for all games
void setup_screen();

//clear_typeahead_buffer: Flush typeahead for traps, etc.
void clear_typeahead_buffer();

void credits();

int getkey();

//readchar: Return the next input character, from the macro or from the keyboard.
int readchar();

void beep();
void tick_pause();

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

