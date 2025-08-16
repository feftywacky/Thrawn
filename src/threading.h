#ifndef THREADING_H
#define THREADING_H

#include <atomic>
#include <mutex>
#include "position.h"
#include "constants.h"

class ThreadData {
public:
    // PV storage: one array for PV lengths and a 2D array for the PV lines.
    std::array<int, MAX_DEPTH> pv_length;
    std::array<std::array<int, MAX_DEPTH>, MAX_DEPTH> pv_table;

    // Killer moves and history moves.
    std::array<std::array<int, MAX_DEPTH>, KILLER_MOVES> killer_moves;
    std::array<std::array<int, MAX_DEPTH>, HISTORY_SIZE> history_moves;

    // Countermove heuristic: for each [piece][toSquare] of the previous move,
    // store the refuting move as a packed move int. Thread-local for safety.
    std::array<std::array<int, 64>, HISTORY_SIZE> counter_moves;
    // Track previous move per ply to address countermoves
    std::array<int, MAX_DEPTH> prev_move_at_ply;

    // Flags used for move ordering and search heuristics.
    bool follow_pv_flag;
    bool score_pv_flag;
    bool allowNullMovePruning;

    long long nodes;

    int final_depth;
    int final_score;

    // Constructor initializes all arrays to zero and flags to false (or true where needed).
    ThreadData();

    // Reset the thread data between searches.
    void resetThreadData();
};

void smp_worker_thread_func(thrawn::Position* pos, int threadID, int maxDepth);

// search position entry point
void search_position_threaded(thrawn::Position* pos, int depth, int numThreads);

ThreadData* select_best_thread(ThreadData threadDatas[], int numThreads);

#endif // THREADING_H
