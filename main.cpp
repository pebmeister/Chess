#include <iostream>

#include "board.h"
#include "bitboard.h"
#include "engine.h"
#include "chess.h"
#include "fen.h"

void print_board(const Board& b, int row, int col, int squareWidth, int squareHeight);
void drawChessboard(int startY, int startX, int squareWidth, int squareHeight);
void printmoves(Board board, std::vector<Move> moves);

std::vector<Move> lastmoves;
ANSI_ESC ansi;

void printmoves(Board board, std::vector<Move> moves)
{
    auto r = 2, c = 50;

    for (auto i = 0; i < lastmoves.size(); ++i) {
        std::cout << ansi.pos(r, c) << ansi.gr(ansi.BLUE_BACKGROUND) << ansi.ERASE_CURSOR_EOL;
        r++;
    }

    r = 2, c = 50;

    auto& color = board.turn == Color::White ? ansi.BRIGHT_BLUE_FOREGROUND : ansi.BRIGHT_GREEN_FOREGROUND;
    for (auto& move : moves) {
        std::cout << ansi.pos(r, c) << ansi.gr(color) << ansi.gr(ansi.BLACK_BACKGROUND) << move.toString() << ansi.gr(std::list<std::string>());
        r++;
    }
    lastmoves = moves;
}

int main()
{
    const int BOARD_ROW = 2;
    const int BOARD_COL = 2;
    const int CHECK_ROW = 35;
    const int CHECK_COL = 11;
    const int MSG_ROW = 31;
    const int MSG_COL = 11;

    const int level = 4;

    Board board;
    Engine engine;

    std::cout  <<
        std::string(ansi.gr(std::list<std::string>({ ansi.BLUE_BACKGROUND }))) <<
        ansi.ERASE_ALL_DISPLAY << ansi.HOME << ansi.HIDE_CURSOR;

    int row = BOARD_ROW, col = BOARD_COL;
    drawChessboard(row, col, 4, 2);
    print_board(board, row, col, 4, 2);

    bool end = false;
    std::vector<Move> moves;
    for (int moveCount = 0; !end && moveCount < 50; ++moveCount) {
        auto move = engine.findBestMove(board, level, moves);
        printmoves(board, moves);
        if (move.from.x == move.to.x && move.from.y == move.to.y) {
            std::cout << ansi.pos(MSG_ROW, MSG_COL) << "No legal moves. ";
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
            << ansi.pos(MSG_ROW, MSG_COL)
            << ansi.gr(ansi.BLUE_BACKGROUND)
            << (board.turn == Color::White ? "White" : "Black")
            << " " << move.toString() 
            << ansi.gr(ansi.BLUE_BACKGROUND) 
            << ansi.ERASE_IN_LINE;

        board.makeMove(move);
        print_board(board, row, col, 4, 2);

        if (board.isCheckmate(board.turn)) {
            std::cout
                << ansi.pos(CHECK_ROW, CHECK_COL)
                << (board.turn == Color::White ? "White" : "Black") << " is in checkmate!\n";
            break;
        }
        else if (board.isInCheck(board.turn)) {
            std::cout 
                << ansi.pos(CHECK_ROW, CHECK_COL)
                << (board.turn == Color::White ? "White" : "Black") << " is in check!\n";
        }
    }

    return 0;
}

void drawChessboard(int startY, int startX, int squareWidth, int squareHeight)
{
    const int boardSize = 8;     // Standard chessboard size

    // Calculate spacing for banners
    const int lside = squareWidth / 2;
    const int rside = squareWidth - lside - 1;

    // Color definitions
    const std::string lightSquare = ansi.gr({ ansi.BRIGHT_WHITE_BACKGROUND, ansi.BLACK_FOREGROUND });
    const std::string darkSquare = ansi.gr({ ansi.BLACK_BACKGROUND, ansi.WHITE_FOREGROUND });
    const std::string bannerColor = ansi.gr({ ansi.BRIGHT_WHITE_BACKGROUND, ansi.BLACK_FOREGROUND });
    const std::string reset = ansi.gr(ansi.RESET_ALL);

    // Draw top border (a-h)

    auto startpos = startX + 2;
    for (char c = 'a'; c <= 'h'; c++) {
        std::cout << bannerColor << ansi.pos(startY, startpos);  // Use lside for alignment
        for (auto l = 0; l < lside; ++l)
            std::cout << ' ';
        std::cout << c;
        for (auto l = 0; l < lside; ++l)
            std::cout << ' ';
        startpos += squareWidth;
    }

    std::cout << reset << std::endl;

    // Draw the chessboard and ranks
    for (int row = 0; row < boardSize; row++) {
        // Draw left border (8-1) - vertically centered
        std::cout << bannerColor
            << ansi.pos(startY + 2 + row * squareHeight + squareHeight / 2, startX)
            << (8 - row)
            << reset;

        // Draw the squares for this row
        for (int col = 0; col < boardSize; col++) {
            bool isLight = (row + col) % 2 == 0;
            std::string color = isLight ? lightSquare : darkSquare;

            // Draw each line of the square
            for (int y = 0; y < squareHeight; y++) {
                std::cout << ansi.pos(startY + 2 + row * squareHeight + y,
                    startX + 2 + col * squareWidth)
                    << color;

                // Fill the square
                for (int x = 0; x < squareWidth; x++) {
                    std::cout << " ";
                }
            }
        }

        // Draw right border (8-1) - vertically centered
        std::cout << bannerColor
            << ansi.pos(startY + 2 + row * squareHeight + squareHeight / 2,
                startX + 3 + boardSize * squareWidth)
            << (8 - row)
            << reset << std::endl;
    }

    // Draw bottom border (a-h)
    startpos = startX + 2;
    for (char c = 'a'; c <= 'h'; c++) {
        std::cout << bannerColor << ansi.pos(startY + 3 + (8 * squareHeight), startpos);  // Use lside for alignment
        for (auto l = 0; l < lside; ++l)
            std::cout << ' ';
        std::cout << c;
        for (auto l = 0; l < lside; ++l)
            std::cout << ' ';
        startpos += squareWidth;
    }

    std::cout << reset << std::endl;
}

void print_board(const Board& b, int row, int col, int squareWidth, int squareHeight)
{
    auto iswhitesquare = true;
    for (int y = 7; y >= 0; --y) {
        for (int x = 0; x < 8; ++x) {
            auto piece = b.get(x, y);  // assumes (x, y) is (file, rank)            
            auto p = piece.toString();
            if (p == ".") p = " ";
            auto piece_forcolor = piece.color == Color::White ? ansi.BRIGHT_BLUE_FOREGROUND : ansi.BRIGHT_GREEN_FOREGROUND;
            auto piece_backcolor = iswhitesquare ? ansi.BRIGHT_WHITE_BACKGROUND : ansi.BLACK_BACKGROUND;

            auto r = 7 - y;
            auto c = x;

            auto rr = row + 1 + squareHeight / 2 + r * squareHeight;
            auto cc = col + 1 + squareWidth / 2 + c * squareWidth;

            std::cout << ansi.pos(rr, cc) << ansi.gr(piece_forcolor) << ansi.gr(piece_backcolor) << ansi.gr(ansi.BOLD) << p;
            std::cout << ansi.gr(std::list<std::string>());
            iswhitesquare = !iswhitesquare;
        }
        iswhitesquare = !iswhitesquare;
    }
}
