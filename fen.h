#pragma once

#include <map>
#include <string>
#include <array>
#include <stdexcept>
#include <sstream>
#include <vector>

#include "chesstypes.h"

inline const std::map<char, Piece>& getPieceMap()
{
    static const std::map<char, Piece> pieceMap =
    {
        { 'p', Piece(PieceType::Pawn, Color::Black)},
        { 'P', Piece(PieceType::Pawn, Color::White)},
        { 'n', Piece(PieceType::Knight, Color::Black)},
        { 'N', Piece(PieceType::Knight, Color::White)},
        { 'r', Piece(PieceType::Rook, Color::Black)},
        { 'R', Piece(PieceType::Rook, Color::White)},
        { 'b', Piece(PieceType::Bishop, Color::Black)},
        { 'B', Piece(PieceType::Bishop, Color::White)},
        { 'q', Piece(PieceType::Queen, Color::Black)},
        { 'Q', Piece(PieceType::Queen, Color::White)},
        { 'k', Piece(PieceType::King, Color::Black)},
        { 'K', Piece(PieceType::King, Color::White)}
    };
    return pieceMap;
}

class Fen {

private:
    static const int NUM_SQ = 64;
    std::array<Piece, NUM_SQ> board;

public:
    uint64_t white_pawns = 0;
    uint64_t white_knights = 0;
    uint64_t white_bishops = 0;
    uint64_t white_rooks = 0;
    uint64_t white_queens = 0;
    uint64_t white_kings = 0;

    uint64_t black_pawns = 0;
    uint64_t black_knights = 0;
    uint64_t black_bishops = 0;
    uint64_t black_rooks = 0;
    uint64_t black_queens = 0;
    uint64_t black_kings = 0;

    uint64_t allPieces = 0;            // all pieces on the board
    uint64_t whitePieces = 0;
    uint64_t blackPieces = 0;

    Color turn = Color::White;
    std::vector<std::string> enpassant;
    bool whiteKingside = false;
    bool whiteQueenside = false;
    bool blackKingside = false;
    bool blackQueenside = false;
    int halfMoves = 0;
    int fullMoves = 0;

private:

    std::string _fenString;
    std::string _boardStr, _turnStr, _castleStr, _enPassantStr, _halfStr, _fullStr;

    Piece FenToPiece(char fen)
    {
        auto pos = getPieceMap().find(fen);
        if (pos == getPieceMap().end()) {
            return Piece(PieceType::None, Color::White);
        }
        return pos->second;
    }

    bool parseBoard()
    {
        int rank = 7;
        int file = 0;

        for (char ch : _boardStr) {
            if (ch == '/') {
                --rank;
                file = 0;
                continue;
            }

            if (std::isdigit(ch)) {
                file += ch - '0';
                continue;
            }

            auto piece = FenToPiece(ch);
            if (file >= 8 || rank < 0)
                throw std::runtime_error("Invalid FEN placement");

            board[rank * 8 + file] = piece;
            ++file;
        }
        return true;
    }

    bool parseTurn()
    {
        if (_turnStr.length() != 1) {
            throw std::runtime_error("Invalid fen file. Turn format incorrect.");
            return false;
        }
        auto ch = _turnStr[0];
        if (ch != 'w' && ch != 'b') {
            throw std::runtime_error("Invalid fen file. Turn format incorrect.");
            return false;
        }
        turn = ch == 'w' ? Color::White : Color::Black;
        return true;
    }

    bool parseCastle()
    {
        bool noneAvailable = false;
        for (auto& ch : _castleStr) {

            if (ch == '-') {
                noneAvailable = true;
                continue;
            }

            if (noneAvailable) {
                throw std::runtime_error("Invalid fen file. Castle incorrect. None available already specified.");
                return false;
            }

            auto p = FenToPiece(ch);

            switch (p.type) {
                case PieceType::King:
                    if (p.color == Color::White) {
                        whiteKingside = true;
                    }
                    else if (p.color == Color::Black) {
                        blackKingside = true;
                    }
                    else {
                        throw std::runtime_error("Invalid fen file. King castle incorrect.");
                        return false;
                    }
                    break;

                case PieceType::Queen:
                    if (p.color == Color::White) {
                        whiteQueenside = true;
                    }
                    else if (p.color == Color::Black) {
                        blackQueenside = true;
                    }
                    else {
                        throw std::runtime_error("Invalid fen file. King castle incorrect.");
                        return false;
                    }
                    break;

                default:
                    throw std::runtime_error("Invalid fen file. Castle incorrect.");
                    return false;
            }
        }
        return true;
    }

    bool parseEnpassant()
    {
        if (_enPassantStr == "-")
            return true;

        auto start = 0;
        while (start < _enPassantStr.size()) {
            if (_enPassantStr.size() - start < 2) {
                throw std::runtime_error("Invalid fen file. enpassant incorrect.");
                return false;
            }
            auto en = _enPassantStr.substr(start, 2);
            if (en[0] < 'a' || en[0] > 'h' || en[1] < '0' || en[1] > '9') {
                throw std::runtime_error("Invalid fen file. enpassant incorrect.");
                return false;
            }
            enpassant.push_back(en);
            start += 2;
        }
        return true;
    }

    bool parseHalfMove()
    {
        if (_halfStr.empty()) return true;
        halfMoves = std::stoi(_halfStr);
        return true;
    }

