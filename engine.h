// engine.h
#pragma once
#include "board.h"

class Engine {
public:
    Move findBestMove(Board& board, int depth, std::vector<Move>& moves);

private:
    int evaluate(const Board& board);
    int minimax(Board& board, int depth, int alpha, int beta, bool maximizingPlayer);
};

