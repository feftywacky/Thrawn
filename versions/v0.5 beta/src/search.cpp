#include "search.h"
#include "evaluation.h"
#include "engine.h"
#include "move_helpers.h"
#include "uci.h"
#include <iostream>
#include <vector>
#include <algorithm>
#include <numeric>
#include <cstring>

using namespace std;

const int MAX_DEPTH = 64;
int ply;
long nodes;

std::vector<int> pv_depth(MAX_DEPTH); // [ply]
std::vector<std::vector<int>> pv_table(MAX_DEPTH, std::vector<int>(MAX_DEPTH)); // [ply][ply]
std::vector<std::vector<int>> killer_moves(2, std::vector<int>(MAX_DEPTH)); // [killer 1 or 2][ply]
std::vector<std::vector<int>> history_moves(12, std::vector<int>(64)); // [piece][sq]

// pv sorting
bool follow_pv_flag = false;
bool score_pv_flag = false;

const int full_depth_moves = 4;
const int reduction_limit = 3;

int negamax(int depth, int alpha, int beta)
{
    if ((nodes & 2047)==0)
        communicate();

    // init pv 
    pv_depth[ply] = ply;

    
    if (depth == 0)
    {
        return quiescence(alpha, beta);
    }

    if (ply>=MAX_DEPTH) // array overflow at max depth
    {
        std::cout<<"array overflow at max depth: "<<ply<<endl;
        return evaluate();
    }


    nodes++;
    int valid_moves = 0;
    bool inCheck = is_square_under_attack((colour_to_move==white ? get_lsb_index(piece_bitboards[K]) : get_lsb_index(piece_bitboards[k])), colour_to_move^1);
    
    if(inCheck)
        depth++;
    
    // null move pruning
    if (depth>=3 && inCheck==false && ply) 
    {
        // give opponent another move
        copyBoard();
        colour_to_move ^= 1;
        enpassant = null_sq;
        // depth - 1 - R, R is reduction constant
        int score = -negamax(depth-1-2, -beta, -beta+1);
        restoreBoard();
        // time is up
        if(stopped == 1) return 0;
        // fail hard beta cut-off
        if (score>=beta)
            return beta;
    }

    vector<int> moves = generate_moves();

    if (follow_pv_flag)
    {
        score_pv(moves);
    }

    sort_moves(moves);

    int moves_searched = 0;

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


        int score = 0;

        // full depth search
        if (moves_searched == 0) 
            score = -negamax(depth-1, -beta, -alpha);

        else // late move reduction (lmr)
        {
            if (moves_searched >= full_depth_moves && depth >= reduction_limit && inCheck == false && get_is_capture_move(move) == 0 && get_promoted_piece(move) == 0)
                score = -negamax(depth - 2, -alpha - 1, -alpha);

            // ensure that full-depth search is done
            else 
                score = alpha + 1;
            
            // if found a better move during LMR
            // PVS
            if(score > alpha)
            {
                // re-search at full depth but with narrowed score bandwith
                score = -negamax(depth-1, -alpha - 1, -alpha);
            
                // if LMR fails re-search at full depth and full score bandwith
                if((score > alpha) && (score < beta))
                    score = -negamax(depth-1, -beta, -alpha);
            }
        }


        ply--;
        restoreBoard();

        // time is up
        if(stopped == 1) return 0;

        moves_searched++;
        
        // fail-hard beta cutoff
        if (score>=beta)
        {
            if (get_is_capture_move(move)==0)
            {
                killer_moves[1][ply] = killer_moves[0][ply];
                killer_moves[0][ply] = move;
            }
            return beta; // fails high
        }

        // found better move, pv
        if (score> alpha)
        {
            if (get_is_capture_move(move)==0)
                history_moves[get_move_piece(move)][get_move_target(move)] += depth;

            alpha = score; // principal variation PV node (best move)

            pv_table[ply][ply] = move;
            // store deeper ply move into current ply
            for (int nextPly=ply+1;nextPly<pv_depth[ply+1];nextPly++)
            {
                pv_table[ply][nextPly] = pv_table[ply+1][nextPly];
            }

            pv_depth[ply] = pv_depth[ply+1];
        }
        
    }

    if (valid_moves == 0)
    {
        if (inCheck)
        {
            return -59000 + ply; // +ply allows engine to find the smallest depth mate
            // penalizing longer mates less than shorter ones
            
        }
        else
            return 0; // stalemate
    }

    // move fails low (<= alpha)
    return alpha;
}

int quiescence(int alpha, int beta)
{
    if ((nodes & 2047)==0)
        communicate();

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

        // time is up
        if(stopped == 1) return 0;
        
        
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
    
    // RESET VARIABLES
    int score = 0;
    nodes = 0;
    pv_depth.clear();
    pv_depth.resize(64);

    pv_table.clear();
    pv_table.resize(64, std::vector<int>(64));

    killer_moves.clear();
    killer_moves.resize(2, std::vector<int>(64));

    history_moves.clear();
    history_moves.resize(12, std::vector<int>(64));

    follow_pv_flag = false;
    score_pv_flag = false;

    int alpha = -60000;
    int beta = 60000;

    // time control
    stopped = 0;

    // iterative deepening
    for (int curr_depth = 1;curr_depth<=depth;curr_depth++)
    {
        // time is up
        if (stopped == 1) break;
        
        follow_pv_flag = true;
        score = negamax(curr_depth, alpha, beta);


        // aspiration window
        if (score<=alpha || score>=beta)
        {
            alpha = -60000;
            beta = 60000;
            continue;
        }

        alpha = score - 50;
        beta = score + 50;
    
        std::cout<< "info score cp " << score << " depth " << curr_depth << " nodes " << nodes << " pv ";

        for (int i=0;i<pv_depth[0];i++)
        { 
            print_move(pv_table[0][i]);
            std::cout<<" ";
        }
        std::cout<<"\n";
    }

    std::cout<<"bestmove ";
    print_move(pv_table[0][0]);
    std::cout<<"\n";

}


// SCORING PRIORITY
// 1) PV
// 2) capture moves in mvv_lva
// 3) killer move 1
// 4) killer move 2
// 5) history moves
// 6) unsorted moves
int score_move(int move)
{
    // scoring pv
    if (score_pv_flag)
    {
        if (pv_table[0][ply] == move)
        {
            score_pv_flag = false;
            return 20000; // give pv move priority
        }
    }

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
        return mvv_lva[get_move_piece(move)][target] + 10000;
    }
    
    // quiet moves
    else
    {   
        if (killer_moves[0][ply] == move)
            return 9000;
        else if (killer_moves[1][ply] == move)
            return 8000;
        else 
            return history_moves[get_move_piece(move)][get_move_target(move)];
    }

    return 0;
}

void score_pv(vector<int> &moves)
{
    follow_pv_flag = false;

    for (int move: moves)
    {
        if (pv_table[0][ply] == move)
        {
            score_pv_flag = true;
            follow_pv_flag = true;
        }
    }
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
        std::cout<<score_move(move)<<"\n";
    }
}

