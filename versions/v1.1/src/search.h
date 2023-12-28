#ifndef SEARCH_H
#define SEARCH_H

#include <vector>
#include <array>

using namespace std;

extern const int MAX_DEPTH;
extern int ply;
extern long nodes;

extern std::vector<std::vector<int>> killer_moves;
extern std::vector<std::vector<int>> history_moves;
extern std::vector<int> pv_depth;
extern std::vector<std::vector<int>> pv_table;

extern bool follow_pv_flag;
extern bool score_pv_flag;

extern const int full_depth_moves;
extern const int reduction_limit;

extern const int INFINITY;
extern const int mateVal;
extern const int mateScore;

// repetition
extern uint64_t repetition_table[1028];
extern int repetition_index;


// negamax with alpha beta pruning
int negamax(int depth, int alpha, int beta);

int quiescence(int alpha, int beta);

void search_position(int depth);

int score_move(int move);

void sort_moves(vector<int>& moves, int bestMove);

void score_pv(vector<int>& moves);

void print_move_scores(const vector<int>& moves);

int isRepetition();

#endif