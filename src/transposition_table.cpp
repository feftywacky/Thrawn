#include "transposition_table.h"
#include "search.h"
#include <cstring>   // for memset
#include <iostream>

/*
 * A small offset so we can store signed scores in 16 bits, for example.
 * Adjust as you like. 
 */
static const int SCORE_OFFSET   = 49000; 
static const int DEPTH_MASK     = 0x3F;  // 6 bits => depth up to 63
static const int FLAGS_MASK     = 0x3;   // 2 bits => 0..3
static const int MOVE_MASK      = 0x1FFFFFF; // 25 bits of move

// Shift constants for packing
static const int SHIFT_DEPTH = 16; 
static const int SHIFT_FLAG  = 22;
static const int SHIFT_MOVE  = 24; 

TranspositionTable::TranspositionTable()
  : table(nullptr),
    numEntries(0)
{
}

TranspositionTable::~TranspositionTable()
{
    if (table) 
    {
        delete[] table;
        table = nullptr;
    }
}

void TranspositionTable::initTable(int mb)
{
    // Convert MB -> # of bytes
    int bytes = mb * 0x100000;
    // Each TTEntry is 16 bytes, so how many can we fit?
    numEntries = bytes / sizeof(TTEntry);

    // Clean up old
    if (table)
    {
        delete[] table;
        table = nullptr;
    }

    if (numEntries < 1)
    {
        std::cerr << "TT init: too small, forcing 1MB.\n";
        initTable(1);
        return;
    }

    table = new TTEntry[numEntries];
    reset();

    std::cout << "TT: allocated " << mb << " MB, entries = "
              << numEntries << std::endl;
}

void TranspositionTable::reset()
{
    curr_hash_age = 0;
    if (table && numEntries > 0)
        std::memset(table, 0, numEntries * sizeof(TTEntry));
}

/*
 * encodeData:
 *  Pack {score, depth, flags, move} into a single 64-bit.
 *  Example layout:
 *     bits  0..15 : (score + SCORE_OFFSET)
 *     bits 16..21 : depth (6 bits)
 *     bits 22..23 : flags (2 bits)
 *     bits 24..48 : move (25 bits)
 *  (You can tweak these as needed.)
 */
uint64_t TranspositionTable::encodeData(int depth, int score,
                                        int hashFlag, int bestMove,
                                        int ply)
{
    // Shift mate scores by ply so mates are distance-aware
    if (score >  mateScore)  score +=  ply; 
    if (score < -mateScore)  score -=  ply;

    // Make sure score fits in 16 bits
    int shiftedScore = score + SCORE_OFFSET;
    if (shiftedScore < 0) shiftedScore = 0;
    if (shiftedScore > 0xFFFF) shiftedScore = 0xFFFF;

    uint64_t data = 0ULL;
    data |= (uint64_t)(shiftedScore & 0xFFFF);         // bits 0..15
    data |= (uint64_t)(depth & DEPTH_MASK) << SHIFT_DEPTH;  // bits 16..21
    data |= (uint64_t)(hashFlag & FLAGS_MASK) << SHIFT_FLAG; // bits 22..23
    data |= (uint64_t)(bestMove & MOVE_MASK) << SHIFT_MOVE;  // bits 24+

    return data;
}

/*
 * decodeData:
 *  Reverse the above packing to retrieve
 *  {depth, score, flags, bestMove}.
 *  We'll re-adjust mate scores by ply at the time we use them, if needed.
 */
void TranspositionTable::decodeData(uint64_t smp_data,
                                    int &depth, int &score,
                                    int &hashFlag, int &bestMove)
{
    int rawScore   = (int)(smp_data & 0xFFFF);
    depth          = (int)((smp_data >> SHIFT_DEPTH) & DEPTH_MASK);
    hashFlag       = (int)((smp_data >> SHIFT_FLAG)  & FLAGS_MASK);
    bestMove       = (int)((smp_data >> SHIFT_MOVE)  & MOVE_MASK);

    score = rawScore - SCORE_OFFSET;
}

/*
 * probe:
 *  1) find index = key % numEntries
 *  2) read the TTEntry
 *  3) check if (entry.xor_key ^ entry.smp_data) == key
 *      => if not, return no_hashmap_entry
 *  4) decode the data
 *  5) if storedDepth >= depth, check bounding and possibly return a score
 *  6) otherwise, fill bestMove (for ordering) and return no_hashmap_entry
 */
int TranspositionTable::probe(const thrawn::Position &pos,
                              int depth, int alpha, int beta,
                              int &bestMove, int ply)
{
    if (!table || numEntries <= 0)
        return no_hashmap_entry;

    uint64_t key   = pos.zobristKey;
    int index      = (int)(key % numEntries);

    // Lockless read of the entry
    TTEntry entry  = table[index];

    // XOR check
    if ((entry.xor_key ^ entry.smp_data) != key || entry.smp_data == 0ULL)
    {
        // mismatch => no valid entry
        return no_hashmap_entry;
    }

    // decode data
    int storedDepth, storedScore, storedFlags, storedMove;
    decodeData(entry.smp_data, storedDepth, storedScore, storedFlags, storedMove);

    // If the entry is deep enough for us
    if (storedDepth >= depth)
    {
        // Re-adjust mate scoring by ply
        if (storedScore < -mateScore) storedScore += ply;
        else if (storedScore > mateScore) storedScore -= ply;

        // bounding
        if (storedFlags == hashFlagEXACT)
            return storedScore;
        if (storedFlags == hashFlagALPHA && storedScore <= alpha)
            return alpha;
        if (storedFlags == hashFlagBETA  && storedScore >= beta)
            return beta;
    }

    // Otherwise, just fill bestMove if we can:
    bestMove = storedMove;
    return no_hashmap_entry;
}

/*
 * store:
 *  "lockless XOR" write: xor_key = posKey ^ smp_data
 *  We also keep an 'age' in the TTEntry. We do a simple replacement scheme:
 *
 *   - if the entry is empty, or
 *   - if its age < newAge, or
 *   - if same age but oldDepth <= newDepth
 *     => store the new data
 *
 */
void TranspositionTable::store(const thrawn::Position &pos,
                               int depth, int score, int hashFlag,
                               int bestMove, int newAge, int ply)
{
    if (!table || numEntries <= 0)
        return;

    uint64_t key       = pos.zobristKey;
    uint64_t smp_data  = encodeData(depth, score, hashFlag, bestMove, ply);
    uint64_t xor_value = key ^ smp_data;

    int index = (int)(key % numEntries);

    // We do a lockless read of the old entry:
    TTEntry oldEntry = table[index];

    // decode the old data
    int oldDepth, oldScore, oldFlags, oldMove;
    decodeData(oldEntry.smp_data, oldDepth, oldScore, oldFlags, oldMove);

    // Replacement logic:
    // (If empty or older age, or same age but new depth is >= old depth.)
    if (oldEntry.smp_data == 0ULL ||           // empty
        oldEntry.age < newAge ||
       (oldEntry.age == newAge && oldDepth <= depth))
    {
        // Build the new TTEntry
        TTEntry newEntry;
        newEntry.xor_key  = xor_value;
        newEntry.smp_data = smp_data;
        newEntry.age      = newAge;

        table[index] = newEntry;
    }
}
