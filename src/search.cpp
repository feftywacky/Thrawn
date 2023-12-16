#include "search.h"
#include "evaluation.h"
#include "engine.h"
#include "move_helpers.h"
#include <iostream>
#include <vector>
#include <algorithm>
#include <numeric>

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
    
    if(inCheck)
        depth++;

    int best_move_for_now;

    int old_alpha = alpha;

    vector<int> moves = generate_moves();
    sort_moves(moves);

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
    nodes++;

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
    sort_moves(moves);

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

int score_move(int move)
{
    if (get_is_capture_move(move))
    {
        int target = P;
        int start_piece;
        int end_piece;
        
        (colour_to_move==white) ? start_piece = p : start_piece = P;
        (colour_to_move==white) ? end_piece = k : end_piece = K;

        for(int i=start_piece; i<=end_piece;i++)
        {
            if (get_bit(piece_bitboards[i], get_move_target(move)))
            {
                target = i;
                break;
            }
        }
        return mvv_lva[get_move_piece(move)][target];
    }
    return 0;
}

void sort_moves(vector<int> &moves)
{
    vector<int> move_scores;

    for (int move : moves)
        move_scores.push_back(score_move(move));

    std::vector<size_t> indices(moves.size());
    std::iota(indices.begin(), indices.end(), 0);

    // Custom comparator that compares scores in descending order
    auto comparator = [&move_scores](size_t a, size_t b) {
        return move_scores[a] > move_scores[b];
    };

    // Sort indices in descending order based on move scores
    std::sort(indices.begin(), indices.end(), comparator);

    // In-place modification of moves vector based on the sorted indices
    std::vector<int> sorted_moves(moves.size());
    for (size_t i = 0; i < moves.size(); i++) {
        sorted_moves[i] = moves[indices[i]];
    }

    // Copy the sorted moves back to the original array (in-place modification)
    moves = std::move(sorted_moves);
}

void print_move_scores(const vector<int>& moves)
{
    for (int move : moves)
    {
        print_move(move);
        cout<<score_move(move)<<"\n";
    }
}

