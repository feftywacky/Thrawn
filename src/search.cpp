#include "search.h"
#include "evaluation.h"
#include "move_generator.h"
#include "move_helpers.h"
#include "zobrist_hashing.h"
#include "transposition_table.h"
#include "bitboard.h"
#include "uci.h" // for 'stopped' and 'communicate()'
#include <iostream>
#include <vector>
#include <algorithm>
#include <numeric>
#include <atomic>

/*
some notes for negamax
3 types
- fail high: causes beta cut-off
- fail low: don't increase alpha
- pv nodes: increase alpha
*/

// -------------------------------------------
// Global node counter
// -------------------------------------------
uint64_t nodes = 0;

/*
 Some global flags controlling pruning
*/
bool allowNullMovePruning = true;
bool allowFutilityPruning = false;

/*
 Late Move Pruning factors
*/
std::array<int, 4> LateMovePruning_factors = {0, 8, 12, 24};
int RFP_factor = 64;

/*
 * QUISCENCE
 * This function has a changed signature to accept (ThreadData &td).
 * We call 'score_move' and 'sort_moves' passing td,
 * so that we can reference killer moves, history, etc. if needed.
*/
int quiescence(thrawn::Position& pos, ThreadData &td,
               int alpha, int beta, int ply)
{
    if ((nodes & 2047) == 0)
    {
        communicate();
    }

    nodes++;

    // safety check for array bounds
    if (ply >= MAX_DEPTH)
    {
        return evaluate(pos);
    }

    int evaluation = evaluate(pos);

    if (evaluation >= beta)
        return beta;

    if (evaluation > alpha)
        alpha = evaluation;

    // Generate all moves (captures + possibly quiet checks if you do that).
    std::vector<int> moves = generate_moves(pos);

    // no bestMove needed at quiescence, so pass 0
    sort_moves(pos, td, moves, 0, ply);

    for (int move : moves)
    {
        // only captures
        if (!get_is_capture_move(move)) 
            continue;

        pos.copyBoard(ply);

        // update repetition
        td.repetition_index++;
        td.repetition_table[td.repetition_index] = pos.zobristKey;

        if (!make_move(pos, move, only_captures, ply))
        {
            td.repetition_index--;
            pos.restoreBoard(ply);
            continue;
        }

        int score = -quiescence(pos, td, -beta, -alpha, ply + 1);

        td.repetition_index--;
        pos.restoreBoard(ply);

        if (stopped == 1)
            return alpha;

        if (score > alpha)
        {
            alpha = score;
            if (score >= beta)
                return beta;
        }
    }

    return alpha;
}

/*
 * NEGAMAX
 * Contains all your logic for:
 *  - repetition checks
 *  - TT probe
 *  - null move pruning
 *  - razoring
 *  - generate moves
 *  - move ordering (including killer, history)
 *  - LMR / PVS
 *  - storing best moves in PV
 *  - storing in TT
 */
