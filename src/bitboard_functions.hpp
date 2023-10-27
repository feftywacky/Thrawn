#ifndef BITBOARD_FUNCTIONS_HPP
#define BITBOARD_FUNCTIONS_HPP

#include <vector>
#include <bitset>
#include <map>
#include <set>

using namespace std;


uint64_t set_bit(uint64_t bitboard, int bit);

uint64_t clear_bit(uint64_t bitboard, int bit);

uint64_t get_bit(uint64_t bitboard, int bit);

bool is_bit_set(uint64_t bitboard, int bit);

uint64_t toggle_bit(uint64_t bitboard, int bit);

vector<int> get_squares_from_bb(uint16_t bitboard);  

void pprint_bb(uint64_t bitboard);

void print_bitboard(uint64_t bitboard);

void print_bits(uint64_t num);

#endif