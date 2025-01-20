#include "threading.h"
#include "search.h"     // for negamax, quiescence, etc.
#include "uci.h"        // for 'stopped'
#include "move_helpers.h"
#include <thread>
#include <iostream>
#include <mutex>
#include <vector>
#include <atomic>

/*
 * We define the atomic stop signal here.
 * All threads check stop_threads.load() to see if they must stop.
 */
std::atomic<bool> stop_threads(false);

/*
 * A global best move/score/depth protected by a mutex
 * for parallel search.
 */
static std::mutex bestMoveMutex;
static int globalBestMove  = 0;
static int globalBestScore = -INFINITY;
static int globalBestDepth = 0;

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
}

/*
 * Worker function for each thread.
 * 
 * Each thread runs an iterative deepening loop from depth=1..N,
 * calling negamax(...) with its own local 'ThreadData' and 'rootPos'.
 */
static void search_thread_func(Thread &threadObj, int depth, int threadID)
{
    // references to local objects
    thrawn::Position &pos = threadObj.rootPos;
    ThreadData &td        = threadObj.td;

    int alpha      = -INFINITY;
    int beta       = INFINITY;
    int bestScore  = -INFINITY;
    int bestMove   = 0;

    // Iterative deepening
    for (int curr_depth = 1; curr_depth <= depth; curr_depth++)
    {
        if (stop_threads.load() || stopped == 1)
            break;

        // allow following PV in ordering
        td.follow_pv_flag = true;

        int score = negamax(pos, td, curr_depth, alpha, beta, 0);

        // aspiration window
        if (score <= alpha || score >= beta)
        {
            alpha = -INFINITY;
            beta  = INFINITY;
            curr_depth--;
            continue;
        }

        alpha = score - 50;
        beta  = score + 50;

        // if we have a PV at ply=0
        if (td.pv_depth[0] > 0)
        {
            bestScore = score;
            bestMove  = td.pv_table[0][0];

            // update global best if we improved
            std::lock_guard<std::mutex> lock(bestMoveMutex);

            if (curr_depth > globalBestDepth ||
               (curr_depth == globalBestDepth && bestScore > globalBestScore))
            {
                globalBestDepth = curr_depth;
                globalBestScore = bestScore;
                globalBestMove  = bestMove;

                // <<< ADDED >>>
                // Print an "info" line similar to single-thread logic
                int currentTime = get_time_ms() - globalSearchStartTime;
                std::cout << "info depth " << globalBestDepth;

                // If we want to handle 'mate' scores as in single-thread:
                if (bestScore > -mateVal && bestScore < -mateScore)
                {
                    // negative mate
                    int mate_in = -(bestScore + mateVal)/2 - 1;
                    std::cout << " score mate " << mate_in;
                }
                else if (bestScore > mateScore && bestScore < mateVal)
                {
                    // positive mate
                    int mate_in = (mateVal - bestScore)/2 + 1;
                    std::cout << " score mate " << mate_in;
                }
                else
                {
                    // normal centipawn
                    std::cout << " score cp " << bestScore;
                }

                // also show nodes and time
                std::cout << " nodes " << nodes
                          << " time " << currentTime 
                          << " thread " << threadID
                          << " pv ";

                // print the PV
                for (int i = 0; i < td.pv_depth[0]; i++)
                {
                    print_move(td.pv_table[0][i]);
                    std::cout << " ";
                }
                std::cout << std::endl;
            }
        }
    }
}

/*
 * search_position_threaded:
 *  1) Resets global best data
 *  2) Creates 'Thread' objects, each with a COPY of 'pos'
 *  3) Launches them in parallel
 *  4) Joins them
 *  5) Prints final best move
 */
void search_position_threaded(const thrawn::Position &pos, int depth, int numThreads)
{
    {
        std::lock_guard<std::mutex> lk(bestMoveMutex);
        globalBestMove  = 0;
        globalBestScore = -INFINITY;
        globalBestDepth = 0;
    }
    stop_threads.store(false);
    stopped = 0;
    nodes   = 0;

    globalSearchStartTime = get_time_ms();

    // Create the Thread objects
    // Each thread gets its own copy of 'pos'
    std::vector<Thread> threads;
    threads.reserve(numThreads);
    for (int i = 0; i < numThreads; i++)
    {
        threads.emplace_back(pos); // copy pos here
    }

    // Launch system threads
    std::vector<std::thread> pool;
    pool.reserve(numThreads);
    for (int i = 0; i < numThreads; i++)
    {
        pool.emplace_back(search_thread_func, std::ref(threads[i]), depth, i);
    }

    // Wait for threads to finish
    for (auto &t : pool) 
        t.join();

    // Print the bestmove from global data
    {
        std::lock_guard<std::mutex> lk(bestMoveMutex);
        std::cout << "bestmove ";
        print_move(globalBestMove);
        std::cout << std::endl;
    }

    stop_threads.store(true);
    stopped = 1;
}
