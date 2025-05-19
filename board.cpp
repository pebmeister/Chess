#include <bit>
#include <bitset>
#include <algorithm>
#include <assert.h>

#include "bitboard.h"
#include "board.h"
#include "fen.h"
#include "chess.h"

extern Fen fen;

inline static uint64_t knightAttacks(uint64_t knights)
{
    uint64_t l1 = (knights >> 1) & 0x7f7f7f7f7f7f7f7fULL;
    uint64_t l2 = (knights >> 2) & 0x3f3f3f3f3f3f3f3fULL;
    uint64_t r1 = (knights << 1) & 0xfefefefefefefefeULL;
    uint64_t r2 = (knights << 2) & 0xfcfcfcfcfcfcfcfcULL;

    uint64_t h1 = l1 | r1;
    uint64_t h2 = l2 | r2;

    return (h1 << 16) | (h1 >> 16) | (h2 << 8) | (h2 >> 8);
}

const int rookDirections[] = { 8, -8, 1, -1 };
const int bishopDirections[] = { 9, 7, -7, -9 };
const int queenDirections[] = { 8, -8, 1, -1, 9, 7, -7, -9 };

Board::Board()
{
    reset();
}

std::string Board::toString() const
{
    std::ostringstream oss;
    for (int y = 7; y >= 0; --y) {
        oss << (y + 1) << " ";
        for (int x = 0; x < 8; ++x) {
            auto piece = get(x, y);  // assumes (x, y) is (file, rank)
            oss << piece.toString() << ' ';
        }
        oss << '\n';
    }
    oss << "  a b c d e f g h\n";
    return oss.str();
}



Board::Board(std::string fen)
{
    moveHistory.clear();
    whiteKingside = true;
    whiteQueenside = true;
    blackKingside = true;
    blackQueenside = true;
    enPassantTarget = Square{ -1, -1 };
    halfMoveClock = 0;
    fullMoveNumber = 1;
    turn = Color::White;
    loadFEN(fen);
}

