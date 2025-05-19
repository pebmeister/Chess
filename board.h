#pragma once
#include <vector>
#include <iostream>
#include <string>
#include <array>
#include <stdint.h>

#include "chesstypes.h"

struct Square {
    int x, y;
    bool operator==(const Square& other) const { return x == other.x && y == other.y; }
    const std::string toString() const
    {
        auto rank = (char)((char)x + 'A');
        auto file = (char)((char)y + '1');
        auto s = std::string("") + rank;
        s += file;
        return s;
    }
};

enum class MoveType {
    Normal,
    Promotion,
    EnPassant,
    Castle,
    Capture,
};

struct Move {
public:
    Square from;
    Square to;
    MoveType type = MoveType::Normal;
    PieceType promotionType;
    int score = 0;

    Move(Square f, Square t, MoveType mt = MoveType::Normal, PieceType pt = PieceType::None)
        : from(f), to(t), type(mt), promotionType(pt)
    {
    }

    Move()
    {
        from = { -1, -1 };
        to = { -1, -1 };
        type = MoveType::Normal;
        promotionType = PieceType::None;
    }

    std::string toString() const
    {
        std::string str;
        str += (char)('a' + from.x);
        str += (char)('8' - from.y);
        str += type == MoveType::Capture || type == MoveType::EnPassant ? 'x' : '-';
        str += (char)('a' + to.x);
        str += (char)('8' - to.y);
        return str;
    }
};

class Board  {

public:
    bool operator== (Board other)
    {
        return
            castlingRights == other.castlingRights &&
            halfMoveClock == other.halfMoveClock &&
            fullMoveNumber == other.fullMoveNumber &&
            whiteKingside == other.whiteKingside &&
            whiteQueenside == other.whiteQueenside &&
            blackKingside == other.blackKingside &&
            blackQueenside == other.blackQueenside &&

            white_pawns == other.white_pawns &&
            white_knights == other.white_knights &&
            white_bishops == other.white_bishops &&
            white_rooks == other.white_rooks &&
            white_queens == other.white_queens &&
            white_kings == other.white_kings &&

            black_pawns == other.black_pawns &&
            black_knights == other.black_knights &&
            black_bishops == other.black_bishops &&
            black_rooks == other.black_rooks &&
            black_queens == other.black_queens &&
            black_kings == other.black_kings &&

            whitePieces == other.whitePieces &&
            blackPieces == other.blackPieces &&
            allPieces == other.allPieces;
    }

    // this is second one '!='
    bool operator!= (Board other)
    {
        return !(*this == other);
    }
    std::string toString() const;

    
private:

    struct BoardState {
        Move move;
        Piece captured;
        bool whiteKingside;
        bool whiteQueenside;
        bool blackKingside;
        bool blackQueenside;
        Square enPassantTarget;
        int halfMoveClock;
        int fullMoveNumber;
    };

    std::vector<Move> generatePawnMoves(Color side) const;
    std::vector<Move> generateWhitePawnMoves() const;
    std::vector<Move> generateBlackPawnMoves() const;
    std::vector<Move> generateKnightMoves(Color side) const;
    std::vector<Move> generateWhiteKnightMoves() const;
    std::vector<Move> generateBlackKnightMoves() const;
    std::vector<Move> generateRookMoves(Color side) const;
    std::vector<Move> generateBishopMoves(Color side) const;
    std::vector<Move> generateQueenMoves(Color side) const;
    std::vector<Move> generateKingMoves(Color side) const;

    std::vector<Move> generateSlidingMoves(Color side, uint64_t pieces, const int* directions, int dirCount) const;
    void updateAggregateBitboards();
    uint64_t& getPieceBB(PieceType type, Color color);

public:
    Board();
    Board(std::string fen);
    void reset();

    Color getTurn() const;
    Piece get(int x, int y) const;
    void makeMove(const Move& m);
    void undoMove();
    void loadFEN(std::string_view);
    bool isSquareAttacked(Square sq, Color bySide) const;
    bool isInCheck(Color side) const;
    bool isCheckmate(Color side);
    std::vector<Move> generateLegalMoves(Color side);

    Color turn;
    Square enPassantTarget;
    uint8_t castlingRights;
    int halfMoveClock = 0;
    int fullMoveNumber = 0;
    bool whiteKingside = false;
    bool whiteQueenside = false;
    bool blackKingside = false;
    bool blackQueenside = false;

    uint64_t white_pawns;
    uint64_t white_knights;
    uint64_t white_bishops;
    uint64_t white_rooks;
    uint64_t white_queens;
    uint64_t white_kings;

    uint64_t black_pawns;
    uint64_t black_knights;
    uint64_t black_bishops;
    uint64_t black_rooks;
    uint64_t black_queens;
    uint64_t black_kings;

    uint64_t allPieces;            // all pieces on the board
    uint64_t whitePieces;
    uint64_t blackPieces;

    std::array<uint64_t, 6> whitePieceType; // [Pawn, Knight, Bishop, Rook, Queen, King]
    std::array<uint64_t, 6> blackPieceType; // same

    Color opposite(Color c) const;


private:
    std::vector<BoardState> moveHistory;

    bool isInside(int x, int y) const;
    std::vector<Move> generatePseudoLegalMoves(Color side) const;
    uint64_t kingAttacks(uint64_t kingBB) const;
};
