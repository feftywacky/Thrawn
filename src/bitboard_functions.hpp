#ifndef BITBOARD_FUNCTIONS_HPP
#define BITBOARD_FUNCTIONS_HPP

#include <vector>
#include <bitset>
#include <map>
#include <set>
#include "constants.hpp"

using namespace std;

class Bitboard;

extern unsigned int random_state;

void set_bit(uint64_t& bitboard, int bit);

void clear_bit(uint64_t& bitboard, int bit);

void pop_bit(uint64_t& bitboard, int bit);

uint64_t get_bit(uint64_t bitboard, int bit);

bool is_bit_set(uint64_t bitboard, int bit);

void toggle_bit(uint64_t& bitboard, int bit);

vector<int> get_squares_from_bb(uint16_t bitboard);  

// BIT MANIPULATION
int count_bits(uint64_t bitboard);

int get_lsb_index(uint64_t bitboard);

// XOR SHIFT RANDOM NUMBER GEN ALGORITHM
// Generate 32-bit pseudo legal numbers
uint32_t get_random_U32();

// Generate 64-bit pseudo legal numbers
uint64_t get_random_U64();

uint64_t gen_magic_num();

void pprint_bb(uint64_t bitboard);

void print_bitboard(uint64_t bitboard);

void print_board(Bitboard& board, Side side);

void print_bits(uint64_t num);

#endif