// engine.h
#pragma once
#include "board.h"

class Engine {
public:
    Move findBestMove(Board& board, int depth, std::vector<Move>& moves);
    int64_t evaluate(const Board& board);

private:
    int64_t minimax(Board& board, int depth, int64_t alpha, int64_t beta, bool maximizingPlayer);
    void orderMoves(Board& board, std::vector<Move>& moves);
};
