#ifndef THREADING_H
#define THREADING_H

#include <atomic>
#include <mutex>
#include "position.h"
#include "constants.h"

/*
 * ThreadData:
 *  Holds all per-thread arrays and data previously global:
 *    - PV arrays
 *    - Killer moves
 *    - History tables
 *    - Flags for move ordering (follow_pv_flag, etc.)
 */
class ThreadData {
public:
    // PV storage: one array for PV lengths and a 2D array for the PV lines.
    std::array<int, MAX_DEPTH> pv_depth;
    std::array<std::array<int, MAX_DEPTH>, MAX_DEPTH> pv_table;

    // Killer moves and history moves.
    std::array<std::array<int, MAX_DEPTH>, KILLER_MOVES> killer_moves;
    std::array<std::array<int, MAX_DEPTH>, HISTORY_SIZE> history_moves;

    // Flags used for move ordering and search heuristics.
    bool follow_pv_flag;
    bool score_pv_flag;
    bool allowNullMovePruning;

    // Constructor initializes all arrays to zero and flags to false (or true where needed).
    ThreadData();

    // Reset the thread data between searches.
    void resetThreadData();
};

/*
 * Worker thread function:
 *  Each thread receives a pointer to its Thread object, so it can use its own copy
 *  of the position (rootPos) and search data (td).
 */
void smp_worker_thread_func(thrawn::Position pos, int threadID, int maxDepth);

/*
 * Threaded search entry point:
 *  Creates numThreads Thread objects (each with its own copy of the root position)
 *  and spawns a worker thread for each.
 */
void search_position_threaded(thrawn::Position* pos, int depth, int numThreads);
void search_pos_single(thrawn::Position* pos, int depth);

#endif // THREADING_H
