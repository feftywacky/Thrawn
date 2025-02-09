#include "transposition_table.h"
#include "constants.h"
#include <cstring>
#include <iostream>

TranspositionTable::TranspositionTable()
    : table(nullptr), numEntries(0), currentAge(0)
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
    int bytes = mb * 1024 * 1024;  // Convert MB to bytes
    numEntries = bytes / sizeof(TTEntry);

    if (table) {
        delete[] table;
        table = nullptr;
    }

    if (numEntries < 1) {
        std::cerr << "TT init: table too small, forcing 4 MB.\n";
        initTable(4);
        return;
    }

    table = new TTEntry[numEntries];
    reset();

    std::cout << "TT: allocated " << mb << " MB, entries = " << numEntries << std::endl;
}

void TranspositionTable::reset()
{
    if (table && numEntries > 0) {
        memset(table, 0, numEntries * sizeof(TTEntry));
    }
    currentAge = 0;
}

int TranspositionTable::probe(const thrawn::Position* pos, int depth, int alpha, int beta, int &bestMove, int ply)
{
    int index = static_cast<int>(pos->zobristKey % numEntries);

    TTEntry entry = table[index];

    uint64_t test_key = pos->zobristKey ^ entry.smp_data;

    if(test_key == entry.smp_key)
    {
        int entry_depth = extractTTDepth(entry.smp_data);
        int entry_best_move = extractTTBestMove(entry.smp_data);
        int entry_score = extractTTScore(entry.smp_data);
        int entry_hash_flag = extractTTHashFlag(entry.smp_data);

        if(entry_depth >= depth)
        {
            int score = entry_score;

            if (score < -mateScore)
                score += ply;
            if (score > mateScore) 
                score -= ply;

            if (entry_hash_flag == hashFlagEXACT) // pv node
                return score;
            if (entry_hash_flag == hashFlagALPHA && score <= alpha) // fail-low score
                return alpha;
            if (entry_hash_flag == hashFlagBETA && score >= beta) // fail-high score
                return beta;
        }

        bestMove = entry_best_move;
    }
    return no_hashmap_entry;
}

void TranspositionTable::store(const thrawn::Position* pos, int depth, int score, int flag, int bestMove, int ply)
{
    int index = static_cast<int>(pos->zobristKey % numEntries);
    TTEntry &entry = table[index];

    bool shouldReplace = false;
    if (entry.smp_data == 0)
    {
        shouldReplace = true;
    }
    else
    {
        if (currentAge > entry.age)
            shouldReplace = true;
        else if ( depth >= extractTTDepth(entry.smp_data))
            shouldReplace = true;
    }

    if (!shouldReplace)
        return;

    if (score < -mateScore) score -= ply;
    if (score > mateScore) score += ply;

    uint64_t data = encodeTTData(bestMove,depth,score,flag);
    uint64_t key = pos->zobristKey ^ data;
    
    table[index].smp_key = key;
    table[index].smp_data = data;
    entry.age = currentAge;
}

// bit allocations:
// best_move: 24 bits (mask: 0xFFFFFF)
// depth:      16 bits (mask: 0xFFFF)
// score:      17 bits (mask: 0x1FFFF) after adding an offset of 50000
// hash_flag:   2 bits (mask: 0x3)
//
// Note: Score is encoded as score + INFINITY so that the range -50000...+50000 becomes 0...100000.

uint64_t TranspositionTable::encodeTTData(int best_move, int depth, int score, int hash_flag) {
    // Offset the score to make it non-negative.
    int encoded_score = score + INFINITY; // now in the range 0 .. 100000

    uint64_t data = 0;
    data |= ((uint64_t)best_move & 0xFFFFFFULL);                // bits 0-23: best_move (24 bits)
    data |= (((uint64_t)depth & 0xFFFFULL) << 24);                // bits 24-39: depth (16 bits)
    data |= (((uint64_t)encoded_score & 0x1FFFFULL) << 40);       // bits 40-56: score (17 bits)
    data |= (((uint64_t)hash_flag & 0x3ULL) << 57);               // bits 57-58: hash_flag (2 bits)
    return data;
}

int TranspositionTable::extractTTBestMove(uint64_t data) {
    // Extract bits 0-23.
    return (int)(data & 0xFFFFFFULL);
}

int TranspositionTable::extractTTDepth(uint64_t data) {
    // Extract bits 24-39.
    return (int)((data >> 24) & 0xFFFFULL);
}

int TranspositionTable::extractTTScore(uint64_t data) {
    // Extract bits 40-56 then remove the offset.
    int encoded_score = (int)((data >> 40) & 0x1FFFFULL);
    return encoded_score - INFINITY;
}

int TranspositionTable::extractTTHashFlag(uint64_t data) {
    // Extract bits 57-58.
    return (int)((data >> 57) & 0x3ULL);
}