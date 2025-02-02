#ifndef TRANSPOSITION_TABLE_H
#define TRANSPOSITION_TABLE_H

#include <cstdint>
#include "position.h"

// Constants for the TT.
static const int no_hashmap_entry = 100000;  // sentinel for "TT miss"
static const int hashFlagEXACT    = 0;
static const int hashFlagALPHA    = 1;
static const int hashFlagBETA     = 2;

extern int curr_hash_age;  // global TT age variable

// A TT entry storing the fields directly.
struct TTEntry 
{
    uint64_t key;    // The Zobrist key for the position.
    int depth;       // Search depth at which this entry was stored.
    int score;       // Evaluation score.
    int flag;        // TT flag (EXACT, ALPHA, BETA).
    int bestMove;    // Best move (for move ordering).
    int age;         // Age for replacement logic.
};

// A simple transposition table with thread-safe probe/store operations.
class TranspositionTable
{
public:
    TranspositionTable();
    ~TranspositionTable();

    // Initialize or resize the table to 'mb' megabytes.
    void initTable(int mb);

    // Clears all entries.
    void reset();

    // Lookup a position in the TT. Returns a score if the entry is valid, 
    // or no_hashmap_entry if not. If a best move was stored, it is written into bestMove.
    int probe(const thrawn::Position &pos, int depth, int alpha, int beta,
              int &bestMove, int ply);

    // Store an entry (pos, depth, score, flag, bestMove, age) in the TT.
    void store(const thrawn::Position &pos, int depth, int score,
               int flag, int bestMove, int newAge, int ply);

private:
    TTEntry*  table;      // Array of TT entries.
    int       numEntries; // Number of entries in the table.
};

#endif // TRANSPOSITION_TABLE_H
