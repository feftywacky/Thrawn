#include "threading.h"
#include "search.h"         // for negamax, quiescence, etc.
#include "uci.h"            // for 'stopped', 'communicate()', etc.
#include "move_helpers.h"
#include "transposition_table.h"
#include "globals.h"
#include <thread>
#include <iostream>
#include <atomic>

static int globalSearchStartTime = 0;

/*
 * ThreadData constructor:
 * Initializes all fixed‑size arrays to zero and sets flags.
 */
ThreadData::ThreadData() {
    pv_length.fill(0);
    for (auto &row : pv_table)
        row.fill(0);
    for (auto &row : killer_moves)
        row.fill(0);
    for (auto &row : history_moves)
        row.fill(0);

    follow_pv_flag = false;
    score_pv_flag  = false;
    allowNullMovePruning = true;

    nodes = 0;

    final_depth = 0;
    final_score = 0;
}

/*
 * Reset the thread data between searches.
 */
void ThreadData::resetThreadData() {
    pv_length.fill(0);
    for (auto &row : pv_table)
        row.fill(0);
    for (auto &row : killer_moves)
        row.fill(0);
    for (auto &row : history_moves)
        row.fill(0);

    follow_pv_flag = false;
    score_pv_flag  = false;
    allowNullMovePruning = true;

    nodes = 0;

    final_depth = 0;
    final_score = 0;
}

/**
 * smp_worker_thread_func
 * Each worker thread performw iterative deepening.
 * Use the local copy of the position and the thread's search data.
 */
void smp_worker_thread_func(thrawn::Position pos, int threadID, int maxDepth)
{
    ThreadData* td = &threadDatas[threadID];
    int alpha = -INFINITY;
    int beta  =  INFINITY;
    int score = 0;

    // Perform iterative deepening from depth 1 to maxDepth.
    for (int curr_depth = 1; curr_depth <= maxDepth; curr_depth++)
    {
        if (stopped == 1)
            break;
        
        td->follow_pv_flag = true;

        std::array<int, MAX_DEPTH> backup_pv = td->pv_table[0];
        int backup_pv_length = td->pv_length[0];
        
        // Perform the search at the current depth.
        score = negamax(&pos, td, curr_depth, alpha, beta);

        // If the score falls outside the aspiration window, widen the window and continue.
        if ((score <= alpha) || (score >= beta))
        {
            td->pv_table[0] = backup_pv;
            td->pv_length[0] = backup_pv_length;
            alpha = -INFINITY;
            beta = INFINITY;
            continue;
        }

        // Update the aspiration window.
        alpha = score - 50;
        beta = score + 50;

        td->final_depth = curr_depth;
        td->final_score = score;
        
        // print an info line from the master thread (thread 0)
        if (threadID == 0)
        {   
            int currentTime = get_time_ms() - globalSearchStartTime;
            std::cout << "info depth " << curr_depth
                      << " nodes " << total_nodes
                      << " time " << currentTime;
            
            // Determine whether to report a mate score or a centipawn score.
            if (score > -mateVal && score < -mateScore)
            {
                std::cout << " score mate " << -(score + mateVal) / 2 - 1;
            }
            else if (score > mateScore && score < mateVal)
            {
                std::cout << " score mate " << (mateVal - score) / 2 + 1;
            }
            else
            {
                std::cout << " score cp " << score;
            }
            
            // Print the principal variation if it exists; otherwise print a default message.
            if (td->pv_length[0] > 0)
            {
                std::cout << " pv ";
                for (int i = 0; i < td->pv_length[0]; i++)
                {
                    print_move(td->pv_table[0][i]);
                    std::cout << " ";
                }
            }
            else
            {
                std::cout << " pv (none)";
            }
            
            std::cout << "\n";
            std::cout.flush();
        }
    }
}

/**
 * entry point to search
 */
void search_position_threaded(thrawn::Position* rootPos, int maxDepth, int numThreads)
{
    // Reset stop flags and counters.
    total_nodes = 0;
    stopped = 0;
    globalSearchStartTime = get_time_ms();

    tt->incrementAge();

    // Limit the number of threads to MAX_THREADS.
    if (numThreads > MAX_THREADS)
        numThreads = MAX_THREADS;
    
    for(int i=0;i<numThreads;i++)
    {
        threadDatas[i].resetThreadData();
    }

    // Create an array of std::thread objects (also on the stack).
    std::vector<std::thread> workerPool;
    workerPool.reserve(numThreads);
    for (int i = 0; i < numThreads; i++)
    {
        workerPool.emplace_back(smp_worker_thread_func, *rootPos, i, maxDepth);
    }

    // Wait for all worker threads to complete.
    for (int i = 0; i < numThreads; i++)
    {
        workerPool[i].join();
    }

    // After all threads have finished, select the best result.
    ThreadData* best_td = select_best_thread(threadDatas, numThreads);
    
    // Print the total nodes and best move.
    //std::cout << "total nodes across all threads " << total_nodes << std::endl;
    std::cout << "bestmove ";
    print_move(best_td->pv_table[0][0]);
    std::cout << "\n";
    std::cout.flush();
}

/**
 * select_best_thread
 * Choose the best result among the threadDatas (mimics Ethereal’s select_from_threads).
 *
 * A thread is considered better than another if:
 *   [1] They have equal depth and the thread’s score is higher.
 *   [2] The thread has a mate score (i.e. its absolute score is greater than mateScore) and a better mate proximity.
 *   [3] The thread has a greater depth and (a higher score or the current best isn’t a mate score).
 */
ThreadData* select_best_thread(ThreadData threadDatas[], int numThreads) {
    ThreadData* best_td = &threadDatas[0];
    int res_thread_id = 0;
    for (int i = 1; i < numThreads; i++) {
        int best_depth = best_td->final_depth;
        int best_score = best_td->final_score;
        int this_depth = threadDatas[i].final_depth;
        int this_score = threadDatas[i].final_score;
        
        if ((this_depth == best_depth && this_score > best_score)
            || (std::abs(this_score) > mateScore && std::abs(this_score) > std::abs(best_score))
            || (this_depth > best_depth && (this_score > best_score || std::abs(best_score) < mateScore)))
        {
            best_td = &threadDatas[i];
            res_thread_id = i;
        }
    }

    // the case where the best thread is not the main thread (thread 0)
    if(best_td!=&threadDatas[0])
    {   
        std::cout<<"thread "<<res_thread_id<<" is better"<<"\n";
        int currentTime = get_time_ms() - globalSearchStartTime;
        std::cout << "info depth " << best_td->final_depth
                  << " nodes " << total_nodes
                  << " time " << currentTime
                  << " score ";

        // Print mate or centipawn score
        if (best_td->final_score > mateScore) {
            std::cout << "mate " << (mateVal - best_td->final_score) / 2 + 1;
        } else if (best_td->final_score < -mateScore) {
            std::cout << "mate " << -(best_td->final_score + mateVal) / 2 - 1;
        } else {
            std::cout << "cp " << best_td->final_score;
        }

        // Print the principal variation (PV)
        if (best_td->pv_length[0] > 0) {
            std::cout << " pv ";
            for (int i = 0; i < best_td->pv_length[0]; i++) {
                print_move(best_td->pv_table[0][i]);
                std::cout << " ";
            }
        } else {
            std::cout << " pv (none)";
        }

        std::cout << "\n";
        std::cout.flush();
    }
    return best_td;
}