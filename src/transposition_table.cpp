#include "transposition_table.h"
#include "zobrist_hashing.h"
#include "search.h"
#include <iostream>

int hashmap_len = 0;

TranspositionTable *hashmap = nullptr;

void reset_hashmap()
{
    //    TranspositionTable *hashval = nullptr;
    //
    //    for (hashval = hashmap; hashval<hashmap+hashmap_len; hashval++)
    //    {
    //        hashval->key = 0;
    //        hashval->depth = 0;
    //        hashval->hash_flag = 0;
    //        hashval->score = 0;
    //    }
    for (int i = 0; i < hashmap_len; i++)
    {
        hashmap[i].key.store(0ULL, std::memory_order_relaxed);
        hashmap[i].data.store(0ULL, std::memory_order_relaxed);
    }
}

void init_hashmap(int mb)
{
    int size = 0x100000 * mb;

    hashmap_len = size / sizeof(TranspositionTable);

    if (hashmap != nullptr)
    {
        std::cout<<"hashmp memory cleared"<<std::endl;
        delete[] hashmap;
    }

    hashmap = new TranspositionTable[hashmap_len];

    if (hashmap == nullptr)
    {
        std::cout<<"cannot allocate memory for hashmap, re-allocating with "<<mb/2<<" MB"<<std::endl;
        init_hashmap(mb/2);
    }
    else
    {
        reset_hashmap();
        std::cout<<"hashmap sucessfully initialized with size "<<mb<<" MB"<<std::endl;
    }
}

/*
    probeHashMap:
    1) Compute index = zobristKey % hashmap_len
    2) Lockless load of the entry's key
    3) If mismatch, return no_hashmap_entry
    4) If match, load the data, verify the key again
    5) If still matches, return the stored score (or alpha/beta boundary) as usual
*/
int probeHashMap(thrawn::Position& pos, int depth, int alpha, int beta, int *best_move)
{
    int index = (int)(pos.zobristKey % hashmap_len);

    // Step 1: Acquire the stored key with acquire semantics
    uint64_t storedKey = hashmap[index].key.load(std::memory_order_acquire);

    // if no entry or mismatch
    if (storedKey != pos.zobristKey)
        return no_hashmap_entry;

    // Step 2: load the data (relaxed is OK, but we must re-check the key later)
    uint64_t rawData = hashmap[index].data.load(std::memory_order_relaxed);

    // Step 3: re-check that the key didn’t change in between
    uint64_t storedKey2 = hashmap[index].key.load(std::memory_order_relaxed);
    if (storedKey2 != pos.zobristKey)
        return no_hashmap_entry;

    // Step 4: unpack
    int ttDepth, ttScore, ttFlag, ttBestMove;
    unpack_data(rawData, ttDepth, ttScore, ttFlag, ttBestMove);

    // If we do not satisfy depth requirement, skip
    if (ttDepth < depth)
    {
        *best_move = ttBestMove;  // might still be useful for move ordering
        return no_hashmap_entry;
    }

    // Adjust mate scores
    if (ttScore < -mateScore) ttScore += ply;
    if (ttScore >  mateScore) ttScore -= ply;

    // We can interpret EXACT, ALPHA, BETA
    if (ttFlag == hashFlagEXACT)
    {
        *best_move = ttBestMove;
        return ttScore;
    }
    else if (ttFlag == hashFlagALPHA && ttScore <= alpha)
    {
        *best_move = ttBestMove;
        return alpha;
    }
    else if (ttFlag == hashFlagBETA && ttScore >= beta)
    {
        *best_move = ttBestMove;
        return beta;
    }

    *best_move = ttBestMove;
    return no_hashmap_entry;

//    // tt instance pointer that points to the hashmap entry that stores board data
//    // hash function key is defined as key % size
//    TranspositionTable *hashEntryPtr = &hashmap[zobristKey % hashmap_len];
//
//    if (hashEntryPtr->key == zobristKey)
//    {
//        if (hashEntryPtr->depth >= depth)
//        {
//            int score = hashEntryPtr->score;
//
//            if (score < -mateScore)
//                score += ply;
//            if (score > mateScore)
//                score -= ply;
//
//            if (hashEntryPtr->hash_flag == hashFlagEXACT) // pv node
//                return score;
//            if (hashEntryPtr->hash_flag == hashFlagALPHA && score <= alpha) // fail-low score
//                return alpha;
//            if (hashEntryPtr->hash_flag == hashFlagBETA && score >= beta) // fail-high score
//                return beta;
//        }
//
//        *best_move = hashEntryPtr->best_move;
//    }
//    return no_hashmap_entry;
}

/*
    writeToHashMap:
    1) index = zobristKey % hashmap_len
    2) load the old key (relaxed)
    3) either overwrite or not, based on replacement scheme (e.g., always replace).
    4) store the new key (relaxed)
    5) store the new data (release)
*/
void writeToHashMap(thrawn::Position& pos, int depth, int score, int hashFlag, int bestMove)
{
    // boundary conditions for mate-scores
    if (score < -mateScore) score -= ply;
    if (score >  mateScore) score += ply;

    uint64_t index = pos.zobristKey % hashmap_len;
    uint64_t oldKey = hashmap[index].key.load(std::memory_order_relaxed);

    // Some custom replacement strategy can go here:
    // if (oldKey != 0ULL && oldKey != zobristKey)
    //     maybe compare depths or something similar ...
    // For now, do a simple always-overwrite:

    // Pack the data
    uint64_t rawData = pack_data(depth, score, hashFlag, bestMove);

    // Step 1: store the key
    hashmap[index].key.store(pos.zobristKey, std::memory_order_relaxed);
    // Step 2: store data with release semantics
    hashmap[index].data.store(rawData, std::memory_order_release);
//    TranspositionTable *hashEntryPtr = &hashmap[zobristKey % hashmap_len];
//
//    if (score < -mateScore) score -= ply;
//    if (score > mateScore) score += ply;
//
//
//    hashEntryPtr->key = zobristKey;
//    hashEntryPtr->depth = depth;
//    hashEntryPtr->hash_flag = hashFlag;
//    hashEntryPtr->score = score;
//    hashEntryPtr->best_move = bestMove;
}

// We pack: [ depth(16 bits) | hash_flag(8 bits) | best_move(16 bits) | score(16 bits) | unused(8 bits) ]
inline uint64_t pack_data(int depth, int score, int hash_flag, int best_move) {
    // clamp inputs into the bit ranges you allow
    // for example, depth cannot exceed 65535, etc.
    // Here, we do something simplistic:
    uint64_t d = (uint64_t)(depth & 0xFFFF);
    uint64_t sc = (uint64_t)((score + 0x7FFF) & 0xFFFF); // shift score so it’s never negative in 16 bits
    uint64_t hf = (uint64_t)(hash_flag & 0xFF);
    uint64_t bm = (uint64_t)(best_move & 0xFFFF);

    // pack
    //   bits 0..15   = best_move
    //   bits 16..31  = score
    //   bits 32..39  = hash_flag
    //   bits 40..55  = depth
    //   bits 56..63  = unused
    uint64_t data = 0ULL;
    data |= bm;
    data |= (sc << 16);
    data |= (hf << 32);
    data |= (d  << 40);
    return data;
}

inline void unpack_data(uint64_t data, int &depth, int &score, int &hash_flag, int &best_move) {
    best_move       = (int)(data & 0xFFFF);
    score           = (int)((data >> 16) & 0xFFFF);
    // shift score back:
    score           -= 0x7FFF;
    hash_flag       = (int)((data >> 32) & 0xFF);
    depth           = (int)((data >> 40) & 0xFFFF);
}
