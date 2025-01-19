#ifndef BITBOARD_H
#define BITBOARD_H

#include <cstdint>
#include <iostream>
#include <map>
#include <set>
#include <vector>
#include "bitboard_helpers.h"
#include "constants.h"
#include "position.h"
#include <array>

using namespace std;

// get occupancy bitboard by colour
uint64_t get_white_occupancy(thrawn::Position& pos);
uint64_t get_black_occupancy(thrawn::Position& pos);
uint64_t get_both_occupancy(thrawn::Position& pos);

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
uint64_t get_bishop_attacks(thrawn::Position& pos, int square, uint64_t occupancy);

// rooks
uint64_t get_rook_mask(const int& square);
uint64_t rook_attack_runtime_gen(int square, uint64_t& blockers);
uint64_t get_rook_attacks(thrawn::Position& pos, int square, uint64_t occupancy);

// queen
uint64_t get_queen_attacks(thrawn::Position& pos, int square, uint64_t occupancy);

// set occupancy
uint64_t set_occupancy(const int& index, const int& bits_in_mask, uint64_t attack_mask);

bool is_square_under_attack(thrawn::Position& pos,int square, int side);


// MAGIC NUMBERS AND BITBOARDS
uint64_t find_magic_num(const int& square, int relevant_bits, int bishop);
void init_magic_nums();

bool noMajorsOrMinorsPieces(thrawn::Position& pos);

// init all piece attacks
void init_leaping_attacks(thrawn::Position& pos);
void init_sliding_attacks(thrawn::Position& pos, int isBishop);
        

#endif
