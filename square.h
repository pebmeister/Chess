#pragma once
#include <string>

struct Square {
    int x, y;
    bool operator==(const Square& other) const { return x == other.x && y == other.y; }
    const std::string toString() const
    {
        auto rank = (char)((char)x + 'a');
        auto file = (char)((char)y + '1');
        auto s = std::string("") + rank;
        s += file;
        return s;
    }
};
