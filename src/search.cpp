#include "search.h"
#include "evaluation.h"
#include "move_generator.h"
#include "move_helpers.h"
#include "uci.h"
#include "zobrist_hashing.h"
#include "transposition_table.h"
#include "position.h"
#include <iostream>
#include <vector>
#include <algorithm>
#include <numeric>
#include <atomic>
#include <thread>

using namespace std;

/*
some notes for negamax
3 types
- fail high: causes beta cut-off
- fail low: don't increase alpha
- pv nodes: increase alpha
*/

// -------------------------------------------
// Removed the global ply here:
// int ply;            // [REMOVED]
// -------------------------------------------
uint64_t nodes = 0;

std::vector<int> pv_depth(MAX_DEPTH);
std::vector<std::vector<int>> pv_table(MAX_DEPTH, std::vector<int>(MAX_DEPTH));
std::vector<std::vector<int>> killer_moves(2, std::vector<int>(MAX_DEPTH));
std::vector<std::vector<int>> history_moves(12, std::vector<int>(64));

// pv sorting
bool follow_pv_flag = false;
bool score_pv_flag = false;

bool allowNullMovePruning = true;
bool allowFutilityPruning = false;

std::array<int, 4> LateMovePruning_factors = {0, 8, 12, 24};
int RFP_factor = 64;

// repetition
std::vector<uint64_t> repetition_table(1028);
int repetition_index = 0;
int fifty_move = 0;

// threading
std::atomic<bool> stop_threads(false);


