#include <iostream>
#include "board.h"
#include "bitboard.h"
#include "engine.h"
#include "chess.h"
#include "fen.h"

void draw_pieces(const Board& b, int row, int col, int squareWidth, int squareHeight);
void drawChessboard(int startY, int startX, int squareWidth, int squareHeight);
void print_moves(const Board& board, const std::vector<Move>& moves, int startRow, int startCol);
void print_status(const Board& board, int msgRow, int msgCol, int checkRow, int checkCol, const std::string& extra = "");

std::vector<Move> lastmoves;
ANSI_ESC ansi;

int main()
{
    constexpr int BOARD_ROW = 2, BOARD_COL = 2;
    constexpr int CHECK_ROW = 30, CHECK_COL = 11;
    constexpr int MSG_ROW = 31, MSG_COL = 11;
    constexpr int MOVES_ROW = 2, MOVES_COL = 50;
    constexpr int SQUARE_W = 4, SQUARE_H = 2;
    constexpr int white_level = 3;
    constexpr int black_level = 3;

    Board board;
    Engine engine;
    board.reset();

    // Clear and set up the display
    std::cout << ansi.gr({ ansi.BLUE_BACKGROUND })
        << ansi.ERASE_ALL_DISPLAY << ansi.HOME << ansi.HIDE_CURSOR;

    drawChessboard(BOARD_ROW, BOARD_COL, SQUARE_W, SQUARE_H);
    draw_pieces(board, BOARD_ROW, BOARD_COL, SQUARE_W, SQUARE_H);

    bool end = false;
    std::vector<Move> moves;
    board.turn = Color::Black;
    for (int moveCount = 0; !end; ++moveCount) {
        auto level = board.turn == Color::White ? white_level : black_level;
        auto move = engine.findBestMove(board, level, moves);
        print_moves(board, moves, MOVES_ROW, MOVES_COL);

        if (move.from == move.to) {
            print_status(board, MSG_ROW, MSG_COL, CHECK_ROW, CHECK_COL, "No legal moves. ");
            end = true;
            break;
        }

        print_status(board, MSG_ROW, MSG_COL, CHECK_ROW, CHECK_COL,
            (board.turn == Color::White ? "White" : "Black") + std::string(" ") + move.toString() + " " + std::to_string(move.score));

        board.makeMove(move);
        draw_pieces(board, BOARD_ROW, BOARD_COL, SQUARE_W, SQUARE_H);

        if (board.isCheckmate(board.turn)) {
            print_status(board, MSG_ROW, MSG_COL, CHECK_ROW, CHECK_COL,
                (board.turn == Color::White ? "White" : "Black") + std::string(" is in checkmate!"));
            break;
        }
        else if (board.isInCheck(board.turn)) {
            print_status(board, MSG_ROW, MSG_COL, CHECK_ROW, CHECK_COL,
                (board.turn == Color::White ? "White" : "Black") + std::string(" is in check!"));
        }
    }

    board.reset();
    std::cout << ansi.SHOW_CURSOR;
    return 0;
}

void print_status(const Board& board, int msgRow, int msgCol, int checkRow, int checkCol, const std::string& extra)
{
    std::cout << ansi.pos(msgRow, msgCol) << ansi.gr(ansi.BLUE_BACKGROUND) << ansi.ERASE_IN_LINE;
    if (!extra.empty())
        std::cout << extra;

}

void print_moves(const Board& board, const std::vector<Move>& moves, int startRow, int startCol)
{
    // Clear previous moves
    int r = startRow, c = startCol;
    for (size_t i = 0; i < lastmoves.size(); ++i) {
        std::cout << ansi.pos(r++, c) << ansi.gr(ansi.BLUE_BACKGROUND) << ansi.ERASE_CURSOR_EOL;
    }

    r = startRow;
    auto& color = board.turn == Color::White ? ansi.BRIGHT_BLUE_FOREGROUND : ansi.BRIGHT_GREEN_FOREGROUND;
    for (const auto& move : moves) {
        if (move.score != 0 || r == startRow) {
            std::cout << ansi.pos(r++, c)
                << ansi.gr(color) << ansi.gr(ansi.BLACK_BACKGROUND)
                << move.toString() << " score " << move.score
                << ansi.gr(std::list<std::string>());
        }
    }
    lastmoves = moves;
}

