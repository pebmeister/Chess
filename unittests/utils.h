#pragma once
#include <vector>
#include <string>

#include "engine.h"

namespace move_unit_test
{
    extern bool TestBoardMoves(std::string fen, std::vector<Move>& expectedMoves, Color side);
    extern std::string boardToFEN(const std::string& input, Color turn);
    extern void placePiece(std::string& boardStr, char piece, int x, int y);

}