int negamax(thrawn::Position& pos, ThreadData &td,
            int depth, int alpha, int beta, int ply)
{
    int score = 0;
    int bestMove = 0;
    int hashFlag = hashFlagALPHA;
    int static_eval = 0;

    // init local pv
    td.pv_depth[ply] = ply;

    // check repetition or 50-move rule
    if (ply && (isRepetition(pos, td) || td.fifty_move >= 100))
    {
        return 0;
    }

    // determines if current node is a pv node
    int pv_node = (beta - alpha) > 1;

    // retrieve from TT if possible
    if (ply && (score = probeHashMap(pos, depth, alpha, beta, &bestMove, ply)) != no_hashmap_entry && !pv_node)
    {
        return score;
    }

    if ((nodes & 2047) == 0)
    {
        communicate();
    }

    if (depth == 0)
    {
        // call quiescence with local ply
        return quiescence(pos, td, alpha, beta, ply);
    }

    if (ply > MAX_DEPTH - 1)
    {
        std::cout << "array overflow at max depth: " << ply << std::endl;
        return evaluate(pos);
    }

    nodes++;
    int valid_moves = 0;

    bool inCheck = is_square_under_attack(
        pos,
        (pos.colour_to_move == white ?
          get_lsb_index(pos.piece_bitboards[K]) :
          get_lsb_index(pos.piece_bitboards[k])),
        pos.colour_to_move ^ 1
    );

    if (inCheck)
    {
        depth++;
    }

    static_eval = evaluate(pos);

    // null move pruning
    if (!inCheck && depth >= 3 && ply && !pv_node && !noMajorsOrMinorsPieces(pos))
    {
        pos.copyBoard(ply);
        td.repetition_index++;
        td.repetition_table[td.repetition_index] = pos.zobristKey;

        if (pos.enpassant != null_sq)
            pos.zobristKey ^= pos.enpassant_hashkey[pos.enpassant];
        pos.enpassant = null_sq;

        pos.colour_to_move ^= 1;
        pos.zobristKey ^= pos.colour_to_move_hashkey;

        // do the reduced-depth search
        score = -negamax(pos, td, depth - 1 - 2, -beta, -beta + 1, ply + 1);

        td.repetition_index--;
        pos.restoreBoard(ply);

        if (stopped == 1)
            return alpha;

        if (score >= beta)
            return beta;
    }

    // razoring pruning
    if (!inCheck && !pv_node && depth <= 3)
    {
        score = static_eval + 125;
        if (score < beta)
        {
            if (depth == 1)
            {
                int razor_score = quiescence(pos, td, alpha, beta, ply);
                return (razor_score > score) ? razor_score : score;
            }
            score += 175;
            if (score < beta && depth <= 2)
            {
                int razor_score = quiescence(pos, td, alpha, beta, ply);
                if (razor_score < beta)
                    return (razor_score > score) ? razor_score : score;
            }
        }
    }

    // generate moves
    std::vector<int> moves = generate_moves(pos);

    // first, see if the bestMove from TT or PV is present
    if (td.follow_pv_flag)
        score_pv(moves, td, ply);

    // now sort them (with local ply)
    sort_moves(pos, td, moves, bestMove, ply);

    int moves_searched = 0;

    for (int move : moves)
    {
        pos.copyBoard(ply);
        td.repetition_index++;
        td.repetition_table[td.repetition_index] = pos.zobristKey;

        if (make_move(pos, move, all_moves, ply) == 0)
        {
            td.repetition_index--;
            pos.restoreBoard(ply);
            continue;
        }

        valid_moves++;

        int move_score;
        if (moves_searched == 0)
        {
            // full window search
            move_score = -negamax(pos, td, depth - 1, -beta, -alpha, ply + 1);
        }
        else
        {
            // late move reductions, PVS, etc.
            move_score = alpha + 1;
            if (valid_moves >= full_depth_moves &&
                depth >= reduction_limit &&
                !inCheck &&
                get_is_capture_move(move) == 0 &&
                get_promoted_piece(move) == 0)
            {
                // do a reduced search
                move_score = -negamax(pos, td, depth - 2, -alpha - 1, -alpha, ply + 1);
            }

            if (move_score > alpha)
            {
                // do a PVS search with narrow window
                move_score = -negamax(pos, td, depth - 1, -alpha - 1, -alpha, ply + 1);

                if (move_score > alpha && move_score < beta)
                {
                    // full window
                    move_score = -negamax(pos, td, depth - 1, -beta, -alpha, ply + 1);
                }
            }
        }

        td.repetition_index--;
        pos.restoreBoard(ply);

        if (stopped == 1)
            return alpha;

        moves_searched++;

        // check for new best move
        if (move_score > alpha)
        {
            alpha = move_score;
            bestMove = move;
            hashFlag = hashFlagEXACT;

            // update history
            if (get_is_capture_move(move) == 0)
            {
                td.history_moves[get_move_piece(move)][get_move_target(move)] += depth;
            }

            // update PV
            td.pv_table[ply][ply] = move;
            for (int nextPly = ply + 1; nextPly < td.pv_depth[ply + 1]; nextPly++)
            {
                td.pv_table[ply][nextPly] = td.pv_table[ply + 1][nextPly];
            }
            td.pv_depth[ply] = td.pv_depth[ply + 1];

            // beta cutoff
            if (alpha >= beta)
            {
                writeToHashMap(pos, depth, beta, hashFlagBETA, bestMove, ply);

                // killer move
                if (get_is_capture_move(move) == 0)
                {
                    td.killer_moves[1][ply] = td.killer_moves[0][ply];
                    td.killer_moves[0][ply] = move;
                }
                return beta;
            }
        }
    }

    if (valid_moves == 0)
    {
        if (inCheck)
        {
            // mate score
            return -mateVal + ply;
        }
        else
        {
            // stalemate
            return 0;
        }
    }

    writeToHashMap(pos, depth, alpha, hashFlag, bestMove, ply);
    return alpha;
}

