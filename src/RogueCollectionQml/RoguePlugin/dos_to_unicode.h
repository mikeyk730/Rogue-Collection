#pragma once
#include <cstdint>
#include <string>

uint16_t DosToUnicode(unsigned char ch);

typedef std::basic_string<uint16_t, std::char_traits<uint16_t>, std::allocator<uint16_t> > uint16_string;

uint16_string DosToUnicode(const std::string& s);
