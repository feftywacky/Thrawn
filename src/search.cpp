#include "search.h"
#include "evaluation.h"
#include "move_generator.h"
#include "move_helpers.h"
#include "zobrist_hashing.h"
#include "transposition_table.h"
#include "bitboard.h"
#include "uci.h" // for 'stopped' and 'communicate()'
#include "globals.h"
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
 Late Move Pruning factors
*/
std::array<int, 4> LateMovePruning_factors = {0, 8, 12, 24};
int RFP_factor = 64;

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
            int depth, int alpha, int beta, int ply, bool isPvNode)
{
    int score = 0;
    int bestMove = 0;
    int hashFlag = hashFlagALPHA;
    int static_eval = 0;

    // init local pv
    td.pv_depth[ply] = ply;

    // 1) Check repetition or 50-move draw
    if (ply && (isRepetition(pos, td) || td.fifty_move >= 100))
    {
        return 0;
    }

    // 2) Transposition Table lookup
    //    If something is found that fits within alpha/beta, return
    if (ply && (score = tt.probe(pos, depth, alpha, beta, bestMove, ply)) != no_hashmap_entry)
    {
        return score;
    }

    // For periodic UCI output / time check
    if ((nodes & 2047) == 0)
    {
        communicate();
    }

    // 3) Depth == 0 ⇒ Quiescence
    if (depth == 0)
    {
        // call quiescence with local ply
        return quiescence(pos, td, alpha, beta, ply);
    }

    // 4) Check for max depth overflow
    if (ply > MAX_DEPTH - 1)
    {
        // fallback to a static evaluation
        return evaluate(pos);
    }

    // 5) Increment node counter
    nodes++;

    // 6) Are we in check?
    bool inCheck = is_square_under_attack(
        pos,
        (pos.colour_to_move == white ?
            get_lsb_index(pos.piece_bitboards[K]) :
            get_lsb_index(pos.piece_bitboards[k])),
        pos.colour_to_move ^ 1
    );

    // If in check, extend depth by 1
    if (inCheck)
    {
        depth++;
    }

    // 7) Compute static evaluation
    static_eval = evaluate(pos);

    // --------------------------------------
    // 8) Razoring (shallow depth, not in check, non-PV)
    // --------------------------------------
    if (!inCheck && !isPvNode && depth <= 3)
    {
        int razorEval = evaluate(pos) + 125;
        // If the position is very likely losing (or not better) vs beta, do a quick check
        if (razorEval < beta)
        {
            if (depth == 1)
            {
                int razorScore = quiescence(pos, td, alpha, beta, ply);
                if (razorScore < beta)
                {
                    // Return the best we can do here
                    return std::max(razorScore, razorEval);
                }
            }
            razorEval += 175;
            if (razorEval < beta && depth <= 2)
            {
                int razorScore = quiescence(pos, td, alpha, beta, ply);
                if (razorScore < beta)
                {
                    return std::max(razorScore, razorEval);
                }
            }
        }
    }

    // --------------------------------------
    // 9) Reverse Futility Pruning (RFP)
    //    Often called "static-nullmove" or "futility"
    // --------------------------------------
    if (!inCheck && !isPvNode && depth < 3)
    {
        int eval_margin = 64 * depth;
        // if static eval already big enough to exceed beta
        if (static_eval - eval_margin >= beta)
        {
            return static_eval - eval_margin;
        }
    }

    // --------------------------------------
    // 10) Null-move pruning
    // --------------------------------------
    // Conditions to do a null-move:
    // - not in check
    // - depth >= a threshold (e.g., 3)
    // - not a PV node
    // - side to move has enough material (not in an endgame with no minors/majors)
    // - allowNullMovePruning not disabled
    if (!inCheck && depth >= 3 && !isPvNode && !noMajorsOrMinorsPieces(pos) && ply && td.allowNullMovePruning)
    {
        pos.copyBoard(ply);
        td.repetition_index++;
        td.repetition_table[td.repetition_index] = pos.zobristKey;

        // Remove en-passant possibility from the key
        if (pos.enpassant != null_sq)
            pos.zobristKey ^= pos.enpassant_hashkey[pos.enpassant];
        pos.enpassant = null_sq;

        // Switch side
        pos.colour_to_move ^= 1;
        pos.zobristKey ^= pos.colour_to_move_hashkey;

        // Null-move search with reduced depth
        int reduction = 2+(depth/6);
        td.allowNullMovePruning = false;
        score = -negamax(pos, td, depth - 1 - reduction, -beta, -beta + 1, ply + 1, isPvNode);
        td.allowNullMovePruning = true;

        td.repetition_index--;
        pos.restoreBoard(ply);

        if (stopped == 1)
            return alpha;

        // If this "fake pass" search fails high, then cut
        if (score >= beta)
        {
            return beta;
        }
    }

    // 11) Generate moves
    std::vector<int> moves = generate_moves(pos);

    // If no moves, it's either checkmate or stalemate
    if (moves.empty())
    {
        if (inCheck)
        {
            // mate score (negative perspective => lost position)
            return -mateVal + ply;
        }
        else
        {
            // stalemate
            return 0;
        }
    }

    // 12) If we had a best move from TT, ensure it gets sorted first
    if (td.follow_pv_flag)
        score_pv(moves, td, ply);

    sort_moves(pos, td, moves, bestMove, ply);

    // We are about to search each move
    int valid_moves = 0;
    int moves_searched = 0;

    // 13) Search each move (LMR, LMP, PVS logic, etc.)
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

        // -------------------------------------------
        // Principal Variation Search logic
        // -------------------------------------------
        if (moves_searched == 0)
        {
            // First move: full-window search
            score = -negamax(pos, td, depth - 1, -beta, -alpha, ply + 1, true);
        }
        else
        {
            // -----------------------------
            // (A) Basic Futility on quiet moves
            //     e.g. if depth is small, not giving check, not a capture, etc.
            // -----------------------------
            bool givesCheck = is_square_under_attack(
                                 pos,
                                 (pos.colour_to_move == white) ? get_lsb_index(pos.piece_bitboards[K])
                                                               : get_lsb_index(pos.piece_bitboards[k]),
                                 pos.colour_to_move ^ 1);

            bool allowFutilityPrune = false;
            if (ply && !isPvNode && (depth <= 3))
            {
                if ((static_eval + futility_margin(depth)) <= alpha)
                {
                    allowFutilityPrune = true;
                }
            }

            if (allowFutilityPrune && !givesCheck &&
                (get_move_piece(move) != P) && (get_move_piece(move) != p) &&
                !get_promoted_piece(move) && !get_is_move_castling(move) &&
                !get_is_capture_move(move))
            {
                td.repetition_index--;
                pos.restoreBoard(ply);
                moves_searched++;
                continue;
            }

            // -----------------------------
            // (B) Late Move Pruning (LMP)
            //     If depth is small, not in check, not a capture,
            //     and we've already searched "too many" quiet moves
            // -----------------------------
            if (ply && depth <= 3 && !isPvNode && !inCheck &&
                !get_is_capture_move(move) &&
                (valid_moves > LateMovePruning_factors[depth]))
            {
                td.repetition_index--;
                pos.restoreBoard(ply);
                moves_searched++;
                continue;
            }

            // -----------------------------
            // (C) Late Move Reductions (LMR)
            // -----------------------------
            if (valid_moves >= full_depth_moves &&
                depth >= reduction_limit &&
                !inCheck &&
                get_is_capture_move(move) == 0 &&
                get_promoted_piece(move) == 0)
            {
                // Reduced search
                int reduction = (depth >= 5) ? 2 : 1;
                score = -negamax(pos, td, depth - reduction, -alpha - 1, -alpha, ply + 1, false);
            }
            else
            {
                // Force a normal re-search with a null window to see if it fails high
                score = alpha + 1;
            }

            // If we got a score above alpha after LMR attempt...
            if (score > alpha)
            {
                // Do a PVS re-search with a narrow window
                score = -negamax(pos, td, depth - 1, -alpha - 1, -alpha, ply + 1, false);

                // If it's still above alpha but not >= beta, do a full re-search
                if (score > alpha && score < beta)
                {
                    score = -negamax(pos, td, depth - 1, -beta, -alpha, ply + 1, isPvNode);
                }
            }
        }

        td.repetition_index--;
        pos.restoreBoard(ply);
        moves_searched++;

        if (stopped == 1)
            return alpha;

        // Check if this move improved alpha
        if (score > alpha)
        {
            alpha = score;
            bestMove = move;
            hashFlag = hashFlagEXACT; // PV node

            // Update history for quiet moves
            if (get_is_capture_move(move) == 0)
            {
                td.history_moves[get_move_piece(move)][get_move_target(move)] += depth;
            }

            // Update PV
            td.pv_table[ply][ply] = move;
            for (int nextPly = ply + 1; nextPly < td.pv_depth[ply + 1]; nextPly++)
            {
                td.pv_table[ply][nextPly] = td.pv_table[ply + 1][nextPly];
            }
            td.pv_depth[ply] = td.pv_depth[ply + 1];

            // Fail-hard beta cutoff
            if (alpha >= beta)
            {
                tt.store(pos, depth, beta, hashFlagBETA, bestMove, curr_hash_age, ply);

                // killer move
                if (get_is_capture_move(move) == 0)
                {
                    td.killer_moves[1][ply] = td.killer_moves[0][ply];
                    td.killer_moves[0][ply] = move;
                }
                return beta;
            }
        }
    } // end of move loop

    // if no valid moves
    if (valid_moves == 0)
    {
        // (should not happen because we handle moves.empty() above,
        //  but just in case)
        if (inCheck)
            return -mateVal + ply;
        else
            return 0;
    }

    // 14) Store in TT and return
    tt.store(pos, depth, alpha, hashFlag, bestMove, curr_hash_age, ply);
    return alpha;
}

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

    int evaluation = evaluate(pos);

    // safety check for array bounds
    if (ply > MAX_DEPTH-1)
    {
        return evaluation;
    }

    // fail-hard beta cutoff
    if (evaluation >= beta)
        return beta; // fails high

    // found better move
    if (evaluation > alpha)
        alpha = evaluation; // principal variation PV node (best move)

    std::vector<int> moves = generate_moves(pos);
    sort_moves(pos, td, moves, 0, ply);

    for (int move : moves)
    {
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

        // found better move
        if (score > alpha)
        {
            alpha = score; // principal variation PV node (best move)

            // fail-hard beta cutoff
            if (score >= beta)
                return beta; // fails high
        }
    }

    // move fails low (<= alpha)
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

    // Create local thread data
    ThreadData td;

    // Initialize repetition history in ThreadData from Position
    for (int j = 0; j <= pos.repetition_index && j < td.repetition_table.size(); j++)
    {
        td.repetition_table[j] = pos.repetition_table[j];
    }
    td.repetition_index = pos.repetition_index;
    td.fifty_move = pos.fifty_move;

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
        score = negamax(pos, td, curr_depth, alpha, beta, 0, true);

        // aspiration window
        if (score <= alpha || score >= beta)
        {
            alpha = -INFINITY;
            beta  = INFINITY;
            continue;
        }

        // set up the window for the next iteration
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

    stopped = 1; // fixes zero eval blundering bug
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
            return 20000; // give pv move priority
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
    else // quiet moves
    {
        if (td.killer_moves[0][ply] == move)
            return 9000;
        else if (td.killer_moves[1][ply] == move)
            return 8000;
        else if (get_promoted_piece(move) == Q || get_promoted_piece(move) == q)
            return mvv_lva[get_move_piece(move)][get_move_target(move)] + 100;
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
    for (int i = 0; i <= td.repetition_index; i++)
    {
        if (td.repetition_table[i] == pos.zobristKey)
            return 1;
    }
    return 0;
}

int futility_margin(int depth)
{
    int margins[4] = {0, 120, 200, 300};
    return margins[depth];
}