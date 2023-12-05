#ifndef BITBOARD_H
#define BITBOARD_H

#include <cstdint>
#include <iostream>
#include <map>
#include <set>
#include <vector>
#include "bitboard_helpers.h"
#include "constants.h"
#include <array>

using namespace std;

// ALl piece bitboards -> 12 in total -> one for each piece type and colour
extern array<uint64_t, 12> piece_bitboards;

// Bitboard occupancies
extern array<uint64_t, 3> occupancies; // white, black, both

// colour to move first
extern int colour_to_move;

// enpassant square
extern int enpassant;

// castle rights
extern int castle_rights;

// for copying and restoring board
extern array<uint64_t, 12> piece_bitboards_copy;
extern array<uint64_t, 3> occupancies_copy;
extern int colour_to_move_copy;
extern int enpassant_copy;
extern int castle_rights_copy;


// leaping
extern std::array<std::array<uint64_t, BOARD_SIZE>, 2> pawn_attacks;
extern std::array<uint64_t, BOARD_SIZE> knight_attacks;
extern std::array<uint64_t, BOARD_SIZE> king_attacks;

// sliding
// [square][occupancy]
extern array<uint64_t, 64> bishop_masks;
extern array<array<uint64_t, 512>, 64> bishop_attacks;
extern array<uint64_t, 64> rook_masks;
extern vector<vector<uint64_t>> rook_attacks;


// METHODS


// get occupancy bitboard by colour
uint64_t get_white_occupancy();
uint64_t get_black_occupancy(); 
uint64_t get_both_occupancy();    


// pre-compute all attacks from a square methods

// pawns
uint64_t get_pawn_attacks(int side,const int& square);

// knights
uint64_t get_knight_attacks(const int& sqaure);

// kings
uint64_t get_king_attacks(const int& square);

// bishops
uint64_t get_bishop_mask(const int& square);
uint64_t bishop_attack_runtime_gen(int square, const uint64_t& blockers);
uint64_t get_bishop_attacks(int square, uint64_t occupancy);

// rooks
uint64_t get_rook_mask(const int& square);
uint64_t rook_attack_runtime_gen(int square, uint64_t& blockers);
uint64_t get_rook_attacks(int square, uint64_t occupancy);

// queen
uint64_t get_queen_attacks(int square, uint64_t occupancy);

// set occupancy
uint64_t set_occupancy(const int& index, const int& bits_in_mask, uint64_t attack_mask);

bool is_square_under_attack(int square, int side);


// MAGIC NUMBERS AND BITBOARDS
uint64_t find_magic_num(const int& square, int relevant_bits, int bishop);
void init_magic_nums();

// init all piece attacks
void init_leaping_attacks();
void init_sliding_attacks(int isBishop);

void init_all();
        

#endif
