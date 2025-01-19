#ifndef ZOBRIST_HASHING_H
#define ZOBRIST_HASHING_H

#include <cstdint>
#include <iostream>
#include <map>
#include <set>
#include <vector>
#include <array>
#include "position.h"


//extern uint64_t piece_hashkey[12][64]; // [piece][square]
//extern uint64_t enpassant_hashkey[64]; // [enpassant square]
//extern uint64_t castling_hashkey[16];  // [castle rights]
//extern uint64_t colour_to_move_hashkey;
//
//extern uint64_t zobristKey;

void init_hashkeys(thrawn::Position& pos);
uint64_t gen_hashkey(thrawn::Position& pos);


#endif 