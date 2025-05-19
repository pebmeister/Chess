#pragma once
#include "ANSIEsc.h"
#include "fen.h"
#include "board.h"

extern ANSI_ESC ansi;
extern Fen fen;
extern void printmoves(Board board, std::vector<Move> moves);
extern void drawChessboard(int startY, int startX, int squareWidth, int squareHeight);
extern void printBoard(const Board& board, int row, int col, int squareWidth, int squareHeight);
