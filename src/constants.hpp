#ifndef CONSTANTS_HPP
#define CONSTANTS_HPP

#include <cstdint>
#include <array>

enum Side {
    white,
    black
};

enum {rook, bishop};

enum {
    a8, b8, c8, d8, e8, f8, g8, h8,
    a7, b7, c7, d7, e7, f7, g7, h7,
    a6, b6, c6, d6, e6, f6, g6, h6,
    a5, b5, c5, d5, e5, f5, g5, h5,
    a4, b4, c4, d4, e4, f4, g4, h4,
    a3, b3, c3, d3, e3, f3, g3, h3,
    a2, b2, c2, d2, e2, f2, g2, h2,
    a1, b1, c1, d1, e1, f1, g1, h1
};

// CONSTANTS (only declarations)
constexpr int BOARD_SIZE = 64;
extern const uint64_t not_a_file;
extern const uint64_t not_h_file;
extern const uint64_t not_hg_file;
extern const uint64_t not_ab_file;
extern const std::array<const char*, 64> square_to_coordinates;

extern const std::array<int, 64> bishop_relevant_bits;
extern const std::array<int, 64> rook_relevant_bits;

extern std::array<uint64_t, 64> rook_magic_nums;
extern std::array<uint64_t, 64> bishop_magic_nums;

#endif