#ifndef BITBOARD_HPP
#define BITBOARD_HPP

#include <cstdint>
#include <iostream>
#include <map>
#include <set>
#include <vector>
#include "bitboard_functions.hpp"
#include "constants.hpp"

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

        uint64_t pawn_attacks[2][BOARD_SIZE];

        // contructor
        Bitboard();

        void printBB();


        // get bitboards methods
        uint64_t get_white_pieces();
        uint64_t get_black_pieces();     


        // pre-compute all attacks from a square methods

        // pawns
        uint64_t get_pawn_attack_from_sq(Side side, int square);
        void init_pawn_attacks();

    private:
        
};

#endif