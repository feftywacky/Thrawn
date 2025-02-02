#include "transposition_table.h"
#include "constants.h"
#include <cstring>
#include <iostream>
#include <mutex>

int curr_hash_age = 0;  // global TT age

// A global mutex protecting all transposition table accesses.
static std::mutex ttMutex;

TranspositionTable::TranspositionTable()
  : table(nullptr), numEntries(0)
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
    int bytes = mb * 1024 * 1024;  // convert MB to bytes
    numEntries = bytes / sizeof(TTEntry);

    if (table)
    {
        delete[] table;
        table = nullptr;
    }

    if (numEntries < 1)
    {
        std::cerr << "TT init: table too small, forcing 1 MB.\n";
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
    {
        std::lock_guard<std::mutex> lock(ttMutex);
        std::memset(table, 0, numEntries * sizeof(TTEntry));
    }
}

int TranspositionTable::probe(const thrawn::Position &pos, int depth, int alpha, int beta,
                              int &bestMove, int ply)
{
    if (!table || numEntries <= 0)
        return no_hashmap_entry;

    uint64_t key = pos.zobristKey;
    int index = (int)(key % numEntries);

    std::lock_guard<std::mutex> lock(ttMutex);
    TTEntry entry = table[index];

    if (entry.key != key)
        return no_hashmap_entry;

    // Only use the entry if it was searched deep enough.
    if (entry.depth < depth)
        return no_hashmap_entry;

    bestMove = entry.bestMove;

    if (entry.flag == hashFlagEXACT)
        return entry.score;
    if (entry.flag == hashFlagALPHA && entry.score <= alpha)
        return alpha;
    if (entry.flag == hashFlagBETA  && entry.score >= beta)
        return beta;

    return no_hashmap_entry;
}

void TranspositionTable::store(const thrawn::Position &pos, int depth, int score,
                               int flag, int bestMove, int newAge, int ply)
{
    if (!table || numEntries <= 0)
        return;

    uint64_t key = pos.zobristKey;
    int index = (int)(key % numEntries);

    std::lock_guard<std::mutex> lock(ttMutex);
    TTEntry &entry = table[index];

    // Replacement logic: always replace if the entry is empty, or if it is older,
    // or if the same age but the new search was as deep or deeper.
    if (entry.key == 0 || entry.age < newAge || (entry.age == newAge && entry.depth <= depth))
    {
        entry.key = key;
        entry.depth = depth;

        if (score < -mateScore) score -= ply;
        if (score > mateScore) score += ply;
        
        entry.score = score;
        entry.flag = flag;
        entry.bestMove = bestMove;
        entry.age = newAge;
    }
}
