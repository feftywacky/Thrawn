#ifndef THREADING_H
#define THREADING_H

#include <atomic>
#include <vector>
#include <mutex>
#include "position.h"

/*
 * ThreadData:
 *  Holds all per-thread arrays and data that were previously global:
 *    - PV arrays
 *    - Killer moves
 *    - History tables
 *    - Repetition data
 *    - Flags for move ordering (follow_pv_flag, etc.)
 * 
 *  We use a class here for clarity; there's no performance issue 
 *  compared to a struct. The overhead is minimal compared to overall search.
 */
class ThreadData {
public:
    // =========== PV STORAGE =========== //
    std::vector<int> pv_depth;                       // PV length array
    std::vector<std::vector<int>> pv_table;          // PV lines

    // =========== KILLER / HISTORY =========== //
    std::vector<std::vector<int>> killer_moves;      // killer_moves[2][depth]
    std::vector<std::vector<int>> history_moves;     // history_moves[piece][square]

    // =========== REPETITION =========== //
    std::vector<uint64_t> repetition_table;          // records zobrist keys for repetition
    int repetition_index;
    int fifty_move;

    // =========== FLAGS FOR PV ORDERING =========== //
    bool follow_pv_flag;
    bool score_pv_flag;

    // Constructor
    ThreadData();
};

/*
 * Thread class:
 *  Each worker thread owns:
 *   - a local rootPos (copy of the main root position)
 *   - a local ThreadData object for storing search heuristics
 */
class Thread {
public:
    // The position copy for this thread
    thrawn::Position rootPos;

    // The local search data
    ThreadData td;

    // Constructor that copies the initial position
    Thread(const thrawn::Position &pos) : rootPos(pos), td() {}
};

/*
 * stop_threads: 
 *  an atomic flag for the threads to check if they should stop (time up, etc.)
 */
extern std::atomic<bool> stop_threads;

/*
 * search_position_threaded:
 *  The multi-threaded search entry point. 
 *  We pass the updated root position (from UCI) by reference, 
 *  then create 'numThreads' local copies for the workers.
 */
void search_position_threaded(const thrawn::Position &pos, int depth, int numThreads);

#endif // THREADING_H
