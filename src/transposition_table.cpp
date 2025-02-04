#include "transposition_table.h"
#include "constants.h"  // for mateScore, etc.
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
        std::cerr << "TT init: table too small, forcing 1 MB.\n";
        initTable(1);
        return;
    }

    table = new TTEntry[numEntries];
    reset();

    std::cout << "TT: allocated " << mb << " MB, entries = " << numEntries << std::endl;
}

void TranspositionTable::reset()
{
    if (table && numEntries > 0) {
        std::memset(table, 0, numEntries * sizeof(TTEntry));
    }
    currentAge = 0; // Reset the age as well.
}

int TranspositionTable::probe(const thrawn::Position &pos, int depth, int alpha, int beta,
                                int &bestMove, int ply)
{
    if (!table || numEntries <= 0)
        return no_hashmap_entry;

    uint64_t key = pos.zobristKey;
    int index = static_cast<int>(key % numEntries);

    TTEntry entry = table[index];

    bestMove = entry.best_move;

    // Check for a matching key and sufficient search depth.
    if (entry.key != key || entry.depth < depth)
        return no_hashmap_entry;

    int score = entry.score;

    if (score < -mateScore) 
        score += ply;
    if (score > mateScore) 
        score -= ply;

    if (entry.hash_flag == hashFlagEXACT)
        return score;
    if (entry.hash_flag == hashFlagALPHA && score <= alpha)
        return alpha;
    if (entry.hash_flag == hashFlagBETA  && score >= beta)
        return beta;

    return no_hashmap_entry;
}

void TranspositionTable::store(const thrawn::Position &pos, int depth, int score,
                                 int flag, int bestMove, int ply)
{
    if (!table || numEntries <= 0)
        return;

    uint64_t key = pos.zobristKey;
    int index = static_cast<int>(key % numEntries);

    // Replacement logic: Replace if the entry is unused or if its stored age is older,
    // or if the stored search depth is less than or equal to the new depth.
    bool replace = false;
    if (table[index].key == 0)
        replace = true;
    else if (table[index].age < currentAge || table[index].depth <= depth)
        replace = true;

    if (!replace)
        return;

    // Adjust mate scores using ply if necessary.
    if (score < -mateScore)
        score -= ply;
    if (score > mateScore)
        score += ply;

    table[index].key = key;
    table[index].depth = depth;
    table[index].score = score;
    table[index].hash_flag = flag;
    table[index].best_move = bestMove;
    table[index].age = currentAge;
}