void drawChessboard(int startY, int startX, int squareWidth, int squareHeight)
{
    const int boardSize = 8;
    const int lside = squareWidth / 2;

    const std::string lightSquare = ansi.gr({ ansi.BRIGHT_WHITE_BACKGROUND, ansi.BLACK_FOREGROUND });
    const std::string darkSquare = ansi.gr({ ansi.BLACK_BACKGROUND, ansi.WHITE_FOREGROUND });
    const std::string bannerColor = ansi.gr({ ansi.BRIGHT_WHITE_BACKGROUND, ansi.BLACK_FOREGROUND });
    const std::string reset = ansi.gr(ansi.RESET_ALL);

    // Top border
    int startpos = startX + 2;
    for (char c = 'a'; c <= 'h'; c++) {
        std::cout << bannerColor << ansi.pos(startY, startpos);
        for (int l = 0; l < lside; ++l) std::cout << ' ';
        std::cout << c;
        for (int l = 0; l < lside; ++l) std::cout << ' ';
        startpos += squareWidth;
    }
    std::cout << reset << std::endl;

    // Board and ranks
    for (int row = 0; row < boardSize; row++) {
        std::cout << bannerColor
            << ansi.pos(startY + 2 + row * squareHeight + squareHeight / 2, startX)
            << (8 - row)
            << reset;

        for (int col = 0; col < boardSize; col++) {
            bool isLight = (row + col) % 2 == 0;
            std::string color = isLight ? lightSquare : darkSquare;
            for (int y = 0; y < squareHeight; y++) {
                std::cout << ansi.pos(startY + 2 + row * squareHeight + y,
                    startX + 2 + col * squareWidth)
                    << color;
                for (int x = 0; x < squareWidth; x++) std::cout << " ";
            }
        }

        std::cout << bannerColor
            << ansi.pos(startY + 2 + row * squareHeight + squareHeight / 2,
                startX + 3 + boardSize * squareWidth)
            << (8 - row)
            << reset << std::endl;
    }

    // Bottom border
    startpos = startX + 2;
    for (char c = 'a'; c <= 'h'; c++) {
        std::cout << bannerColor << ansi.pos(startY + 3 + (8 * squareHeight), startpos);
        for (int l = 0; l < lside; ++l) std::cout << ' ';
        std::cout << c;
        for (int l = 0; l < lside; ++l) std::cout << ' ';
        startpos += squareWidth;
    }
    std::cout << reset << std::endl;
}

void draw_pieces(const Board& b, int row, int col, int squareWidth, int squareHeight)
{
    bool iswhitesquare = true;
    for (int y = 7; y >= 0; --y) {
        for (int x = 0; x < 8; ++x) {
            auto piece = b.get(x, y);
            auto p = piece.toString();
            if (p == ".") p = " ";
            auto piece_forcolor = piece.color == Color::White ? ansi.BRIGHT_BLUE_FOREGROUND : ansi.BRIGHT_GREEN_FOREGROUND;
            auto piece_backcolor = iswhitesquare ? ansi.BRIGHT_WHITE_BACKGROUND : ansi.BLACK_BACKGROUND;

            int r = 7 - y;
            int c = x;
            int rr = row + 1 + squareHeight / 2 + r * squareHeight;
            int cc = col + 1 + squareWidth / 2 + c * squareWidth;

            std::cout << ansi.pos(rr, cc)
                << ansi.gr(piece_forcolor)
                << ansi.gr(piece_backcolor)
                << ansi.gr(ansi.BOLD)
                << p
                << ansi.gr(std::list<std::string>());
            iswhitesquare = !iswhitesquare;
        }
        iswhitesquare = !iswhitesquare;
    }
}