// ---------------------------------------------------
// Changed signature: now we have (pos, depth, alpha, beta, ply).
// ---------------------------------------------------
int negamax(thrawn::Position& pos, int depth, int alpha, int beta, int ply)
{
    int score = 0;
    int bestMove = 0;
    int hashFlag = hashFlagALPHA;
    int static_eval = 0;

    // init local pv
    pv_depth[ply] = ply;

    // check repetition or 50-move rule
    if (ply && (isRepetition(pos) || fifty_move >= 100))
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
        return quiescence(pos, alpha, beta, ply);
    }

    if (ply > MAX_DEPTH - 1)
    {
        std::cout << "array overflow at max depth: " << ply << endl;
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
        pos.copyBoard(ply);          // use local ply
        repetition_index++;
        repetition_table[repetition_index] = pos.zobristKey;

        if (pos.enpassant != null_sq)
            pos.zobristKey ^= pos.enpassant_hashkey[pos.enpassant];
        pos.enpassant = null_sq;

        pos.colour_to_move ^= 1;
        pos.zobristKey ^= pos.colour_to_move_hashkey;

        // do the reduced-depth search
        score = -negamax(pos, depth - 1 - 2, -beta, -beta + 1, ply + 1);

        repetition_index--;
        pos.restoreBoard(ply);       // use local ply

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
                int razor_score = quiescence(pos, alpha, beta, ply);
                return (razor_score > score) ? razor_score : score;
            }
            score += 175;
            if (score < beta && depth <= 2)
            {
                int razor_score = quiescence(pos, alpha, beta, ply);
                if (razor_score < beta)
                    return (razor_score > score) ? razor_score : score;
            }
        }
    }

    // generate moves
    vector<int> moves = generate_moves(pos);

    // first, see if the bestMove from TT or PV is present
    if (follow_pv_flag)
        score_pv(moves, ply);

    // now sort them (with local ply)
    sort_moves(pos, moves, bestMove, ply);

    int moves_searched = 0;

    for (int move : moves)
    {
        pos.copyBoard(ply);       // local ply
        repetition_index++;
        repetition_table[repetition_index] = pos.zobristKey;

        // pass ply to make_move
        if (make_move(pos, move, all_moves, ply) == 0)
        {
            repetition_index--;
            pos.restoreBoard(ply);
            continue;
        }

        valid_moves++;

        int move_score;
        if (moves_searched == 0)
        {
            // full window search
            move_score = -negamax(pos, depth - 1, -beta, -alpha, ply + 1);
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
                move_score = -negamax(pos, depth - 2, -alpha - 1, -alpha, ply + 1);
            }

            if (move_score > alpha)
            {
                // do a PVS search with narrow window
                move_score = -negamax(pos, depth - 1, -alpha - 1, -alpha, ply + 1);

                if (move_score > alpha && move_score < beta)
                {
                    // full window
                    move_score = -negamax(pos, depth - 1, -beta, -alpha, ply + 1);
                }
            }
        }

        repetition_index--;
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
                history_moves[get_move_piece(move)][get_move_target(move)] += depth;
            }

            // update PV
            pv_table[ply][ply] = move;
            for (int nextPly = ply + 1; nextPly < pv_depth[ply + 1]; nextPly++)
            {
                pv_table[ply][nextPly] = pv_table[ply + 1][nextPly];
            }
            pv_depth[ply] = pv_depth[ply + 1];

            // beta cutoff
            if (alpha >= beta)
            {
                writeToHashMap(pos, depth, beta, hashFlagBETA, bestMove, ply);

                // killer move
                if (get_is_capture_move(move) == 0)
                {
                    killer_moves[1][ply] = killer_moves[0][ply];
                    killer_moves[0][ply] = move;
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

    writeToHashMap(pos, depth, alpha, hashFlag, bestMove,ply);
    return alpha;
}


// ---------------------------------------------------
// Changed signature to add (ply).
// ---------------------------------------------------
int quiescence(thrawn::Position& pos, int alpha, int beta, int ply)
{
    if ((nodes & 2047) == 0)
    {
        communicate();
    }

    nodes++;

    // safety check
    if (ply >= MAX_DEPTH)
    {
        return evaluate(pos);
    }

    int evaluation = evaluate(pos);

    if (evaluation >= beta)
        return beta;

    if (evaluation > alpha)
        alpha = evaluation;

    vector<int> moves = generate_moves(pos);

    // no bestMove needed at quiescence, so pass 0
    sort_moves(pos, moves, 0, ply);

    for (int move : moves)
    {
        // only captures
        if (!get_is_capture_move(move)) 
            continue;

        pos.copyBoard(ply);
        repetition_index++;
        repetition_table[repetition_index] = pos.zobristKey;

        if (!make_move(pos, move, only_captures, ply))
        {
            repetition_index--;
            pos.restoreBoard(ply);
            continue;
        }

        int score = -quiescence(pos, -beta, -alpha, ply + 1);

        repetition_index--;
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

// repetition check (unchanged, but note we do not need ply here)
int isRepetition(thrawn::Position& pos)
{
    for (int i = 0; i < repetition_index; i++)
    {
        if (repetition_table[i] == pos.zobristKey)
            return 1;
    }
    return 0;
}

// ----------------------------------------------------------
// In search_position, we now call negamax(..., 0) to start ply=0
// ----------------------------------------------------------
void search_position(thrawn::Position& pos, int depth)
{
    int start = get_time_ms();
    nodes = 0;
    stopped = 0;

    follow_pv_flag = false;
    score_pv_flag = false;

    allowNullMovePruning = false;
    allowFutilityPruning = false;

    pv_depth.assign(MAX_DEPTH, 0);
    for (auto &row : pv_table)
        row.assign(MAX_DEPTH, 0);
    for (auto &row : killer_moves)
        row.assign(MAX_DEPTH, 0);
    for (auto &row : history_moves)
        row.assign(64, 0);

    int score = 0;
    int alpha = -INFINITY;
    int beta = INFINITY;

    // iterative deepening
    for (int curr_depth = 1; curr_depth <= depth; curr_depth++)
    {
        if (stopped == 1)
            break;

        follow_pv_flag = true;

        // ----------------------------
        // Pass ply = 0 here
        // ----------------------------
        score = negamax(pos, curr_depth, alpha, beta, 0);

        // aspiration window
        if (score <= alpha || score >= beta)
        {
            alpha = -INFINITY;
            beta = INFINITY;
            continue;
        }

        alpha = score - 50;
        beta = score + 50;

        // if pv exists
        if (pv_depth[0])
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

            for (int i = 0; i < pv_depth[0]; i++)
            {
                print_move(pv_table[0][i]);
                std::cout << " ";
            }
            std::cout << "\n";
        }
    }

    std::cout << "bestmove ";
    print_move(pv_table[0][0]);
    std::cout << "\n";

    stopped = 1;
}

// Lazy SMP: unchanged logic, but still calls search_position. 
// All threads do the same search with local ply usage inside.
void search_position_threaded(thrawn::Position& pos, int depth, int numThreads)
{
    stop_threads.store(false, std::memory_order_relaxed);

    std::vector<std::thread> threads;
    threads.reserve(numThreads);

    for (int i = 0; i < numThreads; i++)
    {
        threads.emplace_back([&, depth]() {
            search_position(pos, depth);
        });
    }

    for (auto& th : threads)
    {
        if (th.joinable())
            th.join();
    }
}

// ----------------------------------------------------------
// Score a single move with local ply
// ----------------------------------------------------------
int score_move(thrawn::Position& pos, int move, int ply)
{
    // scoring pv
    if (score_pv_flag)
    {
        if (pv_table[0][ply] == move)
        {
            score_pv_flag = false;
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
        if (killer_moves[0][ply] == move)
            return 9000;
        else if (killer_moves[1][ply] == move)
            return 8000;
        else if (get_promoted_piece(move) == Q || get_promoted_piece(move) == q)
            return mvv_lva[get_move_piece(move)][get_move_target(move)] + 100;
        else
            return history_moves[get_move_piece(move)][get_move_target(move)];
    }

    return 0;
}

// ---------------------------------------------------------
// For ordering the PV move
// ---------------------------------------------------------
void score_pv(vector<int> &moves, int ply)
{
    follow_pv_flag = false;
    for (int move : moves)
    {
        if (pv_table[0][ply] == move)
        {
            score_pv_flag = true;
            follow_pv_flag = true;
        }
    }
}

// ---------------------------------------------------------
// Sort moves with local ply
// ---------------------------------------------------------
void sort_moves(thrawn::Position& pos, vector<int> &moves, int bestMove, int ply)
{
    const int n = static_cast<int>(moves.size());
    std::vector<int> scores(n);
    for (int i = 0; i < n; i++)
    {
        // If a move is the bestMove from TT, give it a high score
        if (moves[i] == bestMove)
            scores[i] = 30000;
        else
            scores[i] = score_move(pos, moves[i], ply);
    }

    quicksort_moves(moves, scores, 0, n - 1);
}

void print_move_scores(thrawn::Position& pos, const vector<int> &moves, int ply)
{
    for (int move : moves)
    {
        int s = score_move(pos, move, ply);
        if (s > 0)
        {
            if (s >= 20000)
            {
                print_move(move);
                std::cout << " pv/best: " << s << "\n";
            }
            else
            {
                print_move(move);
                std::cout << " " << s << "\n";
            }
        }
    }
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

int futility_margin(int depth)
{
    return 168 * depth;
}

int futility_move_count(int depth)
{
    return (3 + depth * depth) / 2;
}
