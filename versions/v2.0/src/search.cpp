#include "search.h"
#include "evaluation.h"
#include "move_generator.h"
#include "move_helpers.h"
#include "uci.h"
#include "zobrist_hashing.h"
#include "transposition_table.h"
#include <iostream>
#include <vector>
#include <algorithm>
#include <numeric>
#include <cstring>

using namespace std;

/*
some notes for negamax 
3 types
- fail high: causes beta cut-off
- fail low: don't increase alpha
- pv nodes: increase alpha
*/

int ply;
uint64_t nodes;

std::vector<int> pv_depth(MAX_DEPTH); // [ply]
std::vector<std::vector<int>> pv_table(MAX_DEPTH, std::vector<int>(MAX_DEPTH)); // [ply][ply]
std::vector<std::vector<int>> killer_moves(2, std::vector<int>(MAX_DEPTH)); // [killer 1 or 2][ply]
std::vector<std::vector<int>> history_moves(12, std::vector<int>(64)); // [piece][sq]

// pv sorting
bool follow_pv_flag = false;
bool score_pv_flag = false;

bool allowNullMovePruning = true;
bool allowFutilityPruning = false;

std::array<int, 4> LateMovePruning_factors = { 0, 8, 12, 24};
int RFP_factor = 64;

// repetition
std::vector<uint64_t> repetition_table(1028); // 1028 plies for a game
int repetition_index = 0;
int fifty_move = 0;

