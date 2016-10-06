#pragma once

#include <iosfwd>

//Coordinate data type
struct Coord
{
    int x;
    int y;
};

Coord north(Coord p);
Coord south(Coord p);
Coord east(Coord p);
Coord west(Coord p);

bool operator==(const Coord& c1, const Coord& c2);
bool operator!=(const Coord& c1, const Coord& c2);
Coord operator+(const Coord& c1, const Coord& c2);
std::ostream& operator<<(std::ostream& os, Coord c);
