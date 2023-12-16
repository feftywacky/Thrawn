#ifndef SEARCH_H
#define SEARCH_H

#include <vector>

using namespace std;

extern int ply;
extern int best_move;
extern int nodes;

// negamax with alpha beta pruning
int negamax(int depth, int alpha, int beta);

int quiescence(int alpha, int beta);

void search_position(int depth);

int score_move(int move);

void sort_moves(vector<int>& moves);

void print_move_scores(const vector<int>& moves);

#endif