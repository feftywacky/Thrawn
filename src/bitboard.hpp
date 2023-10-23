#ifndef BITBOARD_HPP
#define HITBOARD_HPP

#include <cstdint>
#include <iostream>
#include <map>
#include <set>
#include <vector>

using namespace std;

class Bitboard
{
    public:

        // contructor
        Bitboard();


        void init();
        void printBB();
        void pprint_bb(uint64_t bitboard, int board_size=8);
        void pprint_pieces(map<char, set<int>> piece_map, int board_size=8);


        vector<int> get_squares_from_bb(uint16_t bitboard);  
        

        // get bitboards methods
        uint64_t get_white_pieces();
        uint64_t get_black_pieces();
              
    
    private:
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
};

#endif