int negamax(int depth, int alpha, int beta)
{
    int score = 0;
    int bestMove = 0;
    int hashFlag = hashFlagALPHA;
    int static_eval = 0;
    // init pv 
    pv_depth[ply] = ply;

    // stalemate if 3 move repetition or fifty-move rule
    if (ply && isRepetition() || fifty_move >= 100)
        return 0;

    // determines if current node is a pv node
    int pv_node = ((beta - alpha) > 1); // IMPORTANT FIXES TRANPOSITION TABLE PV BUG

    // retrieve score if not root ply and not pv node and tt key exists
    // if move has already been searched, return its score instantly
    if (ply && ((score = probeHashMap(depth, alpha, beta, &bestMove)) != no_hashmap_entry) && !pv_node)
    {
        if (fifty_move<90)
            return score;
    }

    if ((nodes & 2047)==0)
    {
        communicate();
    }

    nodes++;
    int valid_moves = 0;
    bool inCheck = is_square_under_attack((colour_to_move==white ? get_lsb_index(piece_bitboards[K]) : get_lsb_index(piece_bitboards[k])), colour_to_move^1);

    if (inCheck)
    {
        depth++;
        goto full_search;
    }

    if (depth == 0)
        return quiescence(alpha, beta);

    if (ply>MAX_DEPTH-1) // array overflow at max depth
    {
        std::cout<<"array overflow at max depth: "<<ply<<endl;
        return evaluate();
    }

    static_eval = evaluate();

    // Reverse Futility Pruning / static null move pruning
	if (depth < 3 && !pv_node && !inCheck &&  abs(beta - 1) > -INFINITY + 100)
	{   
		int eval_margin = 120 * depth;
		
		// evaluation margin substracted from static evaluation score fails high
		if (static_eval - eval_margin >= beta)
			return static_eval - eval_margin;
	}

    // razoring pruning (forward pruning)
    if (!inCheck && !pv_node && depth <= 3)
    {
        // apply bonus to score
        score = evaluate() + 125; 
        int razor_score;
        if (score<beta)
        {
            if (depth == 1)
            {
                razor_score = quiescence(alpha, beta);
                return (razor_score > score) ? razor_score : score;
            }
            // second bonus to score
            score += 175;
            if ( score < beta && depth <= 2)
            {
                razor_score = quiescence(alpha,beta);
                if (razor_score < beta) // quiescence says score fail-low node
                    return (razor_score > score) ? razor_score : score;
            }
        }
    }
    
    // null move pruning
    if (!inCheck && (depth>=3) && allowNullMovePruning && !pv_node && !noMajorsOrMinorsPieces()) 
    {
        // give opponent another move
        copyBoard();

        ply++;
        repetition_index++;
        repetition_table[repetition_index] = zobristKey;

        if (enpassant!=null_sq) 
            zobristKey ^= enpassant_hashkey[enpassant];
        enpassant = null_sq;

        colour_to_move ^= 1;
        zobristKey ^= colour_to_move_hashkey;
        
        // depth - 1 - R, R is reduction constant
        allowNullMovePruning = false;
        score = -negamax(depth-1-2, -beta, -beta+1);
        allowNullMovePruning = true;

        ply--;
        repetition_index--;

        restoreBoard();

        // time is up
        if(stopped == 1) 
            return 0;
        // fail hard beta cut-off
        if (score>=beta)
            return beta;
    }


    //Futility Pruning Detection (extended futility: beyond depth == 1)
    // if (!inCheck && !pv_node && (ply > 0) && (depth <= 8))
    //     if ((static_eval + futility_margin(depth)) <= alpha)
	// 		allowFutilityPruning = true;

    // No-hashmove reduction (taken from Stockfish)
    // If the position is not in TT, decrease depth by 1 (~3 Elo)
    if (!inCheck && pv_node && (depth >= 3) && !bestMove)
        depth--;


    full_search:


    vector<int> moves = generate_moves();

    if (follow_pv_flag)
        score_pv(moves);

    sort_moves(moves, bestMove);
    

    int moves_searched = 0;

    for (int move : moves)
    {
        copyBoard();

        ply++;
        repetition_index++;
        repetition_table[repetition_index] = zobristKey;

        if (make_move(move, all_moves)==0)
        {
            ply--;
            repetition_index--;
            continue;
        }

        // used for avoiding reductions on moves that give check
        bool givesCheck = is_square_under_attack((colour_to_move == white) ? get_lsb_index(piece_bitboards[K]) : 
                                                                    get_lsb_index(piece_bitboards[k]),
                                                                    colour_to_move ^ 1);        
        valid_moves++;

        // full depth search
        if (moves_searched == 0) 
            score = -negamax(depth-1, -beta, -alpha);

        
        // pruning techniques
        else 
        {   
            // Futility Pruning on current move
            // if (allowFutilityPruning && valid_moves>1)
            // {
            //     if (!givesCheck && (killer_moves[0][ply] != move)
            //                     && (killer_moves[1][ply] != move)
            //                     && (get_move_piece(move) != P)
            //                     && (get_move_piece(move) != p)
            //                     && !get_promoted_piece(move)
            //                     && !get_is_move_castling(move)
            //                     && !get_is_capture_move(move))
            //     {
            //         // undo the current move and skip to the next one
            //         restoreBoard();
            //         ply--;
            //         repetition_index--;

            //         continue;
            //     }
            // }

            // Late Move Pruning (LMP)
		    if ((ply > 0) && (depth <= 3)
                          && !pv_node
                          && !inCheck
                          && !get_is_capture_move(move)
                          && (valid_moves > LateMovePruning_factors[depth]))
            {
                // undo the current move and skip to the next one
                restoreBoard();
                ply--;
                repetition_index--;

                continue;
			}

            // late move reduction (LMR)
            if (valid_moves >= full_depth_moves && 
                depth >= reduction_limit && 
                !inCheck && 
                get_is_capture_move(move) == 0 && 
                get_promoted_piece(move) == 0
                )
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
        repetition_index--;

        restoreBoard();

        // time is up
        if(stopped == 1) 
            return 0;

        moves_searched++;

        // found better move, pv
        if (score> alpha)
        {
            bestMove = move;
            hashFlag = hashFlagEXACT; // pv node

            if (get_is_capture_move(move)==0)
                history_moves[get_move_piece(move)][get_move_target(move)] += depth;

            alpha = score; // principal variation PV node (best move)

            pv_table[ply][ply] = move;
            // store deeper ply move into current ply
            for (int nextPly=ply+1; nextPly<pv_depth[ply+1]; nextPly++)
            {
                pv_table[ply][nextPly] = pv_table[ply+1][nextPly];
            }

            pv_depth[ply] = pv_depth[ply+1];

            // fail-hard beta cutoff
            if (score>=beta)
            {
                writeToHashMap(depth, beta, hashFlagBETA, bestMove);

                if (get_is_capture_move(move)==0)
                {
                    killer_moves[1][ply] = killer_moves[0][ply];
                    killer_moves[0][ply] = move;
                }
                return beta; // fails high
            }
        }
        
    }

    if (valid_moves == 0)
    {
        if (inCheck)
        {
            return -mateVal + ply; // +ply allows engine to find the smallest depth mate
            // penalizing longer mates less than shorter ones
        }
        else
            return 0; // stalemate
    }

    writeToHashMap(depth, alpha, hashFlag, bestMove);

    // move fails low (<= alpha)
    return alpha;
}

int quiescence(int alpha, int beta)
{
    if ((nodes & 2047)==0)
    {
        communicate();
    }

    nodes++;

    if (ply>MAX_DEPTH - 1)
        return evaluate();

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
    sort_moves(moves, 0);

    for (int move : moves)
    {
        copyBoard();

        ply++;
        repetition_index++;
        repetition_table[repetition_index] = zobristKey;

        if (!make_move(move, only_captures))
        {
            ply--;
            repetition_index--;
            continue;
        }

        int score = -quiescence(-beta, -alpha);

        ply--;
        repetition_index--;

        restoreBoard();

        // time is up
        if(stopped == 1) return 0;

        // found better move 
        if (score > alpha)
        {
            alpha = score; // principal variation PV node (best move)

            // fail-hard beta cutoff
            if (score>=beta)
                return beta; // fails high
        }
        
    }

    // move fails low (<= alpha)
    return alpha;
}

int isRepetition()
{
    for (int i = 0; i < repetition_index; i++)
    {
        if (repetition_table[i] == zobristKey)
            return 1;
    }
    return 0;
}

void search_position(int depth)
{
    // RESET VARIABLES
    nodes = 0;
    // time control
    stopped = 0;

    follow_pv_flag = false;
    score_pv_flag = false;
    
    allowNullMovePruning = true;
    allowFutilityPruning = false;

    pv_depth.assign(MAX_DEPTH, 0);
    for (auto& row : pv_table) 
        row.assign(MAX_DEPTH, 0);
    for (auto& row : killer_moves)
        row.assign(MAX_DEPTH, 0);
    for (auto& row : history_moves)
        row.assign(64, 0);

    int score = 0;
    int alpha = -INFINITY;
    int beta = INFINITY;

    int start = get_time_ms();

    // iterative deepening
    for (int curr_depth = 1;curr_depth<=depth;curr_depth++)
    {
        // time is up
        if (stopped == 1)
        { 
            break;
        }
        
        follow_pv_flag = true;
        score = negamax(curr_depth, alpha, beta);
        
        // aspiration window
        if ((score<=alpha) || (score>=beta))
        {
            alpha = -INFINITY;
            beta = INFINITY;
            continue;
        }

        // set up the window for the next iteration
        alpha = score - 50;
        beta = score + 50;

        // if pv exist
        if (pv_depth[0])
        {
            if (score > -mateVal && score < -mateScore)
                std::cout << "info score mate " << -(score + mateVal) / 2 - 1
                        << " depth " << curr_depth
                        << " nodes " << nodes
                        << " time " << static_cast<unsigned int>(get_time_ms() - start)
                        << " pv ";
            else if (score > mateScore && score < mateVal)
                std::cout << "info score mate " << (mateVal - score) / 2 + 1
                        << " depth " << curr_depth
                        << " nodes " << nodes
                        << " time " << static_cast<unsigned int>(get_time_ms() - start)
                        << " pv ";
            else
                std::cout << "info score cp " << score
                        << " depth " << curr_depth
                        << " nodes " << nodes
                        << " time " << static_cast<unsigned int>(get_time_ms() - start)
                        << " pv ";

            for (int i=0;i<pv_depth[0];i++)
            { 
                print_move(pv_table[0][i]);
                std::cout<<" ";
            }
            std::cout<<"\n";
        }
    }

    std::cout<<"bestmove ";
    print_move(pv_table[0][0]);
    std::cout<<"\n";

    stopped = 1; // fixes zero eval blundering bug
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

        if (get_promoted_piece(move) == Q || get_promoted_piece(q))
            return mvv_lva[get_move_piece(move)][target] + 10000 + 100;
        return mvv_lva[get_move_piece(move)][target] + 10000;
    }
    
    // quiet moves
    else
    {   
        if (killer_moves[0][ply] == move)
            return 9000;
        else if (killer_moves[1][ply] == move)
            return 8000;
        else if (get_promoted_piece(move) == Q || get_promoted_piece(q))
            return mvv_lva[get_move_piece(move)][get_move_target(move)]+100;
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

void sort_moves(vector<int> &moves, int bestMove)
{
    vector<int> move_scores(moves.size());
    
    // score all the moves within a move list
    for (int count = 0; count < moves.size(); count++)
    {
        // if hash move available
        if (bestMove == moves[count])
        {
            move_scores[count] = 30000;
        }

        else
            move_scores[count] = score_move(moves[count]);
    }

    quicksort_moves(moves, move_scores, 0, moves.size() - 1);
}

void print_move_scores(const vector<int>& moves)
{
    for (int move : moves)
    {
        int score = score_move(move);
        if (score>0)
        {
            if (score>=20000)
            {
                print_move(move);
                printf(" pv/best: %d\n", score);
            }
            print_move(move);
            printf(" %d\n", score);
        }
    }
}

void quicksort_moves(std::vector<int> &moves, std::vector<int> &move_scores, int low, int high) {
    if (low < high) {
        int pivot = move_scores[high];
        int i = low - 1;

        for (int j = low; j <= high - 1; j++) {
            if (move_scores[j] > pivot) {
                i++;
                std::swap(move_scores[i], move_scores[j]);
                std::swap(moves[i], moves[j]);
            }
        }

        std::swap(move_scores[i + 1], move_scores[high]);
        std::swap(moves[i + 1], moves[high]);

        int pi = i + 1;

        quicksort_moves(moves, move_scores, low, pi - 1);
        quicksort_moves(moves, move_scores, pi + 1, high);
    }
}

int futility_margin(int depth)
{
    return 168 * depth;
}

int futility_move_count(int depth)
{
    return (3 + depth * depth) / 2;
}
