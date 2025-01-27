#ifndef TRANSPOSITION_TABLE_H
#define TRANSPOSITION_TABLE_H

#include <cstdint>
#include "position.h"

/*
  Constants for the TT use
*/
static const int no_hashmap_entry = 100000;  // sentinel for "TT miss"
static const int hashFlagEXACT    = 0;
static const int hashFlagALPHA    = 1;
static const int hashFlagBETA     = 2;

extern int curr_hash_age;

/*
 * Each TT entry has two 64-bit fields used in the "lockless XOR" approach:
 *   - xor_key = pos.zobristKey ^ smp_data
 *   - smp_data = a packed encoding of {score, depth, flags, move, etc.}
 *
 * We store 'age' as a separate int.  That is *not* part of the XOR packing.
 */
struct TTEntry 
{
    uint64_t xor_key;   // = pos.zobristKey ^ smp_data
    uint64_t smp_data;  // packed {score, depth, flags, move, etc.}
    int      age;       // used for "age" replacement logic
};

/*
 * The transposition table is an array of TTEntry plus
 * helper functions to init, store, probe, etc.
 */
class TranspositionTable
{
public:
    TranspositionTable();
    ~TranspositionTable();

    // Create or resize the TT to 'mb' megabytes
    void initTable(int mb);

    // Clears all entries
    void reset();

    // Lookup a position in TT. Returns an eval or 'no_hashmap_entry'
    // If found, may fill bestMove if stored. 
    int probe(const thrawn::Position &pos, int depth, int alpha, int beta,
              int &bestMove, int ply);

    // Store (pos, depth, score, flag, bestMove), obeying age replacement
    void store(const thrawn::Position &pos, int depth, int score,
               int hashFlag, int bestMove, int age, int ply);

private:
    // Helper to pack score/depth/flag/move into smp_data
    uint64_t encodeData(int depth, int score, int hashFlag,
                        int bestMove, int ply);

    // Helper to decode smp_data
    void decodeData(uint64_t smp_data,
                    int &depth, int &score, int &hashFlag, int &bestMove);

private:
    TTEntry*  table;      // pointer to TTEntry array
    int       numEntries; // length of 'table'
};

#endif // TRANSPOSITION_TABLE_H
