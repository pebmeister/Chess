// Written by Paul Baxter
#include <gtest/gtest.h>
#include <string>
#include <stdint.h>
#include <algorithm>

#include "fen.h"
#include "board.h"
#include "bitboard.h"
#include "engine.h"
#include "chess.h"
#include "utils.h"

#pragma warning(disable:4996)

namespace move_unit_test
{
    TEST(rook_unit_test, white_rook_test_start)
    {
        auto locationToSquare = [](std::string from) -> Square
            {
                auto file = std::tolower(from[0]) - 'a';
                auto rank = from[1] - '1';
                return Square{ file, rank };
            };

        std::string boardStr = std::string(
            "8 r n b q k b n r\n"
            "7 . . . . . . . . \n"
            "6 . . . . . . . . \n"
            "5 . . . . . . . . \n"
            "4 . . . . . . . . \n"
            "3 . . . . . . . . \n"
            "2 . . . . . . . . \n"
            "1 . . . R . . . . \n"
            "  a b c d e f g h\n");

        auto fen = boardToFEN(boardStr, Color::White);

        std::vector<Move> expectedMoves;
            Move expectedMove;
            auto startrank = 'D';
            auto startfile = '1';
            expectedMove.from = locationToSquare(std::string() + startrank + startfile);
            for (auto file = '1'; file <= '8'; ++file) {
                if (file != startfile) {
                    expectedMove.to = locationToSquare(std::string() + startrank + file);
                    expectedMoves.push_back(expectedMove);
                }
            }
            for (auto rank = 'A'; rank <= 'H'; ++rank) {
                if (rank != startrank) {
                    expectedMove.to = locationToSquare(std::string() + rank + startfile);
                    expectedMoves.push_back(expectedMove);
                }
            }
            TestBoardMoves(fen, expectedMoves, Color::White);

    }
}