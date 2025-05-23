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
    auto locationToSquare = [](std::string from) -> Square
        {
            auto file = std::tolower(from[0]) - 'a';  // 'a' to 0
            auto rank = from[1] - '1';                // '1' to 0, '8' to 7
            return Square{ file, rank };              // no inversion needed
        };

    TEST(pawn_unit_test, white_pawn_test_start)
    {
        std::vector<Move> expectedMoves;
        for (auto rank = 'A'; rank <= 'H'; ++rank) {
            Move expectedMove;
            std::string lock = "A0";
            lock[0] = rank;
            lock[1] = '2';
            expectedMove.from = locationToSquare(lock);
            lock[1] = '3';
            expectedMove.to = locationToSquare(lock);
            expectedMoves.push_back(expectedMove);
            lock[1] = '4';
            expectedMove.to = locationToSquare(lock);
            expectedMoves.push_back(expectedMove);
        }
        TestBoardMoves("k7/8/8/8/8/8/PPPPPPPP/8 w KQkq - 0 1", expectedMoves, Color::White);
    }

    TEST(pawn_unit_test, black_pawn_test_start)
    {
        std::vector<Move> expectedMoves;
        for (auto rank = 'H'; rank >= 'A'; --rank) {
            Move expectedMove;
            expectedMove.from = locationToSquare(std::string() + rank + '7');
            expectedMove.to = locationToSquare(std::string() + rank + '6');

            expectedMoves.push_back(expectedMove);
            expectedMove.to = locationToSquare(std::string() + rank + '5');
            expectedMoves.push_back(expectedMove);
        }
        TestBoardMoves("8/pppppppp/8/8/8/8/8/4K3 w KQkq - 0 1", expectedMoves, Color::Black);
    }

    TEST(pawn_unit_test, white_pawn_test_march)
    {
        std::string fen = "8/8/8/8/8/8/6P1/8 w KQkq - 0 1";

        auto a = 1;
        std::vector<Move> expectedMoves;
        for (auto row = 1; row < 7; ++row) {
            while (a < 8) {
                char r1 = '0' + (char)(7 - a);
                char r2 = '0' + (char)(a);

                fen[12] = r1;
                fen[14] = r2;

                expectedMoves.clear();
                Move expectedMove;

                expectedMove.from = { 7 - a, row };
                expectedMove.to = { 7 - a, row + 1 };
                expectedMoves.push_back(expectedMove);
                if (row == 1) {
                    expectedMove.to = { 7 - a, row + 2 };
                    expectedMoves.push_back(expectedMove);
                }
                TestBoardMoves(fen, expectedMoves, Color::White);

                a++;
            }
        }
    }

    TEST(pawn_unit_test, black_pawn_test_march)
    {
        std::string fen = "8/6p1/8/8/8/8/8/8 w KQkq - 0 1";

        auto a = 1;
        std::vector<Move> expectedMoves;
        for (auto row = 6; row > 0; --row) {
            while (a < 8) {
                char r1 = '0' + (char)(7 - a);
                char r2 = '0' + (char)(a);

                fen[2] = r1;
                fen[4] = r2;

                expectedMoves.clear();
                Move expectedMove;

                expectedMove.from = { 7 - a, row };
                expectedMove.to = { 7 - a, row - 1 };
                expectedMoves.push_back(expectedMove);
                if (row == 6) {
                    expectedMove.to = { 7 - a, row - 2 };
                    expectedMoves.push_back(expectedMove);
                }
                TestBoardMoves(fen, expectedMoves, Color::Black);

                a++;
            }
        }
    }

    TEST(pawn_unit_test, white_pawn_capture_left)
    {

        std::vector<Move> expectedMoves;

        for (int x = 1; x < 8; ++x) { // pawns that can capture left (not on file A)
            for (int y = 1; y < 7; ++y) { // not on rank 1 or 8
                std::string fenSuffix = " w - - 0 1";
                int pawnIndex = (7 - y) * 8 + x;           // white pawn at (x, y)
                int targetIndex = (7 - (y + 1)) * 8 + (x - 1); // black piece at (x-1, y+1)

                std::string board(64, '8');
                board[pawnIndex] = 'P';
                board[targetIndex] = 'n'; // a black knight for capture

                // Build FEN board
                std::string fenBoard;
                for (int r = 0; r < 8; ++r) {
                    int empty = 0;
                    for (int f = 0; f < 8; ++f) {
                        char c = board[r * 8 + f];
                        if (c == '8') {
                            ++empty;
                        }
                        else {
                            if (empty > 0) fenBoard += std::to_string(empty);
                            fenBoard += c;
                            empty = 0;
                        }
                    }
                    if (empty > 0) fenBoard += std::to_string(empty);
                    if (r != 7) fenBoard += '/';
                }

                std::string fullFen = fenBoard + fenSuffix;

                expectedMoves.clear();
                Move captureMove;
                captureMove.from = { x, y };
                captureMove.to = { x - 1, y + 1 };
                expectedMoves.push_back(captureMove);


                Move normalMove;
                normalMove.from = { x, y };
                normalMove.to = { x, y + 1 };
                expectedMoves.push_back(normalMove);
                if (y == 1) {
                    normalMove.to = { x, y + 2 };
                    expectedMoves.push_back(normalMove);
                }
                TestBoardMoves(fullFen, expectedMoves, Color::White);
            }
        }
    }

    TEST(pawn_unit_test, black_pawn_capture_right)
    {
        std::vector<Move> expectedMoves;
        for (int x = 0; x < 7; ++x) { // not on file H
            for (int y = 6; y >= 1; --y) { // ranks 7 to 2 (black pawns can capture to y-1)
                std::string fen = "8/8/8/8/8/8/8/8 b - - 0 1";

                int pawnIndex = (7 - y) * 8 + x;
                int targetIndex = (7 - (y - 1)) * 8 + (x + 1);

                std::string board(64, '8');
                board[pawnIndex] = 'p';    // black pawn
                board[targetIndex] = 'Q';  // white piece to capture

                // Build FEN from board
                std::string fenBoard;
                for (int r = 0; r < 8; ++r) {
                    int empty = 0;
                    for (int f = 0; f < 8; ++f) {
                        char c = board[r * 8 + f];
                        if (c == '8') ++empty;
                        else {
                            if (empty > 0) { fenBoard += std::to_string(empty); empty = 0; }
                            fenBoard += c;
                        }
                    }
                    if (empty > 0) fenBoard += std::to_string(empty);
                    if (r != 7) fenBoard += '/';
                }

                std::string fullFen = fenBoard + " b - - 0 1";

                expectedMoves.clear();

                // Capture right
                expectedMoves.push_back(Move{ Square{x, y}, Square{x + 1, y - 1} });

                // Forward move (if not blocked)
                expectedMoves.push_back(Move{ Square{x, y}, Square{x, y - 1} });

                // Double move if on rank 7
                if (y == 6)
                    expectedMoves.push_back(Move{ Square{x, y}, Square{x, y - 2} });

                if (!TestBoardMoves(fullFen, expectedMoves, Color::Black)) {
                    return;
                }
            }
        }
    }

    TEST(pawn_unit_test, white_pawn_test_nomove)
    {
        std::string fen = "P7/8/8/8/8/8/8/8 w KQkq - 0 1";
        std::vector<Move> expectedMoves;
        expectedMoves.clear();
        TestBoardMoves(fen, expectedMoves, Color::White);
    }

    TEST(pawn_unit_test, white_pawn_every_square)
    {
        auto locationToSquare = [](std::string from) -> Square
            {
                auto file = std::tolower(from[0]) - 'a';
                auto rank = from[1] - '1';
                return Square{ file, rank };
            };

        std::vector<Move> expectedMoves;
        Move expectedMove;

        Fen f;
        for (auto startrank = 'A'; startrank <= 'H'; ++startrank) {

            for (auto startfile = '1'; startfile <= '8'; ++startfile) {
                expectedMoves.clear();
                auto c = startrank - 'A';
                auto r = startfile - '1';

                f.clear();
                f.placePiece({PieceType::Pawn, Color::White}, c, r);

                if (startfile < '8') {
                    expectedMove.from = locationToSquare(std::string() + startrank + startfile);
                    expectedMove.to = locationToSquare(std::string() + startrank + (char)(startfile + 1));
                    expectedMoves.push_back(expectedMove);

                    if (startfile == '2') {
                        expectedMove.to = locationToSquare(std::string() + startrank + (char)(startfile + 2));
                        expectedMoves.push_back(expectedMove);
                    }
                }
                TestBoardMoves(f.toString(), expectedMoves, Color::White);
            }
        }
    }
}

