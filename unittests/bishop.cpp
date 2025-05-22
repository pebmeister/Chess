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

    TEST(bishop_unit_test, white_bishop_test)
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

                placePiece(boardStr, 'B', c, r);
                auto fen = boardToFEN(boardStr, Color::White);

                expectedMove.from = locationToSquare(std::string() + startrank + startfile);

                auto file = (char)(startfile + 1);
                auto rank = (char)(startrank + 1);
                while (file <= '8' && rank <= 'H') {
                    expectedMove.to = locationToSquare(std::string() + rank + file);
                    expectedMoves.push_back(expectedMove);
                    file++;
                    rank++;
                }
                file = (char)(startfile - 1);
                rank = (char)(startrank - 1);
                while (file >= '1' && rank >= 'A') {
                    expectedMove.to = locationToSquare(std::string() + rank + file);
                    expectedMoves.push_back(expectedMove);
                    file--;
                    rank--;
                }
                rank = (char)(startrank - 1);
                file = (char)(startfile + 1);
                while (file <= '8' && rank >= 'A') {
                    expectedMove.to = locationToSquare(std::string() + rank + file);
                    expectedMoves.push_back(expectedMove);
                    file++;
                    rank--;
                }
                rank = (char)(startrank + 1);
                file = (char)(startfile - 1);
                while (file >= '1' && rank <= 'H') {
                    expectedMove.to = locationToSquare(std::string() + rank + file);
                    expectedMoves.push_back(expectedMove);
                    file--;
                    rank++;
                }
                TestBoardMoves(fen, expectedMoves, Color::White);
            }
        }
    }
}