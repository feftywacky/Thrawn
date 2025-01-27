#ifndef SEARCH_H
#define SEARCH_H

#include <vector>
#include <array>
#include <cstdint>
#include "position.h"
#include "threading.h" // for ThreadData

/*
some notes for negamax
3 types
- fail high: causes beta cut-off
- fail low: don't increase alpha
- pv nodes: increase alpha
*/

// global node counter
extern uint64_t nodes;

/*
 Late Move Reductions or pruning factors
*/
extern std::array<int, 4> LateMovePruning_factors;
extern int RFP_factor;

// ----------------------------------------
// Some constants used in the search
// ----------------------------------------
#define full_depth_moves 4
#define reduction_limit 3
#define INFINITY 50000
#define mateVal 49000
#define mateScore 48000
#define MAX_DEPTH 64

/*
 * Negamax & Quiescence:
 * Now they accept a reference to ThreadData (td),
 * which holds PV arrays, killer moves, history, etc.
*/
int negamax(thrawn::Position& pos, ThreadData &td, int depth, int alpha, int beta, int ply, bool isPvNode);
int quiescence(thrawn::Position& pos, ThreadData &td, int alpha, int beta, int ply);

/*
 * Single-threaded search driver
 * (Replaces your old 'search_position(...)' function.)
*/
void search_position_singlethreaded(thrawn::Position& pos, int depth);

/*
 * Move ordering utilities
 */
int score_move(thrawn::Position& pos, ThreadData &td, int move, int ply);
void sort_moves(thrawn::Position& pos, ThreadData &td, std::vector<int>& moves, int bestMove, int ply);
void score_pv(std::vector<int>& moves, ThreadData &td, int ply);

/*
 * Repetition check
 */
int isRepetition(thrawn::Position& pos, ThreadData &td);

/*
 * A helper for quicksort-based move ordering
 */
void quicksort_moves(std::vector<int> &moves,
                     std::vector<int> &move_scores,
                     int low, int high);

/*
 * Some futility / LMP helpers
 */
int futility_margin(int depth);
int futility_move_count(int depth);

#endif // SEARCH_H
