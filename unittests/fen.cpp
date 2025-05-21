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

namespace move_unit_test
{
    static void fenTest(std::string fen, std::string expectedBoard,
        uint64_t expected_white_pawns,
        uint64_t expected_white_knights,
        uint64_t expected_white_bishops,
        uint64_t expected_white_rooks,
        uint64_t expected_white_queens,
        uint64_t expected_white_kings,

        uint64_t expected_black_pawns,
        uint64_t expected_black_knights,
        uint64_t expected_black_bishops,
        uint64_t expected_black_rooks,
        uint64_t expected_black_queens,
        uint64_t expected_black_kings
        )
    {
        Board board(fen);
        auto boardstr = board.toString();
        auto boardEqual = boardstr == expectedBoard;
        auto bitBoardsEqual =
            expected_white_pawns == board.white_pawns &&
            expected_white_knights == board.white_knights &&
            expected_white_bishops == board.white_bishops &&
            expected_white_rooks == board.white_rooks &&
            expected_white_queens == board.white_queens &&
            expected_white_kings == board.white_kings &&

            expected_black_pawns == board.black_pawns &&
            expected_black_knights == board.black_knights &&
            expected_black_bishops == board.black_bishops &&
            expected_black_rooks == board.black_rooks &&
            expected_black_queens == board.black_queens &&
            expected_black_kings == board.black_kings;

        if (!boardEqual || !bitBoardsEqual) {
            std::cout <<
                "FEN " << fen << "\n\n" <<
                "\nGot\n" <<
                "==============================\n" <<
                boardstr <<
                "\n==============================\n" <<
                "White rooks:   " << std::bitset<64>{board.white_rooks} << "\n" <<
                "White knights: " << std::bitset<64>{board.white_knights} << "\n" <<
                "White pawns:   " << std::bitset<64>{board.white_pawns} << "\n" <<
                "White bishops: " << std::bitset<64>{board.white_bishops} << "\n" <<
                "White queens:  " << std::bitset<64>{board.white_queens} << "\n" <<
                "White kings:   " << std::bitset<64>{board.white_kings} << "\n" <<
                "\n" <<
                "Black rooks:   " << std::bitset<64>{board.black_rooks} << "\n" <<
                "Black knights: " << std::bitset<64>{board.black_knights} << "\n" <<
                "Black pawns:   " << std::bitset<64>{board.black_pawns} << "\n" <<
                "Black bishops: " << std::bitset<64>{board.black_bishops} << "\n" <<
                "Black queens:  " << std::bitset<64>{board.black_queens} << "\n" <<
                "Black kings:   " << std::bitset<64>{board.black_kings} << "\n"

                "\nExpected\n" <<
                "==============================\n" <<
                expectedBoard <<
                "White rooks:   " << std::bitset<64>{expected_white_rooks} << "\n" <<
                "White knighs:  " << std::bitset<64>{expected_white_knights} << "\n" <<
                "White pawns:   " << std::bitset<64>{expected_white_pawns} << "\n" <<
                "White bishops: " << std::bitset<64>{expected_white_bishops} << "\n" <<
                "White queen:   " << std::bitset<64>{expected_white_queens} << "\n" <<
                "White kings:   " << std::bitset<64>{expected_white_kings} << "\n" <<
                "\n" <<
                "Black rooks:   " << std::bitset<64>{expected_black_rooks} << "\n" <<
                "Black knighs:  " << std::bitset<64>{expected_black_knights} << "\n" <<
                "Black pawns:   " << std::bitset<64>{expected_black_pawns} << "\n" <<
                "Black bishops: " << std::bitset<64>{expected_black_bishops} << "\n" <<
                "Black queen:   " << std::bitset<64>{expected_black_queens} << "\n" <<
                "Black kings:   " << std::bitset<64>{expected_black_kings} << "\n";

            EXPECT_TRUE(boardEqual && bitBoardsEqual);
        }
    }

