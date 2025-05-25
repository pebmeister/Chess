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
    int value;
    // Optionally: you can add a flag for exact/alpha/beta, and best move
};

Fen fen;
Zobrist zobrist;
thread_local std::unordered_map<uint64_t, TTEntry> transTable;

Move Engine::findBestMove(Board& board, int depth, std::vector<Move>& moves)
{
    moves = board.generateLegalMoves(board.getTurn());
    std::vector<std::future<int>> futures;
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

    int bestValue = std::numeric_limits<int>::min();
    int bestIndex = 0;
    for (size_t i = 0; i < futures.size(); ++i) {
        try {
            int eval = futures[i].get();
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
    return moveList[bestIndex];
}

// In engine.cpp or a suitable place
int pieceValue(PieceType pt)
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
        // Score captures higher
        Piece captured = board.get(move.to.x, move.to.y);
        if (captured.type != PieceType::None) {
            move.score = pieceValue(captured.type) - pieceValue(board.get(move.from.x, move.from.y).type);
        }
        else if (move.type == MoveType::Promotion) {
            move.score = 800 + pieceValue(move.promotionType);
        }
        else {
            move.score = 0;
        }
    }
    std::sort(moves.begin(), moves.end(), [](const Move& a, const Move& b)
        {
            return a.score > b.score;
        });
}

int Engine::evaluate(const Board& board)
{
    int score = 0;
    static const int pieceValues[] = { 0, 100, 320, 330, 500, 900, 20000 };

    // Center squares: d4, e4, d5, e5
    const int centerSquares[4][2] = { {3,3}, {3,4}, {4,3}, {4,4} };
    const int centerBonus = 20; // Tune as desired

    for (int y = 0; y < 8; ++y) {
        for (int x = 0; x < 8; ++x) {
            Piece p = board.get(x, y);
            if (p.type == PieceType::None) continue;

            int value = pieceValues[static_cast<int>(p.type)];
            int pieceScore = (p.color == Color::White) ? value : -value;

            // Center control bonus for pawns and knights
            if (p.type == PieceType::Pawn || p.type == PieceType::Knight) {
                for (const auto& sq : centerSquares) {
                    if (x == sq[0] && y == sq[1]) {
                        pieceScore += (p.color == Color::White) ? centerBonus : -centerBonus;
                        break;
                    }
                }
            }

            score += pieceScore;
        }
    }
    return score;
}

int Engine::minimax(Board& board, int depth, int alpha, int beta, bool maximizingPlayer)
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
        int eval = evaluate(board);
        // Store in TT
        transTable[hash] = { depth, eval };
        return eval;
    }

    // 3. Generate and Order Moves
    std::vector<Move> moves = board.generateLegalMoves(currentSide);
    orderMoves(board, moves);

    int bestValue;
    if (maximizingPlayer) {
        bestValue = std::numeric_limits<int>::min();
        for (const auto& move : moves) {
            Board next = board;
            next.makeMove(move);
            int eval = minimax(next, depth - 1, alpha, beta, false);
            bestValue = std::max(bestValue, eval);
            alpha = std::max(alpha, eval);
            if (beta <= alpha)
                break; // Beta cutoff
        }
    }
    else {
        bestValue = std::numeric_limits<int>::max();
        for (const auto& move : moves) {
            Board next = board;
            next.makeMove(move);
            int eval = minimax(next, depth - 1, alpha, beta, true);
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

