#ifndef ENGINE_HPP
#define ENGINE_HPP

#include <cstdint>
#include <iostream>
#include <map>
#include <set>
#include <vector>
#include "bitboard.hpp"

using namespace std;


class Engine
{
public: 

    //constructor
    Engine();


    Bitboard board;

    std::vector<int> moves;

    void generate_moves();
    

    int make_move(int move, int move_type);
    
    

private:
    void parse_white_pawn_moves(uint64_t& curr);
    void parse_black_pawn_moves(uint64_t& curr);
    void parse_knight_moves(uint64_t& curr, const int& piece);
    void parse_bishop_moves(uint64_t& curr, const int& piece);
    void parse_rook_moves(uint64_t& curr, const int& piece);
    void parse_queen_moves(uint64_t& curr, const int& piece);
    void parse_king_moves(uint64_t& curr, const int& piece);
    void parse_white_castle_moves();
    void parse_black_castle_moves();

};

#endif