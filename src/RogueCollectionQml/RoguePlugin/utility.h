#pragma once
#include <iosfwd>

template <typename T>
std::ostream& Write(std::ostream& out, T t) {
    out.write((char*)&t, sizeof(T));
    return out;
}

template <typename T>
std::istream& Read(std::istream& in, T* t) {
    in.read((char*)t, sizeof(T));
    return in;
}

std::string GetResourcePath(const std::string& filename);
void DisplayMessage(const std::string& type, const std::string& title, const std::string& msg);
void QuitApplication();

std::ostream& WriteShortString(std::ostream& out, const std::string& s);
std::istream& ReadShortString(std::istream& in, std::string* s);

void throw_error(const std::string &msg);

void Delay(int ms);
std::string GetTimeString();
