#ifndef BITBOARD_HELPERS_H
#define BITBOARD_HELPERS_H

#include <vector>
#include <bitset>
#include <map>
#include <set>
#include "constants.h"
#include "zobrist_hashing.h"
#include "search.h"
#include <cstdint>

using namespace std;

// class Bitboard;

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

void print_bitboard(uint64_t bitboard);

void print_board(int side);

void print_bits(uint64_t num);

// copying and restoring for move take backs
//#define copyBoard() \
//    array<uint64_t, 12> piece_bitboards_copy; \
//    array<uint64_t, 3> occupancies_copy; \
//    int colour_to_move_copy; \
//    int enpassant_copy; \
//    int castle_rights_copy; \
//    uint64_t zobristKey_copy; \
//    int fifty_move_copy; \
//    piece_bitboards_copy = piece_bitboards; \
//    occupancies_copy = occupancies; \
//    colour_to_move_copy = colour_to_move; \
//    enpassant_copy = enpassant; \
//    castle_rights_copy = castle_rights; \
//    zobristKey_copy = zobristKey; \
//    fifty_move_copy = fifty_move; \

// Restore board state
//#define restoreBoard() \
//    piece_bitboards = piece_bitboards_copy; \
//    occupancies = occupancies_copy; \
//    colour_to_move = colour_to_move_copy; \
//    enpassant = enpassant_copy; \
//    castle_rights = castle_rights_copy; \
//    zobristKey = zobristKey_copy; \
//    fifty_move = fifty_move_copy; \

#endif