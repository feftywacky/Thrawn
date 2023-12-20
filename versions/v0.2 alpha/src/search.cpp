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
        return quiescence(alpha, beta);
        // return evaluate();
    }

    nodes++;
    int valid_moves = 0;
    bool inCheck = is_square_under_attack((colour_to_move==white ? get_lsb_index(piece_bitboards[K]) : get_lsb_index(piece_bitboards[k])), colour_to_move^1);


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
        valid_moves++;

        int score = -negamax(depth-1, -beta, -alpha);

        ply--;
        restoreBoard();
        
        
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

    if (valid_moves == 0)
    {
        if (inCheck)
        {
            return -19999 + ply; // +ply allows engine to find the smallest depth mate
            // penalizing longer mates less than shorter ones
            
        }
        else
            return 0; // stalemate
    }


    if (old_alpha != alpha)
        best_move = best_move_for_now;

    // move fails low (<= alpha)
    return alpha;
}

int quiescence(int alpha, int beta)
{
    int evaluation = evaluate();

    // fail-hard beta cutoff
    if (evaluation>=beta)
        return beta; // fails high

    // found better move 
    if (evaluation > alpha)
    {
        alpha = evaluation; // principal variation PV node (best move)
    }

    vector<int> moves = generate_moves();

    for (int move : moves)
    {
        copyBoard();

        ply++;

        if (!make_move(move, only_captures))
        {
            ply--;
            continue;
        }

        int score = -quiescence(-beta, -alpha);

        ply--;
        restoreBoard();
        
        
        // fail-hard beta cutoff
        if (score>=beta)
            return beta; // fails high

        // found better move 
        if (score > alpha)
        {
            alpha = score; // principal variation PV node (best move)
        }
        
    }

    // move fails low (<= alpha)
    return alpha;
}

void search_position(int depth)
{
    int score = negamax(depth, -20000, 20000);

    if (best_move)
    {
        cout<< "info score cp " << score << " depth " << depth << " nodes " << nodes <<endl;
        cout<<"bestmove ";
        print_move(best_move);
    }
}

