#ifndef TRANSPOSITION_TABLE_H
#define TRANSPOSITION_TABLE_H
#include <cstdint>
#include "position.h"

/*
  We define some constants for the hash entry flags,
  along with a sentinel for no entry found.
*/
#define no_hashmap_entry 100000
#define hashFlagEXACT 0
#define hashFlagALPHA 1
#define hashFlagBETA 2

/*
 * Each TT entry stores:
 *   - key (64-bit Zobrist)
 *   - depth
 *   - node type (EXACT, ALPHA, BETA)
 *   - score
 *   - best_move (for move ordering hints)
 */
struct TranspositionTable
{
    uint64_t key;     // unique chess position key
    int depth;        // current search depth
    int hash_flag;    // alpha, beta, or exact
    int score;        // the stored eval/score
    int best_move;    // best move from this position
};

extern int hashmap_len;
extern TranspositionTable* hashmap;

/*
 * Initialize the TT of size 'mb' MB
 */
void init_hashmap(int mb);

/*
 * Reset/clear TT
 */
void reset_hashmap();

/*
 * Probe the TT. If found a matching key
 * and depth is high enough, we can potentially
 * return an immediate score. Otherwise, we set
 * the best move pointer if available.
 */
int probeHashMap(thrawn::Position& pos, int depth,
                 int alpha, int beta, int* bestMove, int ply);

/*
 * Write an entry into the TT, storing
 * (depth, score, flag, bestMove).
 */
void writeToHashMap(thrawn::Position& pos, int depth,
                    int score, int hashFlag, int bestMove, int ply);

#endif
