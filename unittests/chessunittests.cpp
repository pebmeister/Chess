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

#pragma warning(disable:4996)

namespace move_unit_test
{
    static bool TestBoardMoves(std::string fen, std::vector<Move>& expectedMoves, Color side)
    {
        Board board(fen);
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

    TEST(pawn_unit_test, white_pawn_test_start)
    {
        auto locationToSquare = [](std::string from) -> Square
            {
                auto file = std::tolower(from[0]) - 'a';
                auto rank = from[1] - '1';
                return Square{ file, rank };
            };

        std::vector<Move> expectedMoves;
        for (auto row = 'A'; row <= 'H'; ++row) {
            Move expectedMove;
            expectedMove.from = locationToSquare(std::string() + row + '2');
            expectedMove.to = locationToSquare(std::string() + row + '3');

            expectedMoves.push_back(expectedMove);
            expectedMove.to = locationToSquare(std::string() + row + '4');
            expectedMoves.push_back(expectedMove);
        }
        TestBoardMoves("8/8/8/8/8/8/PPPPPPPP/4K3 w KQkq - 0 1", expectedMoves, Color::White);
    }

    TEST(pawn_unit_test, black_pawn_test_start)
    {
        auto locationToSquare = [](std::string from) -> Square
            {
                auto file = std::tolower(from[0]) - 'a';
                auto rank = from[1] - '1';
                return Square{ file, rank };
            };

        std::vector<Move> expectedMoves;
        for (auto row = 'H'; row >= 'A'; --row) {
            Move expectedMove;
            expectedMove.from = locationToSquare(std::string() + row + '7');
            expectedMove.to = locationToSquare(std::string() + row + '6');

            expectedMoves.push_back(expectedMove);
            expectedMove.to = locationToSquare(std::string() + row + '5');
            expectedMoves.push_back(expectedMove);
        }
        TestBoardMoves("8/pppppppp/8/8/8/8/8/4K3 w KQkq - 0 1", expectedMoves, Color::Black);
    }

    TEST(pawn_unit_test, white_pawn_test_march)
    {
        auto locationToSquare = [](std::string from) -> Square
            {
                auto file = std::tolower(from[0]) - 'a';
                auto rank = from[1] - '1';
                return Square{ file, rank };
            };
        std::string fen = "8/8/8/8/8/8/6P1/4K3 w KQkq - 0 1";

        auto a = 1;
        std::vector<Move> expectedMoves;
        for (auto row = 8; row < 8; ++row) {
            while (a < 8) {
                char r1 = '0' + (char)(7 - a);
                char r2 = '0' + (char)(a);

                fen[12] = r1;
                fen[14] = r2;

                expectedMoves.clear();
                Move expectedMove;

                expectedMove.from = { 7 - a, row };
                expectedMove.to = { 7 - a, row + 1};
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
        auto locationToSquare = [](std::string from) -> Square
            {
                auto file = std::tolower(from[0]) - 'a';
                auto rank = from[1] - '1';
                return Square{ file, rank };
            };
        std::string fen = "8/6p1/8/8/8/8/8/4K3 w KQkq - 0 1";

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
        auto locationToSquare = [](std::string from) -> Square
            {
                auto file = std::tolower(from[0]) - 'a';
                auto rank = from[1] - '1';
                return Square{ file, rank };
            };

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
                if (y == 1)                     {
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
                std::string fen = "8/8/8/8/8/8/8/4K3 b - - 0 1";

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
        std::string fen = "P7/8/8/8/8/8/8/4K3 w KQkq - 0 1";
        std::vector<Move> expectedMoves;
        expectedMoves.clear();
        TestBoardMoves(fen, expectedMoves, Color::White);
    }

}
