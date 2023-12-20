#ifndef SEARCH_H
#define SEARCH_H


using namespace std;

extern int ply;
extern int best_move;
extern int nodes;

// negamax with alpha beta pruning
int negamax(int depth, int alpha, int beta);

void search_position(int depth);

#endif