#pragma once
#include <string>

class Board;

enum class MoveType {
    Normal,
    Promotion,
    EnPassant,
    Castle,
    Capture,
};

struct Move {
    inline char pieceTypeToCharLower(PieceType pt) const
    {
        switch (pt) {
            default:
            case PieceType::None:
                return '?';
            case PieceType::Pawn:
                return 'p';
            case PieceType::Knight:
                return 'n';
            case PieceType::Bishop:
                return 'b';
            case PieceType::Rook:
                return 'r';
            case PieceType::Queen:
                return 'q';
            case PieceType::King:
                return 'k';
        }
    }

public:
    Square from;
    Square to;
    MoveType type = MoveType::Normal;
    PieceType promotionType = PieceType::None;
    int64_t score = 0;

    Move(Square f, Square t, MoveType mt = MoveType::Normal, PieceType pt = PieceType::None)
        : from(f), to(t), type(mt), promotionType(pt)
    {
    }

    Move() : from({ -1, -1 }), to({ -1, -1 }) {}

    std::string toString() const;
    std::string toString(const Board& board) const;
};
