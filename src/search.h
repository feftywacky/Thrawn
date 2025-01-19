#ifndef SEARCH_H
#define SEARCH_H

#include <vector>
#include <array>
#include <cstdint>
#include <atomic>
#include "position.h"

using namespace std;

extern uint64_t nodes;

extern std::vector<std::vector<int>> killer_moves;
extern std::vector<std::vector<int>> history_moves;
extern std::vector<int> pv_depth;
extern std::vector<std::vector<int>> pv_table;

extern bool follow_pv_flag;
extern bool score_pv_flag;

extern bool allowNullMovePruning;
extern bool allowFutilityPruning;

extern std::array<int, 4> LateMovePruning_factors;
extern int RFP_factor;

// repetition
extern std::vector<uint64_t> repetition_table;
extern int repetition_index;
extern int fifty_move;

// threading
extern std::atomic<bool> stop_threads;

#define full_depth_moves 4
#define reduction_limit 3
#define INFINITY 50000
#define mateVal 49000
#define mateScore 48000
#define MAX_DEPTH 64

// ----------------------
//  Added ply parameter:
// ----------------------
int negamax(thrawn::Position& pos, int depth, int alpha, int beta, int ply);

int quiescence(thrawn::Position& pos, int alpha, int beta, int ply);

void search_position(thrawn::Position& pos, int depth);

void search_position_threaded(thrawn::Position& pos, int depth, int numThreads);

// Also added ply parameter to move-scoring utilities
int score_move(thrawn::Position& pos, int move, int ply);

void sort_moves(thrawn::Position& pos, vector<int>& moves, int bestMove, int ply);

void score_pv(vector<int>& moves, int ply);

void print_move_scores(thrawn::Position& pos, const vector<int>& moves, int ply);

int isRepetition(thrawn::Position& pos);

void quicksort_moves(std::vector<int> &moves, std::vector<int> &move_scores, int low, int high);

int futility_margin(int depth);

int futility_move_count(int depth);

#endif