/*
 * SINGLE-THREAD SEARCH:
 * We replaced your old 'search_position(...)' with
 * 'search_position_singlethreaded(...)'.
 * 
 * This function sets up a local ThreadData instance,
 * does iterative deepening, calls negamax(...) with the 'td',
 * and finally prints "bestmove".
*/
void search_position_singlethreaded(thrawn::Position& pos, int depth)
{
    int start = get_time_ms();
    nodes = 0;
    stopped = 0;

    // for debugging
    allowNullMovePruning = false;
    allowFutilityPruning = false;

    // Create local thread data
    ThreadData td;

    int score = 0;
    int alpha = -INFINITY;
    int beta  = INFINITY;

    // iterative deepening
    for (int curr_depth = 1; curr_depth <= depth; curr_depth++)
    {
        if (stopped == 1)
            break;

        // set up for PV sorting
        td.follow_pv_flag = true;
        score = negamax(pos, td, curr_depth, alpha, beta, 0);

        // aspiration window
        if (score <= alpha || score >= beta)
        {
            alpha = -INFINITY;
            beta  = INFINITY;
            continue;
        }

        alpha = score - 50;
        beta  = score + 50;

        // if pv exists
        if (td.pv_depth[0])
        {
            if (score > -mateVal && score < -mateScore)
            {
                std::cout << "info score mate " << -(score + mateVal) / 2 - 1
                          << " depth " << curr_depth
                          << " nodes " << nodes
                          << " time " << (get_time_ms() - start)
                          << " pv ";
            }
            else if (score > mateScore && score < mateVal)
            {
                std::cout << "info score mate " << (mateVal - score) / 2 + 1
                          << " depth " << curr_depth
                          << " nodes " << nodes
                          << " time " << (get_time_ms() - start)
                          << " pv ";
            }
            else
            {
                std::cout << "info score cp " << score
                          << " depth " << curr_depth
                          << " nodes " << nodes
                          << " time " << (get_time_ms() - start)
                          << " pv ";
            }

            for (int i = 0; i < td.pv_depth[0]; i++)
            {
                print_move(td.pv_table[0][i]);
                std::cout << " ";
            }
            std::cout << "\n";
        }
    }

    std::cout << "bestmove ";
    print_move(td.pv_table[0][0]);
    std::cout << "\n";

    stopped = 1;
}

/*
 * MOVE ORDERING
 * We pass 'ThreadData &td' to access killer or history arrays
*/

// ----------------------------------------------------------
// Score a single move with local ply
// ----------------------------------------------------------
int score_move(thrawn::Position& pos, ThreadData &td, int move, int ply)
{
    // scoring pv
    if (td.score_pv_flag)
    {
        if (td.pv_table[0][ply] == move)
        {
            td.score_pv_flag = false;
            return 20000;
        }
    }

    // handle promotions
    if (get_promoted_piece(move) == Q || get_promoted_piece(move) == q)
    {
        return 10000 + 499;
    }

    // captures: use MVV-LVA
    if (get_is_capture_move(move))
    {
        // find target piece
        int target = P;
        int start_piece;
        int end_piece;
        (pos.colour_to_move == white) ? start_piece = p : start_piece = P;
        (pos.colour_to_move == white) ? end_piece = k : end_piece = K;

        for (int i = start_piece; i <= end_piece; i++)
        {
            if (get_bit(pos.piece_bitboards[i], get_move_target(move)))
            {
                target = i;
                break;
            }
        }
        return mvv_lva[get_move_piece(move)][target] + 10000;
    }
    else
    {
        // quiet moves
        if (td.killer_moves[0][ply] == move)
            return 9000;
        else if (td.killer_moves[1][ply] == move)
            return 8000;
        else if (get_promoted_piece(move) == Q || get_promoted_piece(move) == q)
            return 5000; // example
        else
            return td.history_moves[get_move_piece(move)][get_move_target(move)];
    }
    return 0;
}

// ---------------------------------------------------------
// For ordering the PV move
// ---------------------------------------------------------
void score_pv(std::vector<int> &moves, ThreadData &td, int ply)
{
    td.follow_pv_flag = false;
    for (int move : moves)
    {
        if (td.pv_table[0][ply] == move)
        {
            td.score_pv_flag = true;
            td.follow_pv_flag = true;
        }
    }
}

// ---------------------------------------------------------
// Sort moves with local ply
// ---------------------------------------------------------
void sort_moves(thrawn::Position& pos, ThreadData &td,
                std::vector<int> &moves, int bestMove, int ply)
{
    const int n = static_cast<int>(moves.size());
    std::vector<int> scores(n);
    for (int i = 0; i < n; i++)
    {
        // If a move is the bestMove from TT, give it a high score
        if (moves[i] == bestMove)
            scores[i] = 30000;
        else
            scores[i] = score_move(pos, td, moves[i], ply);
    }

    quicksort_moves(moves, scores, 0, n - 1);
}

// standard quicksort helper
void quicksort_moves(std::vector<int> &moves, std::vector<int> &move_scores,
                     int low, int high)
{
    if (low < high)
    {
        int pivot = move_scores[high];
        int i = low - 1;

        for (int j = low; j <= high - 1; j++)
        {
            if (move_scores[j] > pivot)
            {
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

// repetition check
int isRepetition(thrawn::Position& pos, ThreadData &td)
{
    for (int i = 0; i < td.repetition_index; i++)
    {
        if (td.repetition_table[i] == pos.zobristKey)
            return 1;
    }
    return 0;
}

// futility
int futility_margin(int depth)
{
    return 168 * depth;
}

int futility_move_count(int depth)
{
    return (3 + depth * depth) / 2;
}
