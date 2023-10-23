#include <cstdint>
#include <vector>
#include <bitset>

using namespace std;

uint64_t set_bit(uint64_t bitboard, int bit);

uint64_t clear_bit(uint64_t bitboard, int bit);

bool is_bit_set(uint64_t bitboard, int bit);

uint64_t toggle_bit(uint64_t bitboard, int bit);

vector<int> get_squares_from_bb(uint16_t bitboard);  
