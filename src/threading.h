#ifndef THREADING_H
#define THREADING_H

#include <atomic>
#include <mutex>
#include "position.h"
#include "constants.h"

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

    long long nodes;

    // Constructor initializes all arrays to zero and flags to false (or true where needed).
    ThreadData();

    // Reset the thread data between searches.
    void resetThreadData();
};

void smp_worker_thread_func(thrawn::Position pos, int threadID, int maxDepth);

// search position entry point
void search_position_threaded(thrawn::Position* pos, int depth, int numThreads);

// for testing purposes
void search_pos_single(thrawn::Position* pos, int depth);

#endif // THREADING_H
