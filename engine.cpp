// engine.cpp
#include <limits>
#include <iostream>
#include <assert.h>
#include "engine.h"
#include "chess.h"

Fen fen;

Move Engine::findBestMove(Board& board, int depth, std::vector<Move>& moves)
{
    std::list<Move> bestMove;
    int bestValue = std::numeric_limits<int>::min();

    Color currentTurn = board.getTurn(); // Save current turn
    moves = board.generateLegalMoves(currentTurn);

    for (auto& move : moves) {
        auto next = board;
        next.makeMove(move);

        // Pass the opposite color for the next level since we just made a move
        int eval = minimax(next, depth - 1, std::numeric_limits<int>::min(),
            std::numeric_limits<int>::max(), currentTurn == Color::Black);

        move.score = eval;
        if (eval == bestValue) {
            bestMove.emplace_back(move);
        }
        else if (eval > bestValue) {
            bestValue = eval;
            bestMove.clear();
            bestMove.emplace_back(move);
        }
    }

    return bestMove.front();
}

int Engine::evaluate(const Board& board)
{
    int score = 0;
    static const int pieceValues[] = { 0, 100, 320, 330, 500, 900, 20000 }; // None, Pawn, Knight, Bishop, Rook, Queen, King

    for (int y = 0; y < 8; ++y) {
        for (int x = 0; x < 8; ++x) {
            
            Piece p = board.get(x, y);            
            if (p.type == PieceType::None) continue;
  
            int value = pieceValues[static_cast<int>(p.type)];
            score += (p.color == Color::White) ? value : -value;
        }
    }
    return score;
}

int Engine::minimax(Board& board, int depth, int alpha, int beta, bool maximizingPlayer)
{
    Color currentSide = maximizingPlayer ? board.getTurn() : board.opposite(board.getTurn());
   
    if (depth == 0 || board.isCheckmate(currentSide)) {
        return evaluate(board);
    }

    auto moves = board.generateLegalMoves(currentSide);
    if (maximizingPlayer) {
        int maxEval = std::numeric_limits<int>::min();
        for (const auto& move : moves) {

            auto next = board;
            next.makeMove(move);

            int eval = minimax(next, depth - 1, alpha, beta, false);            
            maxEval = std::max(maxEval, eval);
            alpha = std::max(alpha, eval);
            if (beta <= alpha)
                break;
        }
        return maxEval;
    }
    else {
        int minEval = std::numeric_limits<int>::max();
        for (const auto& move : moves) {

            Board next = board;
            next.makeMove(move);
            int eval = minimax(next, depth - 1, alpha, beta, true);
            next.undoMove();

            minEval = std::min(minEval, eval);
            beta = std::min(beta, eval);
            if (beta <= alpha)
                break;
        }
        return minEval;
    }
}