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
#include "chesstypes.h"

#pragma warning(disable:4996)
namespace engine_unit_test
{
    const std::array<PieceType, 7> pieces
    {
        PieceType::King, PieceType::Queen, PieceType::Rook, PieceType::Bishop, 
        PieceType::Knight, PieceType::Pawn, PieceType::None
    };

    const int centerSquares[4][2] = { {3,3}, {3,4}, {4,3}, {4,4} };
    const int pieceValues[] = { 0, 100, 320, 330, 500, 900, 20000 };

    TEST(evalute_unit_test, evaluate_start)
    {
        Board b("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
        Engine e;

        auto whiteExpectedScore = pieceValues[(int)PieceType::Queen] + pieceValues[(int)PieceType::King];
        for (auto doublePiece : { PieceType::Rook, PieceType::Bishop, PieceType::Knight }) {
            whiteExpectedScore += (pieceValues[(int)doublePiece] * 2);
        }
        whiteExpectedScore += pieceValues[(int)PieceType::Pawn] * 8;

        auto blackExpectedScore = pieceValues[(int)PieceType::Queen] + pieceValues[(int)PieceType::King];
        for (auto doublePiece : { PieceType::Rook, PieceType::Bishop, PieceType::Knight }) {
            blackExpectedScore += (pieceValues[(int)doublePiece] * 2);
        }
        blackExpectedScore += pieceValues[(int)PieceType::Pawn] * 8;

        for (auto turn : { Color::White, Color::Black }) {
            b.turn = turn;
            auto score = e.evaluate(b);
            auto expectedScore = b.turn == Color::White
                ? whiteExpectedScore - blackExpectedScore
                : blackExpectedScore - whiteExpectedScore;

            EXPECT_EQ(score, expectedScore);
        }
    }

    TEST(evalute_unit_test, evaluate_single)
    {
        Fen f;
        Engine e;
        for (auto piece : pieces) {
            for (auto x = 0; x < 8; ++x) {
                for (auto y = 0; y < 8; ++y) {
                    for (auto color : { Color::White, Color::Black }) {
                        f.clear();
                        f.turn = color;
                        f.placePiece({ piece, color }, x, y);
                        Board b(f.toString());

                        auto expectedScore = pieceValues[(int)piece];
                        if (piece == PieceType::Pawn || piece == PieceType::Knight) {

                            for (auto square : centerSquares) {
                                if (x == square[0] && y == square[1]) {
                                    expectedScore += 200;
                                    break;
                                }
                            }
                        }

                        auto score = e.evaluate(b);
                        EXPECT_EQ(score, expectedScore);
                    }
                }
            }

        }
    }
}