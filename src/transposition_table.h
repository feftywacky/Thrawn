#ifndef TRANSPOSITION_TABLE_H
#define TRANSPOSITION_TABLE_H

#include <cstdint>
#include <atomic>
#include "position.h"

#define no_hashmap_entry 100000

#define hashFlagEXACT 0
#define hashFlagALPHA 1
#define hashFlagBETA 2

/*
    We pack everything (depth, hash_flag, score, best_move) into 64 bits of data.
    'key' is stored in a separate 64-bit atomic.
*/
struct TranspositionTable
{
    // The Zobrist key for this entry (or partial key)
    std::atomic<uint64_t> key;

    // The packed data: [lowest bits for best_move][score][hash_flag][depth]
    std::atomic<uint64_t> data;


//    uint64_t key;  // unique chess position key
//    int depth;          // current search depth
//    int hash_flag;           // flag the type of node (fail-low/fail-high/PV)
//    int score;          // score (alpha/beta/PV)
//    int best_move;
};

extern int hashmap_len;
extern TranspositionTable* hashmap;

void init_hashmap(int mb);

void reset_hashmap();

int probeHashMap(thrawn::Position& pos, int depth, int alpha, int beta, int* bestMove);

void writeToHashMap(thrawn::Position& pos, int depth, int score, int hashFlag, int bestMove);

// helpers for atomic packing
inline uint64_t pack_data(int depth, int score, int hash_flag, int best_move);
inline void unpack_data(uint64_t data, int &depth, int &score, int &hash_flag, int &best_move);

#endif