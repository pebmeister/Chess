#include <iostream>

#include "board.h"
#include "bitboard.h"
#include "engine.h"
#include "chess.h"
#include "fen.h"

const int BOARD_ROW = 2;
const int BOARD_COL = 2;

int main()
{
    Board board;
    Engine engine;

    std::cout  <<
        std::string(ansi.gr(std::list<std::string>({ ansi.BLUE_BACKGROUND }))) <<
        ansi.ERASE_ALL_DISPLAY << ansi.HOME << ansi.HIDE_CURSOR;

    int row = BOARD_ROW, col = BOARD_COL;
    drawChessboard(row, col, 4, 2);
    printBoard(board, row, col, 4, 2);

    bool end = false;
    for (int moveCount = 0; !end && moveCount < 50; ++moveCount) {
        auto move = engine.findBestMove(board, 4);
        if (move.from.x == move.to.x && move.from.y == move.to.y) {
            std::cout << ansi.pos(31, 11) << "No legal moves. ";
            end = true;
            if (board.isInCheck(board.turn)) {
                std::cout << (board.turn == Color::White ? "White" : "Black") << " is in checkmate!\n";
            }
            else {
                std::cout << "Stalemate.\n";
            }
            break;
        }
        std::cout
            << ansi.pos(31, 11)
            << ansi.gr(ansi.BLUE_BACKGROUND)
            << (board.turn == Color::White ? "White" : "Black")
            << " " << move.toString() 
            << ansi.gr(ansi.BLUE_BACKGROUND) 
            << ansi.ERASE_IN_LINE;

        board.makeMove(move);
        printBoard(board, row, col, 4, 2);

        if (board.isCheckmate(board.turn)) {
            std::cout
                << ansi.pos(35, 11)
                << (board.turn == Color::White ? "White" : "Black") << " is in checkmate!\n";
            break;
        }
        else if (board.isInCheck(board.turn)) {
            std::cout 
                << ansi.pos(35, 11)
                << (board.turn == Color::White ? "White" : "Black") << " is in check!\n";
        }
    }

    return 0;
}
