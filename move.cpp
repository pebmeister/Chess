#include "chess.h"
#include "move.h"
#include "board.h"

std::string Move::toString() const
{
    std::string str;
    str += from.toString();
    str += (type == MoveType::Capture || type == MoveType::EnPassant) ? 'x' : '-';
    str += to.toString();

    if (type == MoveType::Promotion && promotionType != PieceType::None) {
        str += '=';
        auto p = pieceTypeToCharLower(promotionType);
        str += p; // You’ll need to implement this
    }

    return str;
}

std::string Move::toString(const Board& board) const
{
    std::string str;
    str += from.toString();;
    str += (type == MoveType::Capture || type == MoveType::EnPassant ? 'x' : '-');
    str += to.toString();

    if (type == MoveType::Promotion) {
        auto piece = board.get(from.x, from.y);
        str += '=';
        str += piece.toString(); // assuming this returns correct letter case
    }

    return str;
}
