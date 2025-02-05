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
        for (int i = 0; i < numEntries; i++) {
            table[i].key       = 0ULL;
            table[i].depth     = 0;
            table[i].score     = 0;
            table[i].hash_flag = 0;     
            table[i].best_move = 0;      
            table[i].age       = 0;     
        }
    }
    currentAge = 0;
}

int TranspositionTable::probe(const thrawn::Position &pos, int depth, int alpha, int beta, int &bestMove, int ply)
{
    int index = static_cast<int>(pos.zobristKey % numEntries);

    TTEntry entry = table[index];

    if(entry.key == pos.zobristKey)
    {
        if(entry.depth >= depth)
        {
            int score = entry.score;

            if (score < -mateScore)
                score += ply;
            if (score > mateScore) 
                score -= ply;

            if (entry.hash_flag == hashFlagEXACT) // pv node
                return score;
            if (entry.hash_flag == hashFlagALPHA && score <= alpha) // fail-low score
                return alpha;
            if (entry.hash_flag == hashFlagBETA && score >= beta) // fail-high score
                return beta;
        }
    }
    bestMove = entry.best_move;

    return no_hashmap_entry;
}

void TranspositionTable::store(const thrawn::Position &pos, int depth, int score, int flag, int bestMove, int ply)
{
    int index = static_cast<int>(pos.zobristKey % numEntries);

    if (score < -mateScore) score -= ply;
    if (score > mateScore) score += ply;

    table[index].key = pos.zobristKey;
    table[index].depth = depth;
    table[index].score = score;
    table[index].hash_flag = flag;
    table[index].best_move = bestMove;
    table[index].age = currentAge;
}
