#include <cstdint>
#include <vector>
#include <bitset>

using namespace std;

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

vector<int> get_squares_from_bb(uint16_t bitboard)
{
    vector<int> squares;
    bitset<64> bits(bitboard);

    for (int i=0;i<64;i++)
    {
        if (bits[i]==1)
            squares.push_back(i);
    }
    return squares;
}
