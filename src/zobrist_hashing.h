#ifndef ZOBRIST_HASHING_H
#define ZOBRIST_HASHING_H

#include <cstdint>
#include <iostream>
#include <map>
#include <set>
#include <vector>
#include <array>


extern uint64_t piece_hashkey[12][64]; // [piece][square]
extern uint64_t enpassant_hashkey[64]; // [enpassant square]
extern uint64_t castling_hashkey[16];  // [castle rights]
extern uint64_t colour_to_move_hashkey;

extern uint64_t position_hashkey;

void init_hashkeys();
uint64_t gen_hashkey();


#endif 