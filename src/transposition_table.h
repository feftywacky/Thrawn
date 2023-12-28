#ifndef TRANSPOSITION_TABLE_H
#define TRANSPOSITION_TABLE_H
#include <cstdint>

#define no_hashmap_entry 100000

#define hashFlagEXACT 0
#define hashFlagALPHA 1
#define hashFlagBETA 2

extern int hashmap_len;

struct TranspositionTable
{
    uint64_t key;  // unique chess position key
    int depth;          // current search depth
    int hash_flag;           // flag the type of node (fail-low/fail-high/PV) 
    int score;          // score (alpha/beta/PV)
    int best_move;
};

void init_hashmap(int mb);

void reset_hashmap();

int probeHashMap(int depth, int alpha, int beta, int* bestMove);

void writeToHashMap(int depth, int score, int hashFlag, int bestMove);

#endif