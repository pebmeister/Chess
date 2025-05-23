#pragma once
#include <vector>
#include <string>

#include "engine.h"

namespace move_unit_test
{
    extern bool TestBoardMoves(std::string fen, std::vector<Move>& expectedMoves, Color side);
    extern void GenerateSlideMoves(std::vector<Move>& generatedMoves, const Fen& f, const std::vector<std::pair<int, int>>& moveOffsets, const int x, const int y, bool single = false);
    extern std::vector<Move> FilterMoves(const std::vector<Move>& psuedoMoves, Fen& f);
}