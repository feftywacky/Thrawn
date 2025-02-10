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
    pv_depth.fill(0);
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
}

/*
 * Reset the thread data between searches.
 */
void ThreadData::resetThreadData() {
    pv_depth.fill(0);
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
        
        if (threadID == 0)
            td->follow_pv_flag = true;
        
        // Perform the search at the current depth.
        score = negamax(&pos, td, curr_depth, alpha, beta);

        // If the score falls outside the aspiration window, widen the window and continue.
        if ((score <= alpha) || (score >= beta))
        {
            alpha = -INFINITY;
            beta = INFINITY;
            continue;
        }

        // Update the aspiration window.
        alpha = score - 50;
        beta = score + 50;
        
        // Always print an info line from the master thread (thread 0)
        if (threadID == 0)
        {   
            int currentTime = get_time_ms() - globalSearchStartTime;
            std::cout << "info depth " << curr_depth
                      << " nodes " << td->nodes
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
            if (td->pv_depth[0] > 0)
            {
                std::cout << " pv ";
                for (int i = 0; i < td->pv_depth[0]; i++)
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
            std::cout.flush();  // Ensure immediate output
        }
    }
    
    // When the search is complete, the master thread prints the best move.
    if (threadID == 0)
    {
        std::cout << "total nodes across all threads " << total_nodes << std::endl;
        std::cout << "bestmove ";
        print_move(td->pv_table[0][0]);
        std::cout << "\n";
        std::cout.flush();
        stopped = 1;
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
}

void search_pos_single(thrawn::Position* pos, int depth)
{
    
    ThreadData* td = new ThreadData();
    total_nodes = 0;
    stopped = 0;
    int score = 0;
    int alpha = -INFINITY;
    int beta = INFINITY;
    
    int start = get_time_ms();

    // iterative deepening
    for (int curr_depth = 1; curr_depth <= depth; curr_depth++)
    {
        // time is up
        if (stopped == 1)
        {
            break;
        }

        td->follow_pv_flag = true;
        score = negamax(pos, td, curr_depth, alpha, beta);

        // aspiration window
        if ((score <= alpha) || (score >= beta))
        {
            alpha = -INFINITY;
            beta = INFINITY;
            continue;
        }

        // set up the window for the next iteration
        alpha = score - 50;
        beta = score + 50;

        // if pv exist
        if (td->pv_depth[0])
        {
            if (score > -mateVal && score < -mateScore)
                std::cout << "info score mate " << -(score + mateVal) / 2 - 1
                          << " depth " << curr_depth
                          << " nodes " << total_nodes
                          << " time " << static_cast<unsigned int>(get_time_ms() - start)
                          << " pv ";
            else if (score > mateScore && score < mateVal)
                std::cout << "info score mate " << (mateVal - score) / 2 + 1
                          << " depth " << curr_depth
                          << " nodes " << total_nodes
                          << " time " << static_cast<unsigned int>(get_time_ms() - start)
                          << " pv ";
            else
                std::cout << "info score cp " << score
                          << " depth " << curr_depth
                          << " nodes " << total_nodes
                          << " time " << static_cast<unsigned int>(get_time_ms() - start)
                          << " pv ";

            for (int i = 0; i < td->pv_depth[0]; i++)
            {
                print_move(td->pv_table[0][i]);
                std::cout << " ";
            }
            std::cout << "\n";
        }

        std::cout<<"ply: "<<total_nodes<<endl;
    }

    std::cout << "bestmove ";
    print_move(td->pv_table[0][0]);
    std::cout << "\n";

    stopped = 1; // fixes zero eval blundering bug
}