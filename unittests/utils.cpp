// Written by Paul Baxter
#include <gtest/gtest.h>
#include <string>
#include <stdint.h>
#include <algorithm>

#include "utils.h"

namespace move_unit_test
{
    bool TestBoardMoves(std::string fen, std::vector<Move>& expectedMoves, Color side)
    {
        Board board(fen);
        auto boardstr = board.toString();
        auto generated = board.generateLegalMoves(side);

        // Already defined
        auto encode = [](const Move& m)
            {
                return (m.from.y * 8 + m.from.x) * 64 + (m.to.y * 8 + m.to.x);
            };

        auto decode = [](uint32_t code)
            {
                int fromIdx = code / 64;
                int toIdx = code % 64;
                Square from{ fromIdx % 8, fromIdx / 8 };
                Square to{ toIdx % 8, toIdx / 8 };
                return Move{ from, to };
            };

        auto comp = [](uint32_t a, uint32_t b)
            {
                return a < b;
            };

        std::vector<uint32_t>actualVec;
        for (const auto& m : generated)
            actualVec.push_back(encode(m));

        std::vector<uint32_t>expectedVec;
        for (const auto& m : expectedMoves)
            expectedVec.push_back(encode(m));

        std::sort(actualVec.begin(), actualVec.end(), comp);
        std::sort(expectedVec.begin(), expectedVec.end(), comp);

        std::vector<uint32_t> missing;
        auto miss = std::set_difference(expectedVec.begin(), expectedVec.end(),
            actualVec.begin(), actualVec.end(), inserter(missing, missing.begin()));

        std::vector<uint32_t> extra;
        auto ex = std::set_difference(actualVec.begin(), actualVec.end(),
            expectedVec.begin(), expectedVec.end(), inserter(extra, extra.begin()));

        auto count = missing.size() + extra.size();
        if (count != 0) {
            std::cout << "Fen: " << fen << "\n===========================\n";
            Board board(fen);
            std::cout << board.toString() << "\n============================\n";

            // Missing moves
            for (const auto& m : missing) {
                std::cout << "Missing expected move: " + decode(m).toString() << "\n";
            }

            // Extra moves
            for (const auto& m : extra) {
                std::cout << "Unexpected move: " + decode(m).toString() << "\n";
            }

            EXPECT_EQ(count, 0);
        }
        return count == 0;
    }

    void GenerateSlideMoves(std::vector<Move>& generatedMoves, const Fen& f, const std::vector<std::pair<int, int>>& moveOffsets, const int x, const int y, bool single)
    {
        auto isOccupied = [&f](int x, int y) -> bool
            {
                if (x < 0 || x >= 8 || y < 0 || y >= 8) return false;
                int index = y * 8 + x;
                return (f.allPieces >> index) & 1ULL;
            };

        Square from = Square{ x, y };

        for (auto& [dx, dy] : moveOffsets) {
            int tx = x + dx;
            int ty = y + dy;
            while (tx >= 0 && tx < 8 && ty >= 0 && ty < 8) {
                
                generatedMoves.emplace_back(from, Square{ tx, ty });

                if (single || isOccupied(tx, ty)) {
                    break;
                }

                tx += dx;
                ty += dy;
            }

        }
    }

    std::vector<Move> FilterMoves(const std::vector<Move>& psuedoMoves, Fen& f)
    {
        if ((f.turn == Color::White && f.white_kings == 0) ||
            (f.turn == Color::Black && f.black_kings == 0)) {
            return psuedoMoves;
        }

        std::vector<Move> filteredMoves;
        for (const auto& move : psuedoMoves) {
            Board b(f.toString());
            b.makeMove(move);
            if (!b.isInCheck(f.turn))
                filteredMoves.push_back(move);
        }
        return filteredMoves;
    }

}