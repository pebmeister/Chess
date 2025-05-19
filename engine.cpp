// engine.cpp
#include <limits>
#include <iostream>
#include <assert.h>
#include "engine.h"
#include "chess.h"

ANSI_ESC ansi;
Fen fen;

void drawChessboard(int startY, int startX, int squareWidth = 4, int squareHeight = 2)
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

void printBoard(const Board& board, int row, int col, int squareWidth = 4, int squareHeight = 2)
{
    std::string piece_forcolor;
    std::string piece_backcolor;

    char ch;
    bool  iswhitesquare = true;
    for (auto r = 0; r < 8; ++r) {
        for (auto c = 0; c < 8; ++c) {
            auto p = board.get(c, r);
            switch (p.type) {
                case PieceType::Pawn:
                    ch = 'P';
                    break;

                case PieceType::Rook:
                    ch = 'R';
                    break;

                case PieceType::Knight:
                    ch = 'N';
                    break;

                case PieceType::Bishop:
                    ch = 'B';
                    break;

                case PieceType::Queen:
                    ch = 'Q';
                    break;

                case PieceType::King:
                    ch = 'K';
                    break;

                default:
                    ch = ' ';
                    break;
            }
            if (p.color == Color::Black) {
                ch = tolower(ch);
            }

            piece_forcolor = p.color == Color::White ? ansi.BRIGHT_BLUE_FOREGROUND : ansi.BRIGHT_GREEN_FOREGROUND;
            piece_backcolor = iswhitesquare ? ansi.BRIGHT_WHITE_BACKGROUND : ansi.BLACK_BACKGROUND;

            auto rr = row + 1 + squareHeight / 2 + r * squareHeight;
            auto cc = col + 1 + squareWidth / 2 + c * squareWidth;
            std::cout << ansi.pos(rr, cc) << ansi.gr(piece_forcolor) << ansi.gr(piece_backcolor) << ansi.gr(ansi.BOLD) << ch;
            std::cout << ansi.gr(std::list<std::string>());

            iswhitesquare = !iswhitesquare;
        }
        iswhitesquare = !iswhitesquare;
    }
}

std::vector<Move> lastmoves;
void printmoves(Board board, std::vector<Move> moves)
{
    auto r = 1, c = 50;

    for (auto i = 0; i < lastmoves.size(); ++i) {
        std::cout << ansi.pos(r, c) << ansi.ERASE_CURSOR_EOL;
        r++;
    }

    r = 1, c = 50;

    auto& color = board.turn == Color::White ? ansi.BRIGHT_BLUE_FOREGROUND : ansi.BRIGHT_GREEN_FOREGROUND;
    for (auto& move : moves) {
        std::cout << ansi.pos(r, c) << ansi.gr(color) << move.toString() << ansi.gr(std::list<std::string>());
        r++;
    }
    lastmoves = moves;
}

Move Engine::findBestMove(Board& board, int depth)
{
    Move bestMove;
    int bestValue = std::numeric_limits<int>::min();

    Color currentTurn = board.getTurn(); // Save current turn
    auto moves = board.generateLegalMoves(currentTurn);
    printmoves(board, moves);

    for (auto& move : moves) {
        auto next = board;
        next.makeMove(move);

        // Pass the opposite color for the next level since we just made a move
        int eval = minimax(next, depth - 1, std::numeric_limits<int>::min(),
            std::numeric_limits<int>::max(), currentTurn == Color::Black);

        move.score = eval;
        if (eval > bestValue) {
            bestValue = eval;
            bestMove = move;
        }
    }

    std::cout << ansi.pos(30, 11) << "Best move: " << bestMove.toString()
        << " value: " << bestValue << ansi.ERASE_IN_LINE;
    return bestMove;
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