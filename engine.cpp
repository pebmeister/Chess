// engine.cpp
#include <algorithm>
#include <limits>
#include <iostream>
#include <assert.h>
#include <future>
#include <vector>
#include <unordered_map>

#include "engine.h"
#include "chess.h"
#include "zobrist.h"

struct TTEntry {
    int depth;
    int64_t value;
    // Optionally: you can add a flag for exact/alpha/beta, and best move
};

Fen fen;
Zobrist zobrist;
thread_local std::unordered_map<uint64_t, TTEntry> transTable;

Move Engine::findBestMove(Board& board, int depth, std::vector<Move>& moves)
{
    moves = board.generateLegalMoves(board.getTurn());
    std::vector<std::future<int64_t>> futures;
    std::vector<Move> moveList = moves;

    orderMoves(board, moves);

    for (auto& move : moveList) {
        futures.push_back(std::async(std::launch::async, [&, move]()
            {
                Board next = board;
                next.makeMove(move);
                return minimax(next, depth - 1, std::numeric_limits<int>::min(),
                    std::numeric_limits<int>::max(), board.getTurn() == Color::Black);
            }));
    }

    auto bestValue = std::numeric_limits<int64_t>::min();
    int bestIndex = 0;
    for (size_t i = 0; i < futures.size(); ++i) {
        try {
            auto eval = futures[i].get();
            moveList[i].score = eval;
            if (eval > bestValue) {
                bestValue = eval;
                bestIndex = static_cast<int>(i);
            }
        }
        catch (const std::exception& ex) {
            std::cerr << "Exception in async move " << i << ": " << ex.what() << std::endl;
            moveList[i].score = std::numeric_limits<int>::min(); // or another sentinel value
        }
        catch (...) {
            std::cerr << "Unknown exception in async move " << i << std::endl;
            moveList[i].score = std::numeric_limits<int>::min();
        }
    }
    if (futures.size() == 0) {
        return Move();
    }
    return moveList[bestIndex];
}

// In engine.cpp or a suitable place
int64_t pieceValue(PieceType pt)
{
    switch (pt) {
        case PieceType::Pawn: return 100;
        case PieceType::Knight: return 320;
        case PieceType::Bishop: return 330;
        case PieceType::Rook: return 500;
        case PieceType::Queen: return 900;
        case PieceType::King: return 20000;
        default: return 0;
    }
}

void Engine::orderMoves(Board& board, std::vector<Move>& moves)
{
    for (auto& move : moves) {
        move.score = 0;

        // Score captures higher
        Piece captured = board.get(move.to.x, move.to.y);
        if (captured.type != PieceType::None) {
            move.score = pieceValue(captured.type) - pieceValue(board.get(move.from.x, move.from.y).type) / 10;
        }
        else if (move.type == MoveType::Promotion) {
            move.score += 800 + pieceValue(move.promotionType);
        }
    }
    std::sort(moves.begin(), moves.end(), [](const Move& a, const Move& b)
        {
            return a.score > b.score;
        });
}

// Example: White pawn PST (flip for black)
static const int pawnPST[8][8] = {
    { 0, 0, 0, 0, 0, 0, 0, 0 },
    { 5, 10, 10, -20, -20, 10, 10, 5 },
    { 5, -5, -10, 0, 0, -10, -5, 5 },
    { 0, 0, 0, 20, 20, 0, 0, 0 },
    { 5, 5, 10, 25, 25, 10, 5, 5 },
    { 10, 10, 20, 30, 30, 20, 10, 10 },
    { 50, 50, 50, 50, 50, 50, 50, 50 },
    { 0, 0, 0, 0, 0, 0, 0, 0 }
};



