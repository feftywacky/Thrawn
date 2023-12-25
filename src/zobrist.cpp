#include "zobrist.h"
#include "bitboard.h"
#include "bitboard_helpers.h"
#include <cstdint>
#include <iostream>
#include <map>
#include <set>
#include <vector>
#include <array>
#include <iomanip>


uint64_t piece_hashkey[12][64]; // [piece][square]
uint64_t enpassant_hashkey[64]; // [enpassant square]
uint64_t castling_hashkey[16]; // [castle rights]
uint64_t colour_to_move_hashkey;

uint64_t position_hashkey;

void init_hashkeys()
{
    random_state = 1804289383;
    // piece hashkey
    for (int piece = P; piece<=k; piece++)
    {
        for (int sq = 0; sq<64; sq++)
        {
            piece_hashkey[piece][sq] = get_random_U64();
        }
    }

    // enpassant hashkey
    for (int sq=0;sq<64;sq++)
    {
        enpassant_hashkey[sq] = get_random_U64();
    }

    // castling hashkey
    for (int i=0;i<16;i++)
    {
        castling_hashkey[i] = get_random_U64();
    }

    colour_to_move_hashkey = get_random_U64();
}

uint64_t gen_hashkey()
{
    uint64_t hashkey = 0ULL;
    uint64_t bitboard;

    for (int piece=P;piece<=k;piece++)
    {
        bitboard = piece_bitboards[piece];
        while(bitboard)
        {
            int sq = get_lsb_index(bitboard);
            hashkey ^= piece_hashkey[piece][sq];
            pop_bit(bitboard, sq);

        }
    }

    if (enpassant!=null_sq)
        hashkey ^= enpassant_hashkey[enpassant];
    
    hashkey ^= castling_hashkey[castle_rights];

    if (colour_to_move == black) 
        hashkey ^= colour_to_move_hashkey;
    
    return hashkey;
}