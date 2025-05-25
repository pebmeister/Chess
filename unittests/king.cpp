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
    TEST(king_unit_test, white_king_check1_test)
    {
        std::vector<Move> expectedMoves;
        Fen fen("k7/8/8/8/8/8/5q2/4K3 w - - 0 1");
        expectedMoves.push_back(Move({ 4, 0 }, { 3, 0 }));
        expectedMoves.push_back(Move({ 4, 0 }, { 5, 1 }));
        TestBoardMoves(fen.toString(), expectedMoves, Color::White);
    }

    TEST(king_unit_test, white_king_check2_test)
    {
        Fen fen("8/8/8/8/8/4k3/5q2/4K3 w - - 0 1");
        std::vector<Move> expectedMoves;
        expectedMoves.push_back(Move({ 4, 0 }, { 3, 0 }));
        TestBoardMoves(fen.toString(), expectedMoves, Color::White);
    }

    TEST(king_unit_test, white_king_check3_test)
    {
        std::vector<Move> expectedMoves;
        Fen fen("k7/8/8/8/8/8/5q2/5K3 w - - 0 1");
        expectedMoves.push_back(Move({ 5, 0 }, { 5, 1 }));
        TestBoardMoves(fen.toString(), expectedMoves, Color::White);
    }

    TEST(king_unit_test, white_king_check4_test)
    {
        Fen fen("k7/8/8/8/8/4p3/5q2/4K3 w - - 0 1");
        std::vector<Move> expectedMoves;
        expectedMoves.push_back(Move({ 4, 0 }, { 3, 0 }));
        TestBoardMoves(fen.toString(), expectedMoves, Color::White);
    }

    TEST(king_unit_test, white_king_checkmate1_test)
    {
        Fen fen("k7/8/8/8/8/4r2/4q3/4K3 w - - 0 1");
        std::vector<Move> expectedMoves;
        TestBoardMoves(fen.toString(), expectedMoves, Color::White);
    }

    TEST(king_unit_test, white_king_checkmate2_test)
    {
        Fen fen("k7/3qrr2/8/8/8/8/8/4K3 w - - 0 1");
        std::vector<Move> expectedMoves;
        TestBoardMoves(fen.toString(), expectedMoves, Color::White);
    }

}

