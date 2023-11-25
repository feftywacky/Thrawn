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

    void generate_moves();
    int parse_move(int source, int target, int piece, int promoted_piece, int capture, int double_pawn_move, int enpassant, int castling);
    int get_move_source(int move);
    int get_move_target(int move);
    int get_move_piece(int move);
    int get_promoted_piece(int move);
    int get_is_cpature_move(int move);
    int get_is_double_pawn_move(int move);
    int get_is_move_enpassant(int move);
    int get_is_move_castling(int move);

private:
        

};

#endif