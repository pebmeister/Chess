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
    static std::map<PieceType, std::vector<std::pair<int, int>>> offsetMap =
    {
        {   PieceType::King,    { { -1, 1 }, { 0, 1  }, { 1, 1  }, { -1, 0 }, { 1, 0 }, { -1, -1 }, { 0, -1 }, { 1,  -1} }},
        {   PieceType::Queen,   { { -1, 1 }, { 0, 1  }, { 1, 1  }, { -1, 0 }, { 1, 0 }, { -1, -1 }, { 0, -1 }, { 1, -1 } }},
        {   PieceType::Rook,    { { 0, 1  }, { 0, -1 }, { 1, 0  }, { -1, 0 } } },
        {   PieceType::Bishop,  { { -1, 1 }, { 1, 1  }, { -1, -1}, { 1, -1 } } },
        {   PieceType::Knight,  { { 1,  2 }, { 2,  1 }, { 2, -1 }, { 1, -2 }, {-1, -2}, {-2, -1  }, {-2,  1 }, {-1,  2} } }
    };

    static std::map<PieceType, bool> singleMoveMap =
    {
        { PieceType::King, true },
        { PieceType::Queen, false },
        { PieceType::Rook, false },
        { PieceType::Bishop, false },
        { PieceType::Knight, true },
    };

    TEST(basicmove_unit_test, move_test)
    {
        Fen f;
        std::vector<Move> psuedoExpectedMoves;

        for (auto color : { Color::White, Color::Black }) {
            f.turn = color;
            for (auto piece : { PieceType::King, PieceType::Queen, PieceType::Rook, PieceType::Bishop, PieceType::Knight }) {
                auto& offsets = offsetMap[piece];
                auto single = singleMoveMap[piece];

                for (auto c = 0; c < 8; ++c) {
                    for (auto r = 0; r < 8; ++r) {
                        f.clear();
                        f.placePiece({ piece, color }, c, r);
                        psuedoExpectedMoves.clear();

                        GenerateSlideMoves(psuedoExpectedMoves, f, offsets, c, r, single);
                        auto expectedMoves = FilterMoves(psuedoExpectedMoves, f);

                        TestBoardMoves(f.toString(), expectedMoves, color);
                    }
                }
            }
        }
    }

    TEST(basicmove_unit_test, move_obsticle_test)
    {
        Fen f;
        std::vector<Move> psuedoExpectedMoves;

        for (auto color : { Color::White, Color::Black }) {
            f.turn = color;

            auto opColor = color == Color::White ? Color::Black : Color::White;
            for (auto piece : { PieceType::King, PieceType::Queen, PieceType::Rook, PieceType::Bishop, PieceType::Knight }) {
                auto& offsets = offsetMap[piece];
                auto single = singleMoveMap[piece];

                for (auto opPawnRow = 6; opPawnRow > 1; --opPawnRow) {
         
                    for (auto c = 0; c < 8; ++c) {
                        for (auto r = 0; r < opPawnRow; ++r) {
                            f.clear();

                            for (auto cc = 0; cc < 8; ++cc) {
                                f.placePiece({ PieceType::Pawn, opColor }, cc, opPawnRow);
                            }

                            f.placePiece({ piece, color }, c, r);
                            psuedoExpectedMoves.clear();

                            GenerateSlideMoves(psuedoExpectedMoves, f, offsets, c, r, single);
                            auto expectedMoves = FilterMoves(psuedoExpectedMoves, f);

                            TestBoardMoves(f.toString(), expectedMoves, color);
                        }
                    }
                }
            }
        }
    }
}