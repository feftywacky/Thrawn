#include "transposition_table.h"
#include "search.h"
#include <cstring>
#include <iostream>

// We use 17 bits for the score to avoid clamping big mate scores
static const int SCORE_BITS     = 17;
static const int SCORE_MASK     = (1 << SCORE_BITS) - 1; // 0x1FFFF
static const int SCORE_OFFSET   = 65536;  // for shifting negative scores
                                          // ( if you want max = ±49k, you can set offset=65k )

// 6 bits for depth => up to 63
static const int DEPTH_BITS = 6;
static const int DEPTH_MASK = (1 << DEPTH_BITS) - 1; // 0x3F

// 2 bits for flags => up to 3
static const int FLAGS_BITS = 2;
static const int FLAGS_MASK = (1 << FLAGS_BITS) - 1; // 0x3

// 25 bits for moves
static const int MOVE_BITS = 25;
static const int MOVE_MASK = (1 << MOVE_BITS) - 1; // 0x1FFFFFF

// Shift constants
static const int SHIFT_SCORE = 0;
static const int SHIFT_DEPTH = SCORE_BITS; // 17
static const int SHIFT_FLAG  = SCORE_BITS + DEPTH_BITS; // 23
static const int SHIFT_MOVE  = SCORE_BITS + DEPTH_BITS + FLAGS_BITS; // 25

// The global TT age
int curr_hash_age = 0;

TranspositionTable::TranspositionTable()
  : table(nullptr),
    numEntries(0)
{
}

TranspositionTable::~TranspositionTable()
{
    if (table) {
        delete[] table;
        table = nullptr;
    }
}

void TranspositionTable::initTable(int mb)
{
    int bytes = mb * 0x100000; // MB => #bytes
    numEntries = bytes / sizeof(TTEntry);

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
        std::fill(table, table + numEntries, TTEntry{});
}

/*
 * encodeData:
 *  We store 17 bits for (score + OFFSET),
 *  6 bits for depth,
 *  2 bits for flag,
 *  25 bits for bestMove.
 */
uint64_t TranspositionTable::encodeData(int depth, int score,
                                        int hashFlag, int bestMove,
                                        int ply)
{
    // Adjust mate scores by ply for distance to mate
    if (score >  mateScore) score += ply; 
    if (score < -mateScore) score -= ply;

    // clamp the score into [-65536, +65536] just in case
    if (score < -65536) score = -65536;
    if (score >  65536) score =  65536;

    // shift by offset to store in 17 bits
    int shiftedScore = score + SCORE_OFFSET;
    if (shiftedScore < 0) shiftedScore = 0;
    if (shiftedScore > SCORE_MASK) shiftedScore = SCORE_MASK;

    uint64_t data = 0ULL;

    // bits 0..16 => score
    data |= (uint64_t)(shiftedScore & SCORE_MASK) << SHIFT_SCORE;

    // bits 17..22 => depth
    data |= (uint64_t)(depth & DEPTH_MASK) << SHIFT_DEPTH;

    // bits 23..24 => flag
    data |= (uint64_t)(hashFlag & FLAGS_MASK) << SHIFT_FLAG;

    // bits 25..49 => bestMove
    data |= (uint64_t)(bestMove & MOVE_MASK) << SHIFT_MOVE;

    return data;
}

/*
 * decodeData:
 *  Reverse the above packing.
 */
void TranspositionTable::decodeData(uint64_t smp_data,
                                    int &depth, int &score,
                                    int &hashFlag, int &bestMove)
{
    // retrieve raw fields
    int rawScore   = (int)((smp_data >> SHIFT_SCORE) & SCORE_MASK);
    depth          = (int)((smp_data >> SHIFT_DEPTH) & DEPTH_MASK);
    hashFlag       = (int)((smp_data >> SHIFT_FLAG)  & FLAGS_MASK);
    bestMove       = (int)((smp_data >> SHIFT_MOVE)  & MOVE_MASK);

    // convert rawScore back to signed
    score = rawScore - SCORE_OFFSET;
}

int TranspositionTable::probe(const thrawn::Position &pos,
                              int depth, int alpha, int beta,
                              int &bestMove, int ply)
{
    if (!table || numEntries <= 0)
        return no_hashmap_entry;

    uint64_t key   = pos.zobristKey;
    int index      = (int)(key % numEntries);

    // lockless read
    TTEntry entry  = table[index];

    if (entry.smp_data == 0ULL)
        return no_hashmap_entry;

    // check if valid
    if ((entry.xor_key ^ entry.smp_data) != key)
        return no_hashmap_entry;

    // decode
    int storedDepth, storedScore, storedFlags, storedMove;
    decodeData(entry.smp_data, storedDepth, storedScore, storedFlags, storedMove);

    // re-adjust mate scoring
    if (storedScore < -mateScore) storedScore += ply;
    else if (storedScore >  mateScore) storedScore -= ply;

    // is it deep enough?
    if (storedDepth >= depth)
    {
        // bounding
        if (storedFlags == hashFlagEXACT)
            return storedScore;
        if (storedFlags == hashFlagALPHA && storedScore <= alpha)
            return alpha;
        if (storedFlags == hashFlagBETA  && storedScore >= beta)
            return beta;
    }

    // just return the bestMove for ordering if stored
    bestMove = storedMove;
    return no_hashmap_entry;
}

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

    // old entry
    TTEntry old = table[index];

    // decode old data
    int oldDepth, oldScore, oldFlags, oldMove;
    if (old.smp_data != 0ULL)
        decodeData(old.smp_data, oldDepth, oldScore, oldFlags, oldMove);
    else
        oldDepth = -1;

    // replacement logic: always store if empty or older age,
    // or same age but new depth >= old depth
    if (old.smp_data == 0ULL ||
        old.age < newAge ||
       (old.age == newAge && oldDepth <= depth))
    {
        TTEntry newEntry;
        newEntry.xor_key  = xor_value;
        newEntry.smp_data = smp_data;
        newEntry.age      = newAge;
        table[index] = newEntry;
    }
}