void Board::reset()
{
    moveHistory.clear();
    whiteKingside = true;
    whiteQueenside = true;
    blackKingside = true;
    blackQueenside = true;
    enPassantTarget = Square{ -1, -1 };
    halfMoveClock = 0;
    fullMoveNumber = 1;
    turn = Color::White;
    loadFEN("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
}

bool Board::isInside(int x, int y) const
{
    return x >= 0 && x < 8 && y >= 0 && y < 8;
}

Piece Board::get(int x, int y) const
{
    uint64_t mask = 1ULL << (y * 8 + x);
    Color color = Color::White;
    PieceType piecetype = PieceType::None;

    if (blackPieces & mask) {
        color = Color::Black;
    }
    else {
        color = Color::White;
    }
    if ((white_pawns | black_pawns) & mask) {
        piecetype = PieceType::Pawn;
    }
    else if ((white_rooks | black_rooks) & mask) {
        piecetype = PieceType::Rook;
    }
    else if ((white_knights | black_knights) & mask) {
        piecetype = PieceType::Knight;
    }
    else if ((white_bishops | black_bishops) & mask) {
        piecetype = PieceType::Bishop;
    }
    else if ((white_queens | black_queens) & mask) {
        piecetype = PieceType::Queen;
    }
    else if ((white_kings | black_kings) & mask) {
        piecetype = PieceType::King;
    }
    else {
        piecetype = PieceType::None;
    }    
    return Piece{ piecetype, color };
}

void Board::makeMove(const Move& move)
{
    // Save current state for undo
    BoardState state;
    state.move = move;
    state.whiteKingside = whiteKingside;
    state.whiteQueenside = whiteQueenside;
    state.blackKingside = blackKingside;
    state.blackQueenside = blackQueenside;
    state.enPassantTarget = enPassantTarget;
    state.halfMoveClock = halfMoveClock;
    state.fullMoveNumber = fullMoveNumber;

    // Clear en passant target
    enPassantTarget = Square{ -1, -1 };

    int fromIndex = move.from.y * 8 + move.from.x;
    int toIndex = move.to.y * 8 + move.to.x;
    uint64_t fromBB = 1ULL << fromIndex;
    uint64_t toBB = 1ULL << toIndex;

    // Handle captures (including en passant)
    if (move.type == MoveType::EnPassant) {
        // For en passant, the captured pawn is in a different square
        int capturedPawnIndex = (turn == Color::White) ? toIndex - 8 : toIndex + 8;
        uint64_t capturedBB = 1ULL << capturedPawnIndex;

        if (turn == Color::White) {
            black_pawns &= ~capturedBB;
        }
        else {
            white_pawns &= ~capturedBB;
        }
        state.captured = Piece{ PieceType::Pawn, opposite(turn) };
    }
    else {
        // Regular capture
        state.captured = get(move.to.x, move.to.y);
        if (state.captured.type != PieceType::None) {
            uint64_t& pieceBB = getPieceBB(state.captured.type, state.captured.color);
            pieceBB &= ~toBB;
        }
    }

    // Handle castling
    if (move.type == MoveType::Castle) {
        // Move the rook
        if (move.to.x == 6) { // Kingside
            uint64_t rookFromBB = turn == Color::White ? 0x80 : 0x8000000000000000;
            uint64_t rookToBB = turn == Color::White ? 0x20 : 0x2000000000000000;

            uint64_t& rooks = turn == Color::White ? white_rooks : black_rooks;
            rooks &= ~rookFromBB;
            rooks |= rookToBB;
        }
        else { // Queenside
            uint64_t rookFromBB = turn == Color::White ? 0x1 : 0x100000000000000;
            uint64_t rookToBB = turn == Color::White ? 0x8 : 0x800000000000000;

            uint64_t& rooks = turn == Color::White ? white_rooks : black_rooks;
            rooks &= ~rookFromBB;
            rooks |= rookToBB;
        }
    }

    // Move the piece
    Piece movedPiece = get(move.from.x, move.from.y);
    
    if (movedPiece.type == PieceType::None) {            
        throw new std::runtime_error("attempt to move a blank square.");
    }

    uint64_t& pieceBB = getPieceBB(movedPiece.type, movedPiece.color);
    pieceBB &= ~fromBB;

    // Handle promotion
    if (move.type == MoveType::Promotion) {
        PieceType promotedType = move.promotionType;
        uint64_t& promotedBB = getPieceBB(promotedType, movedPiece.color);
        promotedBB |= toBB;
    }
    else {
        pieceBB |= toBB;
    }

    // Update castling rights if rook or king moves
    if (movedPiece.type == PieceType::King) {
        if (turn == Color::White) {
            whiteKingside = false;
            whiteQueenside = false;
        }
        else {
            blackKingside = false;
            blackQueenside = false;
        }
    }
    else if (movedPiece.type == PieceType::Rook) {
        if (turn == Color::White) {
            if (move.from.x == 0 && move.from.y == 0) whiteQueenside = false;
            if (move.from.x == 7 && move.from.y == 0) whiteKingside = false;
        }
        else {
            if (move.from.x == 0 && move.from.y == 7) blackQueenside = false;
            if (move.from.x == 7 && move.from.y == 7) blackKingside = false;
        }
    }

    // Set en passant target for double pawn push
    if (movedPiece.type == PieceType::Pawn && abs(move.to.y - move.from.y) == 2) {
        enPassantTarget = Square{ move.from.x, (move.from.y + move.to.y) / 2 };
    }

    // Update move clocks
    if (movedPiece.type == PieceType::Pawn || state.captured.type != PieceType::None) {
        halfMoveClock = 0;
    }
    else {
        halfMoveClock++;
    }

    if (turn == Color::Black) {
        fullMoveNumber++;
    }

    // Update aggregate bitboards
    updateAggregateBitboards();

    // Switch turns
    turn = opposite(turn);

    // Save state for undo
    moveHistory.push_back(state);
}

void Board::undoMove()
{
    if (moveHistory.empty()) 
        return;

    const BoardState& state = moveHistory.back();

    // Switch turns back
    turn = opposite(turn);

    int fromIndex = state.move.from.y * 8 + state.move.from.x;
    int toIndex = state.move.to.y * 8 + state.move.to.x;
    uint64_t fromBB = 1ULL << fromIndex;
    uint64_t toBB = 1ULL << toIndex;

    // Undo the piece movement
    Piece movedPiece = get(state.move.to.x, state.move.to.y); // Get the piece at destination

    // Handle promotion undo
    if (state.move.type == MoveType::Promotion) {
        uint64_t& promotedBB = getPieceBB(state.move.promotionType, movedPiece.color);
        promotedBB &= ~toBB;

        // Restore pawn
        uint64_t& pawnBB = getPieceBB(PieceType::Pawn, movedPiece.color);
        pawnBB |= fromBB;
    }
    else {
        uint64_t& pieceBB = getPieceBB(movedPiece.type, movedPiece.color);
        pieceBB &= ~toBB;
        pieceBB |= fromBB;
    }

    // Handle castling undo
    if (state.move.type == MoveType::Castle) {
        // Move the rook back
        if (state.move.to.x == 6) { // Kingside
            uint64_t rookFromBB = turn == Color::White ? 0x20 : 0x2000000000000000;
            uint64_t rookToBB = turn == Color::White ? 0x80 : 0x8000000000000000;

            uint64_t& rooks = turn == Color::White ? white_rooks : black_rooks;
            rooks &= ~rookFromBB;
            rooks |= rookToBB;
        }
        else { // Queenside
            uint64_t rookFromBB = turn == Color::White ? 0x8 : 0x800000000000000;
            uint64_t rookToBB = turn == Color::White ? 0x1 : 0x100000000000000;

            uint64_t& rooks = turn == Color::White ? white_rooks : black_rooks;
            rooks &= ~rookFromBB;
            rooks |= rookToBB;
        }
    }

    // Restore captured piece
    if (state.captured.type != PieceType::None) {
        if (state.move.type == MoveType::EnPassant) {
            // For en passant, the captured pawn goes to a different square
            int capturedIndex = (turn == Color::White) ? toIndex - 8 : toIndex + 8;
            uint64_t capturedBB = 1ULL << capturedIndex;
            uint64_t& pieceBB = getPieceBB(state.captured.type, state.captured.color);
            pieceBB |= capturedBB;
        }
        else {
            uint64_t& pieceBB = getPieceBB(state.captured.type, state.captured.color);
            pieceBB |= toBB;
        }
    }

    // Restore game state
    whiteKingside = state.whiteKingside;
    whiteQueenside = state.whiteQueenside;
    blackKingside = state.blackKingside;
    blackQueenside = state.blackQueenside;
    enPassantTarget = state.enPassantTarget;
    halfMoveClock = state.halfMoveClock;
    fullMoveNumber = state.fullMoveNumber;

    // Update aggregate bitboards
    updateAggregateBitboards();

    // Remove from history
    moveHistory.pop_back();
}

uint64_t& Board::getPieceBB(PieceType type, Color color)
{
    if (color == Color::White) {
        switch (type) {
            case PieceType::Pawn: return white_pawns;
            case PieceType::Knight: return white_knights;
            case PieceType::Bishop: return white_bishops;
            case PieceType::Rook: return white_rooks;
            case PieceType::Queen: return white_queens;
            case PieceType::King: return white_kings;
            default: throw std::runtime_error("Invalid piece type");
        }
    }
    else {
        switch (type) {
            case PieceType::Pawn: return black_pawns;
            case PieceType::Knight: return black_knights;
            case PieceType::Bishop: return black_bishops;
            case PieceType::Rook: return black_rooks;
            case PieceType::Queen: return black_queens;
            case PieceType::King: return black_kings;
            default: throw std::runtime_error("Invalid piece type");
        }
    }
}

void Board::updateAggregateBitboards()
{
    whitePieces = white_pawns | white_knights | white_bishops |
        white_rooks | white_queens | white_kings;
    blackPieces = black_pawns | black_knights | black_bishops |
        black_rooks | black_queens | black_kings;
    allPieces = whitePieces | blackPieces;
}

void Board::loadFEN(std::string_view  fenstr)
{
    fen.load(fenstr);
        
    white_pawns = fen.white_pawns;
    white_rooks = fen.white_rooks;
    white_knights = fen.white_knights;
    white_bishops = fen.white_bishops;
    white_queens = fen.white_queens;
    white_kings = fen.white_kings;
    whitePieces = fen.whitePieces;

    black_pawns = fen.black_pawns;
    black_rooks = fen.black_rooks;
    black_knights = fen.black_knights;
    black_bishops = fen.black_bishops;
    black_queens = fen.black_queens;
    black_kings = fen.black_kings;
    blackPieces = fen.blackPieces;
    allPieces = fen.allPieces;

    turn = fen.turn;
}

bool Board::isSquareAttacked(Square sq, Color bySide) const
{
    auto theirMoves = generatePseudoLegalMoves(bySide);
    for (const auto& m : theirMoves) {
        if (m.to.x == sq.x && m.to.y == sq.y)
            return true;
    }
    return false;
}

bool Board::isInCheck(Color side) const
{
    // Locate the king
    for (int y = 0; y < 8; ++y) {
        for (int x = 0; x < 8; ++x) {
            Piece p = get(x, y);
            if (p.type == PieceType::King && p.color == side)
                return isSquareAttacked({ x, y }, opposite(side));
        }
    }
    return false;
}

bool Board::isCheckmate(Color side)
{
    if (!isInCheck(side)) return false;
    auto moves = generateLegalMoves(side);
    return moves.empty();
}

std::vector<Move> Board::generateLegalMoves(Color side)
{
    std::vector<Move> legalMoves;
    auto pseudoMoves = generatePseudoLegalMoves(side);

    auto temp = *this;
    for (auto& m : pseudoMoves) {
        makeMove(m);
        if (!isInCheck(side))
            legalMoves.push_back(m);
        undoMove();
        assert(*this == temp);
    }
    return legalMoves;
}

std::vector<Move> Board::generateKingMoves(Color side) const
{
    std::vector<Move> moves;

    auto indexToSquare = [](int index) -> Square
        {
            return Square{ index % 8, index / 8 };
        };

    uint64_t kingBB = (side == Color::White) ? white_kings : black_kings;
    int kingIndex = std::countr_zero(kingBB);
    uint64_t ownPieces = (side == Color::White) ? whitePieces : blackPieces;

    // Normal king moves
    uint64_t targets = kingAttacks(kingBB) & ~ownPieces;

    for (uint64_t bb = targets; bb; bb &= bb - 1) {
        int toIndex = std::countr_zero(bb);
        moves.emplace_back(indexToSquare(kingIndex), indexToSquare(toIndex));
    }

    // Castling
    if (side == Color::White) {
        if (whiteKingside) {
            if ((allPieces & 0x0000000000000060ULL) == 0 && !isSquareAttacked({ 4, 0 }, Color::Black) &&
                !isSquareAttacked({ 5, 0 }, Color::Black) && !isSquareAttacked({ 6, 0 }, Color::Black)) {
                moves.emplace_back(Square{ 4, 0 }, Square{ 6, 0 }, MoveType::Castle);
            }
        }
        if (whiteQueenside) {
            if ((allPieces & 0x000000000000000EULL) == 0 && !isSquareAttacked({ 4, 0 }, Color::Black) &&
                !isSquareAttacked({ 3, 0 }, Color::Black) && !isSquareAttacked({ 2, 0 }, Color::Black)) {
                moves.emplace_back(Square{ 4, 0 }, Square{ 2, 0 }, MoveType::Castle);
            }
        }
    }
    else {
        if (blackKingside) {
            if ((allPieces & 0x6000000000000000ULL) == 0 && !isSquareAttacked({ 4, 7 }, Color::White) &&
                !isSquareAttacked({ 5, 7 }, Color::White) && !isSquareAttacked({ 6, 7 }, Color::White)) {
                moves.emplace_back(Square{ 4, 7 }, Square{ 6, 7 }, MoveType::Castle);
            }
        }
        if (blackQueenside) {
            if ((allPieces & 0x0E00000000000000ULL) == 0 && !isSquareAttacked({ 4, 7 }, Color::White) &&
                !isSquareAttacked({ 3, 7 }, Color::White) && !isSquareAttacked({ 2, 7 }, Color::White)) {
                moves.emplace_back(Square{ 4, 7 }, Square{ 2, 7 }, MoveType::Castle);
            }
        }
    }

    return moves;
}

uint64_t Board::kingAttacks(uint64_t kingBB) const
{
    const uint64_t notAFile = 0xfefefefefefefefeULL;
    const uint64_t notHFile = 0x7f7f7f7f7f7f7f7fULL;

    uint64_t attacks = 0;

    attacks |= (kingBB << 8);                    // North
    attacks |= (kingBB >> 8);                    // South
    attacks |= (kingBB << 1) & notAFile;         // East
    attacks |= (kingBB >> 1) & notHFile;         // West
    attacks |= (kingBB << 9) & notAFile;         // NorthEast
    attacks |= (kingBB << 7) & notHFile;         // NorthWest
    attacks |= (kingBB >> 7) & notAFile;         // SouthEast
    attacks |= (kingBB >> 9) & notHFile;         // SouthWest

    return attacks;
}

std::vector<Move> Board::generateSlidingMoves(Color side, uint64_t pieces, const int* directions, int dirCount) const
{
    std::vector<Move> moves;
    uint64_t ownPieces = (side == Color::White) ? whitePieces : blackPieces;
    uint64_t opponentPieces = (side == Color::White) ? blackPieces : whitePieces;

    auto indexToSquare = [](int index) -> Square
        {
            return Square{ index % 8, index / 8 };
        };

    auto onBoard = [](int from, int to, int delta)
        {
            if (to < 0 || to >= 64) return false;
            int fx = from % 8, tx = to % 8;

            bool fromFileH = (from % 8 == 7);
            bool fromFileA = (from % 8 == 0);

            // Disallow wrap across ranks
            if ((delta == 1 || delta == 9 || delta == -7) && fromFileH) return false;
            if ((delta == -1 || delta == -9 || delta == 7) && fromFileA) return false;

            return true;
        };

    for (uint64_t bb = pieces; bb; bb &= bb - 1) {
        int from = std::countr_zero(bb);
        for (int i = 0; i < dirCount; ++i) {
            int delta = directions[i];
            int to = from + delta;

            int square = from + delta;
            while (onBoard(from, square, delta)) {
                uint64_t toBB = 1ULL << square;
                if (ownPieces & toBB) break;

                moves.emplace_back(indexToSquare(from), indexToSquare(square));
                if (opponentPieces & toBB) break;

                square += delta;
            }
        }
    }

    return moves;
}

std::vector<Move> Board::generateRookMoves(Color side) const
{
    uint64_t rooks = (side == Color::White) ? white_rooks : black_rooks;
    return generateSlidingMoves(side, rooks, rookDirections, 4);
}

std::vector<Move> Board::generateBishopMoves(Color side) const
{
    uint64_t bishops = (side == Color::White) ? white_bishops : black_bishops;
    return generateSlidingMoves(side, bishops, bishopDirections, 4);
}

std::vector<Move> Board::generateQueenMoves(Color side) const
{
    uint64_t queens = (side == Color::White) ? white_queens : black_queens;
    return generateSlidingMoves(side, queens, queenDirections, 8);
}

std::vector<Move> Board::generateKnightMoves(Color side) const
{
    if (side == Color::White)
        return generateWhiteKnightMoves();
    return generateBlackKnightMoves();
}

std::vector<Move> Board::generateWhiteKnightMoves() const
{
    std::vector<Move> moves;

    auto indexToSquare = [](int index) -> Square
        {
            return Square{ index % 8, index / 8 };
        };

    for (uint64_t knights = white_knights; knights; knights &= knights - 1) {
        int fromIndex = std::countr_zero(knights);
        uint64_t fromBB = 1ULL << fromIndex;

        uint64_t targets = knightAttacks(fromBB) & ~whitePieces;

        for (uint64_t bb = targets; bb; bb &= bb - 1) {
            int toIndex = std::countr_zero(bb);
            moves.emplace_back(indexToSquare(fromIndex), indexToSquare(toIndex));
        }
    }

    return moves;
}

std::vector<Move> Board::generateBlackKnightMoves() const
{
    std::vector<Move> moves;

    auto indexToSquare = [](int index) -> Square
        {
            return Square{ index % 8, index / 8 };
        };

    for (uint64_t knights = black_knights; knights; knights &= knights - 1) {
        int fromIndex = std::countr_zero(knights);
        uint64_t fromBB = 1ULL << fromIndex;

        uint64_t targets = knightAttacks(fromBB) & ~blackPieces;

        for (uint64_t bb = targets; bb; bb &= bb - 1) {
            int toIndex = std::countr_zero(bb);
            moves.emplace_back(indexToSquare(fromIndex), indexToSquare(toIndex));
        }
    }

    return moves;
}

std::vector<Move> Board::generatePawnMoves(Color side) const
{
    auto indexToSquare = [](int index) -> Square
        {
            return Square{ index % 8, index / 8 };
        };

    std::vector<Move> moves;

    // === Side-dependent constants ===
    int dir = (side == Color::White) ? 8 : -8;
    int dblDir = dir * 2;

    uint64_t pawns = (side == Color::White) ? white_pawns : black_pawns;
    uint64_t opponentPieces = (side == Color::White) ? blackPieces : whitePieces;
    uint64_t startRank = (side == Color::White) ? RANK_2 : RANK_7;
    uint64_t promoRank = (side == Color::White) ? RANK_8 : RANK_1;

    uint64_t empty = ~allPieces;

    // === Push moves ===
    uint64_t singlePush = (dir > 0) ? (pawns << dir) : (pawns >> -dir);
    singlePush &= empty;

    uint64_t doublePush = (pawns & startRank);
    doublePush = (dir > 0) ? (doublePush << dir) : (doublePush >> -dir);
    doublePush &= empty;
    doublePush = (dir > 0) ? (doublePush << dir) : (doublePush >> -dir);
    doublePush &= empty;

    // === Captures ===
    uint64_t leftMask = (side == Color::White) ? ~FILE_H : ~FILE_A;
    uint64_t rightMask = (side == Color::White) ? ~FILE_A : ~FILE_H;

    int leftOffset = (dir + ((side == Color::White) ? -1 : 1));
    int rightOffset = (dir + ((side == Color::White) ? 1 : -1));

    uint64_t leftCapture = (dir > 0) ? (pawns << leftOffset) : (pawns >> -leftOffset);
    leftCapture &= opponentPieces & leftMask;

    uint64_t rightCapture = (dir > 0) ? (pawns << rightOffset) : (pawns >> -rightOffset);
    rightCapture &= opponentPieces & rightMask;

    // === En Passant ===
    uint64_t ep = 0;
    if (enPassantTarget.x >= 0 && enPassantTarget.y >= 0) {
        ep = 1ULL << (enPassantTarget.y * 8 + enPassantTarget.x);
    }

    uint64_t epLeft = (dir > 0) ? (pawns << leftOffset) : (pawns >> -leftOffset);
    epLeft &= ep & leftMask;

    uint64_t epRight = (dir > 0) ? (pawns << rightOffset) : (pawns >> -rightOffset);
    epRight &= ep & rightMask;

    // === Single Pushes ===
    for (uint64_t bb = singlePush; bb; bb &= bb - 1) {
        int to = std::countr_zero(bb);
        int from = to - dir;
        Square fromSq = indexToSquare(from);
        Square toSq = indexToSquare(to);

        if ((1ULL << to) & promoRank) {
            moves.emplace_back(fromSq, toSq, MoveType::Promotion, PieceType::Queen);
        }
        else {
            moves.emplace_back(fromSq, toSq);
        }
    }

    // === Double Pushes ===
    for (uint64_t bb = doublePush; bb; bb &= bb - 1) {
        int to = std::countr_zero(bb);
        int from = to - dblDir;
        moves.emplace_back(indexToSquare(from), indexToSquare(to));
    }

    // === Captures ===
    auto handleCaptures = [&](uint64_t bb, int offset)
        {
            for (; bb; bb &= bb - 1) {
                int to = std::countr_zero(bb);
                int from = to - offset;
                Square fromSq = indexToSquare(from);
                Square toSq = indexToSquare(to);

                if ((1ULL << to) & promoRank) {
                    moves.emplace_back(fromSq, toSq, MoveType::Promotion, PieceType::Queen);
                }
                else {
                    moves.emplace_back(fromSq, toSq);
                }
            }
        };

    handleCaptures(leftCapture, leftOffset);
    handleCaptures(rightCapture, rightOffset);

    // === En Passant ===
    for (uint64_t bb = epLeft; bb; bb &= bb - 1) {
        int to = std::countr_zero(bb);
        int from = to - leftOffset;
        moves.emplace_back(indexToSquare(from), indexToSquare(to));
    }
    for (uint64_t bb = epRight; bb; bb &= bb - 1) {
        int to = std::countr_zero(bb);
        int from = to - rightOffset;
        moves.emplace_back(indexToSquare(from), indexToSquare(to));
    }

    return moves;
}

//std::vector<Move> Board::generatePawnMoves(Color side) const
//{
//    if (side == Color::White)
//        return generateWhitePawnMoves();
//    return generateBlackPawnMoves();
//}
//
std::vector<Move> Board::generateWhitePawnMoves() const
{
    auto indexToSquare = [](int index) -> Square
        {
            return Square{ index % 8, index / 8 };
        };

    std::vector<Move> moves;
    uint64_t empty = ~allPieces;

    uint64_t singlePush = (white_pawns << 8) & empty;
    uint64_t doublePush = ((white_pawns & RANK_2) << 8) & empty;
    doublePush = (doublePush << 8) & empty;

    uint64_t leftCapture = (white_pawns << 7) & blackPieces & ~FILE_H;
    uint64_t rightCapture = (white_pawns << 9) & blackPieces & ~FILE_A;

    // En passant targets
    uint64_t ep = 0;
    if (enPassantTarget.x >= 0 && enPassantTarget.y >= 0) {
        ep = 1ULL << (enPassantTarget.y * 8 + enPassantTarget.x);
    }

    uint64_t epLeft = (white_pawns << 7) & ep & ~FILE_H;
    uint64_t epRight = (white_pawns << 9) & ep & ~FILE_A;

    for (uint64_t bb = singlePush; bb; bb &= bb - 1) {
        int to = std::countr_zero(bb);
        int from = to - 8;
        Square fromSq = indexToSquare(from);
        Square toSq = indexToSquare(to);

        // Promotion
        if (toSq.y == 7) {
            moves.emplace_back(fromSq, toSq, MoveType::Promotion, PieceType::Queen);
        }
        else {
            moves.emplace_back(fromSq, toSq);
        }
    }

    for (uint64_t bb = doublePush; bb; bb &= bb - 1) {
        int to = std::countr_zero(bb);
        int from = to - 16;
        moves.emplace_back(indexToSquare(from), indexToSquare(to));
    }

    for (uint64_t bb = leftCapture; bb; bb &= bb - 1) {
        int to = std::countr_zero(bb);
        int from = to - 7;
        Square fromSq = indexToSquare(from);
        Square toSq = indexToSquare(to);

        if (toSq.y == 7) {
            // Promotion capture
            moves.emplace_back(fromSq, toSq, MoveType::Promotion, PieceType::Queen);
        }
        else {
            moves.emplace_back(fromSq, toSq);
        }
    }

    for (uint64_t bb = rightCapture; bb; bb &= bb - 1) {
        int to = std::countr_zero(bb);
        int from = to - 9;
        Square fromSq = indexToSquare(from);
        Square toSq = indexToSquare(to);

        if (toSq.y == 7) {
            // Promotion capture
            moves.emplace_back(fromSq, toSq, MoveType::Promotion, PieceType::Queen);
        }
        else {
            moves.emplace_back(fromSq, toSq);
        }
    }

    // En passant captures
    for (uint64_t bb = epLeft; bb; bb &= bb - 1) {
        int to = std::countr_zero(bb);
        int from = to - 7;
        moves.emplace_back(indexToSquare(from), indexToSquare(to));
    }
    for (uint64_t bb = epRight; bb; bb &= bb - 1) {
        int to = std::countr_zero(bb);
        int from = to - 9;
        moves.emplace_back(indexToSquare(from), indexToSquare(to));
    }

    return moves;
}

std::vector<Move> Board::generateBlackPawnMoves() const
{
    std::vector<Move> moves;
    uint64_t empty = ~allPieces;

    uint64_t singlePush = (black_pawns >> 8) & empty;
    uint64_t doublePush = ((black_pawns & RANK_7) >> 8) & empty;
    doublePush = (doublePush >> 8) & empty;

    uint64_t leftCapture = (black_pawns >> 9) & whitePieces & ~FILE_H;
    uint64_t rightCapture = (black_pawns >> 7) & whitePieces & ~FILE_A;

    // En passant targets
    uint64_t ep = 0;
    if (enPassantTarget.x >= 0 && enPassantTarget.y >= 0) {
        ep = 1ULL << (enPassantTarget.y * 8 + enPassantTarget.x);
    }

    uint64_t epLeft = (black_pawns >> 9) & ep & ~FILE_H;
    uint64_t epRight = (black_pawns >> 7) & ep & ~FILE_A;

    auto indexToSquare = [](int index) -> Square
        {
            return Square{ index % 8, index / 8 };
        };

    for (uint64_t bb = singlePush; bb; bb &= bb - 1) {
        int to = std::countr_zero(bb);
        int from = to + 8;
        Square fromSq = indexToSquare(from);
        Square toSq = indexToSquare(to);

        if (toSq.y == 0) {
            moves.emplace_back(fromSq, toSq, MoveType::Promotion, PieceType::Queen); // Promotion
        }
        else {
            moves.emplace_back(fromSq, toSq);
        }
    }

    for (uint64_t bb = doublePush; bb; bb &= bb - 1) {
        int to = std::countr_zero(bb);
        int from = to + 16;
        moves.emplace_back(indexToSquare(from), indexToSquare(to));
    }

    for (uint64_t bb = leftCapture; bb; bb &= bb - 1) {
        int to = std::countr_zero(bb);
        int from = to + 9;
        Square fromSq = indexToSquare(from);
        Square toSq = indexToSquare(to);

        if (toSq.y == 0) {
            moves.emplace_back(fromSq, toSq, MoveType::Promotion, PieceType::Queen); // Promotion capture
        }
        else {
            moves.emplace_back(fromSq, toSq);
        }
    }

    for (uint64_t bb = rightCapture; bb; bb &= bb - 1) {
        int to = std::countr_zero(bb);
        int from = to + 7;
        Square fromSq = indexToSquare(from);
        Square toSq = indexToSquare(to);

        if (toSq.y == 0) {
            moves.emplace_back(fromSq, toSq, MoveType::Promotion, PieceType::Queen); // Promotion capture
        }
        else {
            moves.emplace_back(fromSq, toSq);
        }
    }

    // En passant captures
    for (uint64_t bb = epLeft | epRight; bb; bb &= bb - 1) {
        int to = std::countr_zero(bb);
        int from = to + (bb & epLeft ? 9 : 7);
        moves.emplace_back(indexToSquare(from), indexToSquare(to));
    }

    return moves;
}

std::vector<Move> Board::generatePseudoLegalMoves(Color side) const
{
    std::vector<Move> moves;

    auto pawnMoves = generatePawnMoves(side);
    moves.insert(moves.end(), pawnMoves.begin(), pawnMoves.end());

    auto rookMoves = generateRookMoves(side);
    moves.insert(moves.end(), rookMoves.begin(), rookMoves.end());

    auto knightMoves = generateKnightMoves(side);
    moves.insert(moves.end(), knightMoves.begin(), knightMoves.end());

    auto bishopMoves = generateBishopMoves(side);
    moves.insert(moves.end(), bishopMoves.begin(), bishopMoves.end());

    auto queenMoves = generateQueenMoves(side);
    moves.insert(moves.end(), queenMoves.begin(), queenMoves.end());

//    auto kingMoves = generateKingMoves(side);
//    moves.insert(moves.end(), kingMoves.begin(), kingMoves.end());

    return moves;
}

Color Board::getTurn() const
{
    return turn;
}

Color Board::opposite(Color c) const
{
    return (c == Color::White) ? Color::Black : Color::White;
}
