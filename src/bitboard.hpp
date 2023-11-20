#ifndef BITBOARD_HPP
#define BITBOARD_HPP

#include <cstdint>
#include <iostream>
#include <map>
#include <set>
#include <vector>
#include "bitboard_functions.hpp"
#include "constants.hpp"
#include <array>

using namespace std;

class Bitboard
{
public:
    // ALl piece bitboards -> 12 in total -> one for each piece type and colour
    array<uint64_t, 12> piece_bitboards;

    // Bitboard occupancies
    array<uint64_t, 3> occupancies; // white, black, both

    // colour to move first
    Side colour_to_move;

    // enpassant square
    int enpassant;

    // castle rights
    int castle_rights;



    // leaping
    std::array<std::array<uint64_t, BOARD_SIZE>, 2> pawn_attacks;
    std::array<uint64_t, BOARD_SIZE> knight_attacks;
    std::array<uint64_t, BOARD_SIZE> king_attacks;

    // sliding
    // [square][occupancy]
    array<uint64_t, 64> bishop_masks;
    array<array<uint64_t, 512>, 64> bishop_attacks;
    array<uint64_t, 64> rook_masks;
    vector<vector<uint64_t>> rook_attacks;
    



    // contructor
    Bitboard();

    // get occupancy bitboard by colour
    uint64_t get_white_occupancy();
    uint64_t get_black_occupancy(); 
    uint64_t get_both_occupancy();    


    // pre-compute all attacks from a square methods

    // pawns
    uint64_t get_pawn_attacks(Side side,const int& square);
    
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

private:
    // MAGIC NUMBERS AND BITBOARDS
    uint64_t find_magic_num(const int& square, int relevant_bits, int bishop);
    void init_magic_nums();

    // init all piece attacks
    void init_leaping_attacks();
    void init_sliding_attacks(int isBishop);
        
};

#endif