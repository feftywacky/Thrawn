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
 * Thread:
 *  A convenience class that groups together the per-thread state:
 *    - a local copy of the root position
 *    - the local search data (ThreadData)
 */
class SMP_Thread {
public:
    thrawn::Position rootPos;
    ThreadData td;

    // Default constructor (required for stack allocation).
    SMP_Thread();

    // Parameterized constructor that initializes the root position.
    SMP_Thread(const thrawn::Position* pos);
};

/*
 * An atomic flag to signal threads to stop (time up, etc.)
 */
extern std::atomic<bool> stop_threads;

/*
 * Worker thread function:
 *  Each thread receives a pointer to its Thread object, so it can use its own copy
 *  of the position (rootPos) and search data (td).
 */
void smp_worker_thread_func(SMP_Thread* threadObj, int threadID, int maxDepth);

/*
 * Threaded search entry point:
 *  Creates numThreads Thread objects (each with its own copy of the root position)
 *  and spawns a worker thread for each.
 */
void search_position_threaded(const thrawn::Position* pos, int depth, int numThreads);

#endif // THREADING_H
