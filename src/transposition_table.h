#ifndef TRANSPOSITION_TABLE_H
#define TRANSPOSITION_TABLE_H

#include <cstdint>
#include "position.h"

// Constants for the TT.
static const int no_hashmap_entry = 100000;  // Sentinel for "TT miss"
static const int hashFlagEXACT    = 0;
static const int hashFlagALPHA    = 1;
static const int hashFlagBETA     = 2;

struct TTEntry 
{
    uint64_t key;    // The Zobrist key for the position.
    int depth;       // Search depth at which this entry was stored.
    int score;       // Evaluation score.
    int hash_flag;   // TT flag (EXACT, ALPHA, BETA)
    int best_move;   // Best move (for move ordering)
    int age;         // Age for replacement logic
};

class TranspositionTable
{
public:
    TranspositionTable();
    ~TranspositionTable();

    // Initialize or resize the table to 'mb' megabytes.
    void initTable(int mb);

    // Clears all entries and resets the current age.
    void reset();

    // Increments the current age (to be called at the start of a new search)
    void incrementAge() { currentAge++; }

    // Lookup a position in the TT.
    int probe(const thrawn::Position &pos, int depth, int alpha, int beta,
              int &bestMove, int ply);

    // Store an entry in the TT.
    void store(const thrawn::Position &pos, int depth, int score,
               int flag, int bestMove, int ply);

private:
    TTEntry* table;      // Array of TT entries.
    int      numEntries; // Number of entries in the table.
    int      currentAge; // Current age, updated once per search.
};

#endif // TRANSPOSITION_TABLE_H
