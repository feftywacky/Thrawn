#include "threading.h"
#include "search.h"     // for negamax, quiescence, etc.
#include "uci.h"        // for 'stopped', 'communicate()'
#include "move_helpers.h"
#include "transposition_table.h"
#include <thread>
#include <iostream>
#include <mutex>
#include <vector>
#include <atomic>

std::atomic<bool> stop_threads(false);
static std::mutex printMutex;
static std::mutex bestMoveMutex;
static int globalBestMove  = 0;
static int globalSearchStartTime = 0;

/*
 * ThreadData constructor: 
 * Allocates the arrays to the needed sizes for up to MAX_DEPTH=64.
 */
ThreadData::ThreadData()
{
    pv_depth.resize(64, 0);
    pv_table.resize(64, std::vector<int>(64, 0));

    killer_moves.resize(2, std::vector<int>(64, 0));
    history_moves.resize(12, std::vector<int>(64, 0));

    repetition_table.resize(1028, 0ULL);
    repetition_index = 0;
    fifty_move       = 0;

    follow_pv_flag = false;
    score_pv_flag  = false;
    allowNullMovePruning = true;
}

/**
 * smp_worker_thread_func
 *
 * Each thread makes its own copy of the root Position and does an iterative deepening
 * search from depth=1..maxDepth. The master thread (threadID=0) reports the best
 * move at each iteration. Other threads share the same transposition table to aid thread 0 to search
 *
 * Lazy SMP does not require a sophisticated split at the root;
 * we simply let multiple threads run the same iterative deepening and share the TT.
 */
static void smp_worker_thread_func(const thrawn::Position& rootPos, ThreadData td, int threadID, int maxDepth)
{
    thrawn::Position pos = rootPos;
    int alpha = -INFINITY;
    int beta  =  INFINITY;
    int score = 0;
    
    // Iterative deepening from 1..maxDepth
    for (int curr_depth = 1; curr_depth <= maxDepth; curr_depth++)
    {
        if (stop_threads.load() || stopped == 1)
            break;
        
        // For PV ordering
        td.follow_pv_flag = true;
        
        // Call negamax with the board (pos) and the thread's SearchData.
        score = negamax(pos, td, curr_depth, alpha, beta, 0, true);
        
        // Only thread 0 logs the result.
        if (threadID == 0)
        {
            globalBestMove = td.pv_table[0][0];
            
            if (td.pv_depth[0])
            {
                int currentTime = get_time_ms() - globalSearchStartTime;
                if (score > -mateVal && score < -mateScore)
                {
                    std::cout << "info score mate " << -(score + mateVal) / 2 - 1
                            << " depth " << curr_depth
                            << " nodes " << nodes
                            << " time " << currentTime
                            << " pv ";
                }
                else if (score > mateScore && score < mateVal)
                {
                    std::cout << "info score mate " << (mateVal - score) / 2 + 1
                            << " depth " << curr_depth
                            << " nodes " << nodes
                            << " time " << currentTime
                            << " pv ";
                }
                else
                {
                    std::cout << "info score cp " << score
                            << " depth " << curr_depth
                            << " nodes " << nodes
                            << " time " << currentTime
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
    }
}

/**
 * search_position_threaded
 *
 * Creates 'numThreads' worker threads. The master thread (ID=0) eventually
 * prints out "bestmove X" after all threads are done or a stop is triggered.
 */
void search_position_threaded(const thrawn::Position &rootPos, int maxDepth, int numThreads)
{
    globalBestMove  = 0;

    // Reset stop flags and counters
    stop_threads.store(false);
    stopped = 0;
    nodes   = 0;
    globalSearchStartTime = get_time_ms();
    curr_hash_age++;

    // Spawn threads
    std::vector<std::thread> workerPool;
    workerPool.reserve(numThreads);

    for (int i = 0; i < numThreads; i++)
    {
        workerPool.emplace_back(smp_worker_thread_func, rootPos, ThreadData(), i, maxDepth);
    }

    // Wait for all threads
    for (auto &t : workerPool)
        t.join();

    // After all threads finish, output the final best move (from thread 0’s results).
    {
        std::lock_guard<std::mutex> lock(printMutex);
        std::cout << "bestmove ";
        print_move(globalBestMove);
        std::cout << std::endl;
    }

    // Signal that we are done
    stopped = 1;
}
