#include <cstdint>


uint64_t set_bit(uint64_t bitboard, int bit) {
    return bitboard | (1ULL << bit);
}

uint64_t clear_bit(uint64_t bitboard, int bit) {
    return bitboard & ~(1ULL << bit);
}

bool is_bit_set(uint64_t bitboard, int bit) {
    return (bitboard & (1ULL << bit)) != 0;
}

uint64_t toggle_bit(uint64_t bitboard, int bit) {
    return bitboard ^ (1ULL << bit);
}