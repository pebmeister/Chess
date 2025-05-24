#pragma once
#include <random>
#include <array>
#include <cstdint>

struct Zobrist {
    static constexpr int NUM_SQUARES = 64;
    static constexpr int NUM_PIECE_TYPES = 6; // Pawn, Knight, Bishop, Rook, Queen, King
    static constexpr int NUM_COLORS = 2;      // White, Black

    // [piece][color][square]
    std::array<std::array<std::array<uint64_t, NUM_SQUARES>, NUM_COLORS>, NUM_PIECE_TYPES> pieceSquare;
    uint64_t sideToMove;
    std::array<uint64_t, 4> castlingRights; // KQkq
    std::array<uint64_t, 8> enPassantFile;  // a-h

    Zobrist()
    {
        std::mt19937_64 rng(20240524); // Fixed seed for reproducibility
        std::uniform_int_distribution<uint64_t> dist;

        for (int pt = 0; pt < NUM_PIECE_TYPES; ++pt)
            for (int c = 0; c < NUM_COLORS; ++c)
                for (int sq = 0; sq < NUM_SQUARES; ++sq)
                    pieceSquare[pt][c][sq] = dist(rng);

        sideToMove = dist(rng);
        for (auto& v : castlingRights) v = dist(rng);
        for (auto& v : enPassantFile) v = dist(rng);
    }
};
