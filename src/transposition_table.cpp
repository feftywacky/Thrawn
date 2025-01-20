#include "transposition_table.h"
#include "zobrist_hashing.h"
#include "search.h"
#include <iostream>

int hashmap_len = 0;
TranspositionTable* hashmap = nullptr;

/*
 * reset_hashmap:
 * Clears every entry in the table.
 */
void reset_hashmap()
{
    for (int i = 0; i < hashmap_len; i++)
    {
        hashmap[i].key       = 0ULL;
        hashmap[i].depth     = 0;
        hashmap[i].hash_flag = 0;
        hashmap[i].score     = 0;
        hashmap[i].best_move = 0;
    }
}

/*
 * init_hashmap:
 * Allocate a table in memory of size 'mb' MB.
 * Then calls reset_hashmap() to clear it.
 */
void init_hashmap(int mb)
{
    int size = 0x100000 * mb; // # bytes

    hashmap_len = size / sizeof(TranspositionTable);

    if (hashmap != nullptr)
    {
        std::cout << "hashmp memory cleared" << std::endl;
        delete[] hashmap;
    }

    hashmap = new TranspositionTable[hashmap_len];

    if (hashmap == nullptr)
    {
        std::cout << "cannot allocate memory for hashmap, re-allocating with " << mb/2 << " MB" << std::endl;
        init_hashmap(mb/2);
    }
    else
    {
        reset_hashmap();
        std::cout << "hashmap sucessfully initialized with size " << mb << " MB" << std::endl;
    }
}

/*
 * probeHashMap:
 * Lockless read. We do key % hashmap_len
 * to find the relevant entry. If it matches
 * our Zobrist key, we check if the depth is
 * good enough, then see if we can return a 
 * stored score or bestMove.
 */
int probeHashMap(thrawn::Position& pos, int depth,
                 int alpha, int beta, int* best_move, int ply)
{
    TranspositionTable* hashEntryPtr = &hashmap[pos.zobristKey % hashmap_len];

    if (hashEntryPtr->key == pos.zobristKey)
    {
        if (hashEntryPtr->depth >= depth)
        {
            int score = hashEntryPtr->score;

            // handle mate-scores so we can re-adjust them by ply
            if (score < -mateScore) 
                score += ply;
            if (score > mateScore) 
                score -= ply;

            if (hashEntryPtr->hash_flag == hashFlagEXACT)
                return score;
            if (hashEntryPtr->hash_flag == hashFlagALPHA && score <= alpha)
                return alpha;
            if (hashEntryPtr->hash_flag == hashFlagBETA && score >= beta)
                return beta;
        }
        // if we can't return a score, we at least update best_move
        *best_move = hashEntryPtr->best_move;
    }
    return no_hashmap_entry;
}

/*
 * writeToHashMap:
 * Overwrite the entry. Usually we do so if the new depth is >= old depth,
 * or we do an always-replace approach. 
 * We also store mate-scores with an offset so we can recover it.
 */
void writeToHashMap(thrawn::Position& pos, int depth,
                    int score, int hashFlag, int bestMove, int ply)
{
    TranspositionTable* hashEntryPtr = &hashmap[pos.zobristKey % hashmap_len];

    if (score < -mateScore) score -= ply;
    if (score > mateScore)  score += ply;

    hashEntryPtr->key       = pos.zobristKey;
    hashEntryPtr->depth     = depth;
    hashEntryPtr->hash_flag = hashFlag;
    hashEntryPtr->score     = score;
    hashEntryPtr->best_move = bestMove;
}