    TEST(fen_unit_test, fen_opening_test)
    {
        auto fen = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
        auto expectedBoard = std::string(
            "8 r n b q k b n r \n"
            "7 p p p p p p p p \n"
            "6 . . . . . . . . \n"
            "5 . . . . . . . . \n"
            "4 . . . . . . . . \n"
            "3 . . . . . . . . \n"
            "2 P P P P P P P P \n"
            "1 R N B Q K B N R \n"
            "  a b c d e f g h\n"
        );

        uint64_t expected_white_pawns = 0x000000000000FF00ULL;
        uint64_t expected_black_pawns = 0x00FF000000000000ULL;
        uint64_t expected_white_knights = 0x0000000000000042ULL;
        uint64_t expected_black_knights = 0x4200000000000000ULL;
        uint64_t expected_white_bishops = 0x0000000000000024ULL;
        uint64_t expected_black_bishops = 0x2400000000000000ULL;
        uint64_t expected_white_rooks = 0x0000000000000081ULL;
        uint64_t expected_black_rooks = 0x8100000000000000ULL;
        uint64_t expected_white_queens = 0x0000000000000008ULL;
        uint64_t expected_black_queens = 0x0800000000000000ULL;
        uint64_t expected_white_kings = 0x0000000000000010ULL;
        uint64_t expected_black_kings = 0x1000000000000000ULL;

        fenTest(fen, expectedBoard,
            expected_white_pawns,
            expected_white_knights,
            expected_white_bishops,
            expected_white_rooks,
            expected_white_queens,
            expected_white_kings,

            expected_black_pawns,
            expected_black_knights,
            expected_black_bishops,
            expected_black_rooks,
            expected_black_queens,
            expected_black_kings
            );
    }

    TEST(fen_unit_test, fen_test_2)
    {
        auto fen = "8/8/8/8/8/8/1p6/N7 b KQkq - 0 1";
        auto expectedBoard = std::string(
            "8 . . . . . . . . \n"
            "7 . . . . . . . . \n"
            "6 . . . . . . . . \n"
            "5 . . . . . . . . \n"
            "4 . . . . . . . . \n"
            "3 . . . . . . . . \n"
            "2 . p . . . . . . \n"
            "1 N . . . . . . . \n"
            "  a b c d e f g h\n"
        );
        uint64_t expected_white_pawns = 0x0000000000000000ULL;
        uint64_t expected_black_pawns = 0x00000000000000200ULL;
        uint64_t expected_white_knights = 0x0000000000000001ULL;
        uint64_t expected_black_knights = 0x0000000000000000ULL;
        uint64_t expected_white_bishops = 0x0000000000000000ULL;
        uint64_t expected_black_bishops = 0x0000000000000000ULL;
        uint64_t expected_white_rooks = 0x0000000000000000ULL;
        uint64_t expected_black_rooks = 0x0000000000000000ULL;
        uint64_t expected_white_queens = 0x0000000000000000ULL;
        uint64_t expected_black_queens = 0x0000000000000000ULL;
        uint64_t expected_white_kings = 0x0000000000000000ULL;
        uint64_t expected_black_kings = 0x0000000000000000ULL;

        fenTest(fen, expectedBoard,
            expected_white_pawns,
            expected_white_knights,
            expected_white_bishops,
            expected_white_rooks,
            expected_white_queens,
            expected_white_kings,

            expected_black_pawns,
            expected_black_knights,
            expected_black_bishops,
            expected_black_rooks,
            expected_black_queens,
            expected_black_kings
        );
    }

    TEST(fen_unit_test, empty_fen_test)
    {
        auto fen = "8/8/8/8/8/8/8/8 w - - 0 1";
        auto expectedBoard = std::string(
            "8 . . . . . . . . \n"
            "7 . . . . . . . . \n"
            "6 . . . . . . . . \n"
            "5 . . . . . . . . \n"
            "4 . . . . . . . . \n"
            "3 . . . . . . . . \n"
            "2 . . . . . . . . \n"
            "1 . . . . . . . . \n"
            "  a b c d e f g h\n"
        );
        uint64_t expected_white_pawns = 0x0000000000000000ULL;
        uint64_t expected_black_pawns = 0x00000000000000000ULL;
        uint64_t expected_white_knights = 0x0000000000000000ULL;
        uint64_t expected_black_knights = 0x0000000000000000ULL;
        uint64_t expected_white_bishops = 0x0000000000000000ULL;
        uint64_t expected_black_bishops = 0x0000000000000000ULL;
        uint64_t expected_white_rooks = 0x0000000000000000ULL;
        uint64_t expected_black_rooks = 0x0000000000000000ULL;
        uint64_t expected_white_queens = 0x0000000000000000ULL;
        uint64_t expected_black_queens = 0x0000000000000000ULL;
        uint64_t expected_white_kings = 0x0000000000000000ULL;
        uint64_t expected_black_kings = 0x0000000000000000ULL;

        fenTest(fen, expectedBoard,
            expected_white_pawns,
            expected_white_knights,
            expected_white_bishops,
            expected_white_rooks,
            expected_white_queens,
            expected_white_kings,

            expected_black_pawns,
            expected_black_knights,
            expected_black_bishops,
            expected_black_rooks,
            expected_black_queens,
            expected_black_kings
        );
    }
}