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
        //variables 
        uint64_t whitePawns;
        uint64_t whiteRooks;
        uint64_t whiteKnights;
        uint64_t whiteBishops;
        uint64_t whiteQueens;
        uint64_t whiteKings;

        uint64_t blackPawns;
        uint64_t blackRooks;
        uint64_t blackKnights;
        uint64_t blackBishops;
        uint64_t blackQueens;
        uint64_t blackKings;

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

        void printBB();


        // get bitboards methods
        uint64_t get_white_pieces();
        uint64_t get_black_pieces();     


        // pre-compute all attacks from a square methods

        // pawns
        uint64_t get_pawn_attack_from_sq(Side side, int square);
        
        // knights
        uint64_t get_knight_attack_from_sq(int sqaure);

        // kings
        uint64_t get_king_attack_from_sq(int square);

        // bishops
        uint64_t get_bishop_attack_from_sq(int square);
        uint64_t bishop_attack_runtime_gen(int square, uint64_t blockers);
        uint64_t get_bishop_attacks(int square, uint64_t occupancy);

        // rooks
        uint64_t get_rook_attack_from_sq(int square);
        uint64_t rook_attack_runtime_gen(int square, uint64_t blockers);
        uint64_t get_rook_attacks(int square, uint64_t occupancy);

        // set occupancy
        uint64_t set_occupancy(int index, int bits_in_mask, uint64_t attack_mask);
        
        // init all piece attacks
        void init_piece_attacks();
        void init_sliding_attacks(int isBishop);


    private:
        // MAGIC NUMBERS AND BITBOARDS
        uint64_t find_magic_num(int square, int relevant_bits, int bishop);
        void init_magic_nums();
        
};

#endif