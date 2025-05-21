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

        auto comp = [](int a, int b)
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

    std::string boardToFEN(const std::string& input, Color turn)
    {
        std::istringstream stream(input);
        std::string line, fen;
        int rowCount = 0;

        while (std::getline(stream, line)) {
            // Skip the coordinate line at the bottom
            if (line.empty() || std::isdigit(line[0]) == 0)
                continue;

            std::istringstream linestream(line);
            std::string rowLabel;
            linestream >> rowLabel; // discard rank number (8,7,...)

            int emptyCount = 0;
            char piece;
            while (linestream >> piece) {
                if (piece == '.') {
                    ++emptyCount;
                }
                else {
                    if (emptyCount > 0) {
                        fen += std::to_string(emptyCount);
                        emptyCount = 0;
                    }
                    fen += piece;
                }
            }
            if (emptyCount > 0)
                fen += std::to_string(emptyCount);

            ++rowCount;
            if (rowCount < 8)
                fen += '/';
        }

        if (turn == Color::White)
            fen += " w ";
        else
            fen += " b ";
        // Append standard FEN suffix
        fen += "KQkq - 0 1";
        return fen;
    }


}