int64_t Engine::evaluate(const Board& board)
{
    int64_t score = 0;

    // Center squares: d4, e4, d5, e5
    const int centerSquares[4][2] = { {3,3}, {3,4}, {4,3}, {4,4} };
    const int centerBonus = 50; // Tune as desired

    for (int y = 0; y < 8; ++y) {
        for (int x = 0; x < 8; ++x) {
            Piece p = board.get(x, y);
            if (p.type == PieceType::None) continue;

            auto value = pieceValue(p.type);
            auto pieceScore = (p.color == board.turn) ? value : -value;

            Square sq{ x, y };
            Color pieceColor = p.color;
            Color opponentColor = board.opposite(pieceColor);

            // PSTs
            if (p.type == PieceType::Pawn) {
                int pstY = (p.color == Color::White) ? y : 7 - y;
                int pstX = x;
                pieceScore += pawnPST[pstY][pstX];
            }

            // Pawn structure
            // Example: Penalty for doubled pawns
            for (int file = 0; file < 8; ++file) {
                int whitePawns = 0, blackPawns = 0;
                for (int rank = 0; rank < 8; ++rank) {
                    Piece p = board.get(file, rank);
                    if (p.type == PieceType::Pawn) {
                        if (p.color == Color::White) whitePawns++;
                        else blackPawns++;
                    }
                }
                if (whitePawns > 1) score -= 20 * (whitePawns - 1);
                if (blackPawns > 1) score += 20 * (blackPawns - 1);
            }

            // King safety
            // Example: Bonus for castled king
            if (board.whiteKingside || board.whiteQueenside)
                score += 300;
            if (board.blackKingside || board.blackQueenside)
                score -= 300;

            // Mobility
            Board bb = board;
            auto whiteMobility = bb.generateLegalMoves(Color::White).size();
            auto blackMobility = bb.generateLegalMoves(Color::Black).size();
            score += 3 * (whiteMobility - blackMobility);


            // 5. Bishop Pair Bonus
            int whiteBishops = 0, blackBishops = 0;
            for (int y = 0; y < 8; ++y)
                for (int x = 0; x < 8; ++x) {
                    Piece p = board.get(x, y);
                    if (p.type == PieceType::Bishop) {
                        if (p.color == Color::White) whiteBishops++;
                        else blackBishops++;
                    }
                }
            if (whiteBishops >= 2) score += 300;
            if (blackBishops >= 2) score -= 300;


            // Center control bonus for pawns and knights
            if (p.type == PieceType::Pawn || p.type == PieceType::Knight) {
                for (const auto& sq : centerSquares) {
                    if (x == sq[0] && y == sq[1]) {
                        pieceScore += (p.color == board.turn) ? centerBonus : -centerBonus;
                        break;
                    }
                }
            }

            // Square sq{ x, y };
            bool attacked = board.isSquareAttacked(sq, opponentColor);

            if (attacked) {
                bool defended = board.isSquareAttacked(sq, pieceColor);
                if (!defended) {
                    score =  -(value * 100);
                    return score;
                }
            }

            score += pieceScore;
        }
    }
    return score;
}

int64_t Engine::minimax(Board& board, int depth, int64_t alpha, int64_t beta, bool maximizingPlayer)
{
    // 1. Transposition Table Lookup
    uint64_t hash = board.zobristHash();
    auto it = transTable.find(hash);
    if (it != transTable.end() && it->second.depth >= depth) {
        return it->second.value;
    }

    // 2. Terminal Node or Depth Limit
    Color currentSide = maximizingPlayer ? board.getTurn() : board.opposite(board.getTurn());
    if (depth == 0 || board.isCheckmate(currentSide)) {
        auto eval = evaluate(board);
        // Store in TT
        transTable[hash] = { depth, eval };
        return eval;
    }

    // 3. Generate and Order Moves
    std::vector<Move> moves = board.generateLegalMoves(currentSide);
    orderMoves(board, moves);

    int64_t bestValue;
    if (maximizingPlayer) {
        bestValue = std::numeric_limits<int64_t>::min();
        for (const auto& move : moves) {
            Board next = board;
            next.makeMove(move);
            auto eval = minimax(next, depth - 1, alpha, beta, false);
            bestValue = std::max(bestValue, eval);
            alpha = std::max(alpha, eval);
            if (beta <= alpha)
                break; // Beta cutoff
        }
    }
    else {
        bestValue = std::numeric_limits<int64_t>::max();
        for (const auto& move : moves) {
            Board next = board;
            next.makeMove(move);
            int64_t eval = minimax(next, depth - 1, alpha, beta, true);
            bestValue = std::min(bestValue, eval);
            beta = std::min(beta, eval);
            if (beta <= alpha)
                break; // Alpha cutoff
        }
    }

    // 4. Store in Transposition Table
    transTable[hash] = { depth, bestValue };
    return bestValue;
}