    bool parseFullMove()
    {
        if (_fullStr.empty()) return true;
        fullMoves = std::stoi(_fullStr);
        return true;
    }

public:
    bool parse()
    {
        std::istringstream ss(_fenString);
        ss >> _boardStr >> _turnStr >> _castleStr >> _enPassantStr;
        if (!ss.eof()) {
            ss >> _halfStr >> _fullStr;
        }

        auto okay = parseBoard();

        if (okay) okay = parseTurn();
        if (okay) okay = parseCastle();
        if (okay) okay = parseEnpassant();
        if (okay) okay = parseHalfMove();
        if (okay) okay = parseFullMove();

        if (okay) {
            white_pawns = white_knights = white_bishops = white_rooks = white_queens = white_kings = 0;
            black_pawns = black_knights = black_bishops = black_rooks = black_queens = black_kings = 0;

            for (int y = 0; y < 8; ++y) {
                for (int x = 0; x < 8; ++x) {
                    auto p = board[y * 8 + x];

                    auto bit = y * 8 + x;
                    uint64_t mask = 1ULL << bit;

                    switch (p.type) {
                        case PieceType::King:
                            if (p.color == Color::White)
                                white_kings |= mask;
                            else
                                black_kings |= mask;
                            break;

                        case PieceType::Queen:
                            if (p.color == Color::White)
                                white_queens |= mask;
                            else
                                black_queens |= mask;
                            break;

                        case PieceType::Bishop:
                            if (p.color == Color::White)
                                white_bishops |= mask;
                            else
                                black_bishops |= mask;
                            break;

                        case PieceType::Knight:
                            if (p.color == Color::White)
                                white_knights |= mask;
                            else
                                black_knights |= mask;
                            break;

                        case PieceType::Rook:
                            if (p.color == Color::White)
                                white_rooks |= mask;
                            else
                                black_rooks |= mask;
                            break;

                        case PieceType::Pawn:
                            if (p.color == Color::White)
                                white_pawns |= mask;
                            else
                                black_pawns |= mask;
                            break;

                        default:
                            break;
                    }
                }
            }
            whitePieces =
                white_pawns |
                white_knights |
                white_bishops |
                white_rooks |
                white_queens |
                white_kings;

            blackPieces =
                black_pawns |
                black_knights |
                black_bishops |
                black_rooks |
                black_queens |
                black_kings;

            allPieces = whitePieces | blackPieces;
        }
        return okay;
    }

    // 
    std::string toString() const
    {
        std::ostringstream fen;

        // Board: rank 8 to 1
        for (int rank = 7; rank >= 0; --rank) {
            int empty = 0;
            for (int file = 0; file < 8; ++file) {
                const Piece& p = board[rank * 8 + file];
                if (p.type == PieceType::None) {
                    ++empty;
                }
                else {
                    if (empty > 0) {
                        fen << empty;
                        empty = 0;
                    }
                    char c = ' ';
                    switch (p.type) {
                        case PieceType::Pawn:   c = 'p'; break;
                        case PieceType::Knight: c = 'n'; break;
                        case PieceType::Bishop: c = 'b'; break;
                        case PieceType::Rook:   c = 'r'; break;
                        case PieceType::Queen:  c = 'q'; break;
                        case PieceType::King:   c = 'k'; break;
                        default: break;
                    }
                    if (p.color == Color::White)
                        c = std::toupper(c);
                    fen << c;
                }
            }
            if (empty > 0)
                fen << empty;
            if (rank > 0)
                fen << '/';
        }

        // Turn
        fen << ' ' << (turn == Color::White ? 'w' : 'b');

        // Castling
        std::string castles;
        if (whiteKingside) castles += 'K';
        if (whiteQueenside) castles += 'Q';
        if (blackKingside) castles += 'k';
        if (blackQueenside) castles += 'q';
        fen << ' ' << (castles.empty() ? "-" : castles);

        // En passant
        if (enpassant.empty()) {
            fen << " -";
        }
        else {
            std::string all;
            for (auto& e : enpassant) {
                all += e;
            }
            fen << ' ' << all;
        }

        // Half/full move counts
        fen << ' ' << halfMoves << ' ' << fullMoves;

        return fen.str();
    }

    void clear()
    {
        for (auto i = 0; i < NUM_SQ; ++i)
            board[i] = { PieceType::None, Color::White };

        enpassant.clear();

        _fenString = "";
        _boardStr = "";
        _turnStr = "";
        _castleStr = "";
        _enPassantStr = "";
        _halfStr = "";
        _fullStr = "";
    }

    void load(std::string_view fen)
    {
        clear();
        _fenString = fen;
        auto _ = parse();
    }


    Fen()
    {
        clear();
    }

    Fen(std::string_view fen)
    {
        load(fen);
    }

    Fen(std::array<Piece, NUM_SQ>& _board,
        std::vector<std::string> _enpassant = { "-" },
        Color _turn = Color::White,
        bool _whiteKingside = false,
        bool _whiteQueenside = false,
        bool _blackKingside = false,
        bool _blackQueenside = false,
        int _halfMoves = 0,
        int _fullMoves = 0)
    {
        clear();

        board = _board;
        enpassant = _enpassant;
        turn = _turn;
        whiteKingside = _whiteKingside;
        whiteQueenside = _whiteQueenside;
        blackKingside = _blackKingside;
        blackQueenside = _blackQueenside;
        halfMoves = _halfMoves;
        fullMoves = _fullMoves;
    }
};
