#include "search.h"
#include "evaluation.h"
#include "engine.h"
#include "move_helpers.h"
#include <iostream>
#include <vector>

using namespace std;

int ply;
int best_move;
int nodes;

int negamax(int depth, int alpha, int beta)
{
    if (depth == 0)
    {
        return evaluate();
    }

    nodes++;

    int best_move_for_now;

    int old_alpha = alpha;

    vector<int> moves = generate_moves();

    for (int move : moves)
    {
        copyBoard();

        ply++;

        if (!make_move(move, all_moves))
        {
            ply--;
            continue;
        }

        int score = -negamax(depth-1, -beta, -alpha);

        restoreBoard();
        ply--;
        
        // fail-hard beta cutoff
        if (score>=beta)
            return beta; // fails high

        // found better move 
        if (score > alpha)
        {
            alpha = score; // principal variation PV node (best move)
            if (ply==0) // at root move
                best_move_for_now = move;
        }
        
    }

    if (old_alpha != alpha)
        best_move = best_move_for_now;

    // move fails low (<= alpha)
    return alpha;
}

void search_position(int depth)
{
    int score = negamax(depth, -500000, 500000);

    cout<<"bestmove ";
    print_move(best_move);
}

