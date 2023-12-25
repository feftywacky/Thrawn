#ifndef TRANSPOSITION_TABLE_H
#define TRANSPOSITION_TABLE_H
#include <cstdint>

#define hashmap_size 0x800000 // size of transposition table (4MB)
#define no_hashmap_entry 100000

#define hashFlagEXACT 0
#define hashFlagALPHA 1
#define hashFlagBETA 2

struct TranspositionTable
{
    uint64_t key;  // unique chess position key
    int depth;          // current search depth
    int hash_flag;           // flag the type of node (fail-low/fail-high/PV) 
    int score;          // score (alpha/beta/PV)
};

void reset_hashmap();

int probeHashMap(int depth, int alpha, int beta);

void writeToHashMap(int depth, int score, int hashFlag);

#endif