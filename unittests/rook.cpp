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
    void placeRook(std::string& boardStr, int x, int y)
    {
        int row = 7 - y;  // board row (0 at top)
        int col = x;      // file (0 = a)
        int index = row * 19 + ((col * 2) + 2);
        boardStr[index] = 'R';
    }

    TEST(rook_unit_test, white_rook_test_start)
    {
        auto locationToSquare = [](std::string from) -> Square
            {
                auto file = std::tolower(from[0]) - 'a';
                auto rank = from[1] - '1';
                return Square{ file, rank };
            };

        std::vector<Move> expectedMoves;
        Move expectedMove;

        for (auto startrank = 'A'; startrank <= 'H'; ++startrank) {

            for (auto startfile = '1'; startfile <= '8'; ++startfile) {
                expectedMoves.clear();
                auto c = startrank - 'A';
                auto r = startfile - '1';

                std::string boardStr = std::string(
                    "8 . . . . . . . . \n"
                    "7 . . . . . . . . \n"
                    "6 . . . . . . . . \n"
                    "5 . . . . . . . . \n"
                    "4 . . . . . . . . \n"
                    "3 . . . . . . . . \n"
                    "2 . . . . . . . . \n"
                    "1 . . . . . . . . \n"
                    "  a b c d e f g h\n");

                placeRook(boardStr, c, r);
                auto fen = boardToFEN(boardStr, Color::White);

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
    }
}