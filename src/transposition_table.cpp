#include "transposition_table.h"
#include "zobrist_hashing.h"
#include "search.h"
#include <iostream>

TranspositionTable hashMap[hashmap_size];

void reset_hashmap()
{
    for (int i=0;i<hashmap_size;i++)
    {
        hashMap[i].key = 0ULL;
        hashMap[i].depth = 0;
        hashMap[i].hash_flag = 0;
        hashMap[i].score = 0;
    }
}

int probeHashMap(int depth, int alpha, int beta)
{
    // tt instance pointer that points to the hashmap entry that stores board data
    // hash function key is defined as key % size
    TranspositionTable *hashEntryPtr = &hashMap[zobristKey % hashmap_size]; 

    if (hashEntryPtr->key == zobristKey)
    {
        if (hashEntryPtr->depth >= depth)
        {
            int score = hashEntryPtr->score;

            if (score < -mateScore) score += ply;
            if (score > mateScore) score -= ply;

            if (hashEntryPtr->hash_flag == hashFlagEXACT) // pv node
                return score;
            if (hashEntryPtr->hash_flag == hashFlagALPHA && score <= alpha) // fail-low score
                return alpha;
            if (hashEntryPtr->hash_flag == hashFlagBETA && score >= beta) // fail-high score
                return beta;
        }
    }
    return no_hashmap_entry;
}

void writeToHashMap(int depth, int score, int hashFlag)
{
    TranspositionTable *hashEntryPtr = &hashMap[zobristKey % hashmap_size]; 

    if (score < -mateScore) score -= ply;
    if (score > mateScore) score += ply;
    

    hashEntryPtr->key = zobristKey;
    hashEntryPtr->depth = depth;
    hashEntryPtr->hash_flag = hashFlag;
    hashEntryPtr->score = score;
}
