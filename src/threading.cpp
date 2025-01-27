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

/*
 * We'll track the global start time of the search so each thread
 * can print "time = ..." in the "info" lines consistently.
 */
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

/*
 * Worker function for each thread.
 *
 * Each thread runs an iterative deepening loop from depth=1..(maxDepth).
 *   - They check the global stop_threads flag.
 *   - If not stopping, they call negamax at the new iteration depth.
 *   - If they improve upon the global best data, they:
 *       1) Acquire a lock
 *       2) Update the global best depth, score, move
 *       3) Print an "info" line with the new PV
 */
static void search_thread_func(Thread &threadObj, int maxDepth, int threadID)
{
    thrawn::Position &pos = threadObj.rootPos;
    ThreadData &td        = threadObj.td;

    int alpha = -INFINITY;
    int beta  = INFINITY;

    // iterative deepening from 1..maxDepth
    for (int depth = 1; depth <= maxDepth; depth++)
    {
        if (stop_threads.load() || stopped == 1)
            break;

        // Age is incremented once per iteration so that TT entries
        // from deeper iteration can replace older entries.
        curr_hash_age++;

        // For PV ordering
        td.follow_pv_flag = true;

        int score = negamax(pos, td, depth, alpha, beta, 0, true);

        // aspiration window logic
        if (score <= alpha || score >= beta)
        {
            alpha = -INFINITY;
            beta  = INFINITY;
            // re-search at the same depth
            depth--;
            if (depth < 1) depth = 1; 
            continue;
        }

        alpha = score - 50;
        beta  = score + 50;

        // If the thread found a new PV at ply=0
        if (td.pv_depth[0] > 0)
        {
            std::lock_guard<std::mutex> lock(bestMoveMutex);

            // if this depth is better than the global or same depth but better score
            if (depth > globalBestDepth ||
               (depth == globalBestDepth && score > globalBestScore))
            {
                globalBestDepth = depth;
                globalBestScore = score;
                globalBestMove  = td.pv_table[0][0];

                // Print an "info" line like in single-thread
                int currentTime = get_time_ms() - globalSearchStartTime;
                std::cout << "info depth " << globalBestDepth;

                // Format the mate or cp score
                if (score > -mateVal && score < -mateScore)
                {
                    // negative mate
                    int mate_in = -(score + mateVal)/2 - 1;
                    std::cout << " score mate " << mate_in;
                }
                else if (score > mateScore && score < mateVal)
                {
                    // positive mate
                    int mate_in = (mateVal - score)/2 + 1;
                    std::cout << " score mate " << mate_in;
                }
                else
                {
                    // normal centipawn
                    std::cout << " score cp " << score;
                }

                std::cout << " nodes " << nodes
                          << " time " << currentTime
                          << " thread " << threadID
                          << " pv ";

                // print the new PV
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
 *  1) Reset global best data
 *  2) Create 'Thread' objects, each with a COPY of 'pos'
 *  3) Launch them in parallel
 *  4) Joins them
 *  5) Print final best move
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

    // Create local thread objects
    std::vector<Thread> threads;
    threads.reserve(numThreads);
    for (int i = 0; i < numThreads; i++)
    {
        threads.emplace_back(pos); // copy constructor
    }

    // Initialize repetition info from the main pos
    for (int i = 0; i < numThreads; i++)
    {
        for (int j = 0; j <= pos.repetition_index && j < (int)threads[i].td.repetition_table.size(); j++)
        {
            threads[i].td.repetition_table[j] = pos.repetition_table[j];
        }
        threads[i].td.repetition_index = pos.repetition_index;
        threads[i].td.fifty_move       = pos.fifty_move;
    }

    // Launch system threads
    std::vector<std::thread> pool;
    pool.reserve(numThreads);
    for (int i = 0; i < numThreads; i++)
    {
        pool.emplace_back(search_thread_func, std::ref(threads[i]), depth, i);
    }

    // Wait for all threads
    for (auto &t : pool)
        t.join();

    // Print the final bestmove from global data
    {
        std::lock_guard<std::mutex> lk(bestMoveMutex);
        std::cout << "bestmove ";
        print_move(globalBestMove);
        std::cout << std::endl;
    }

    stop_threads.store(true);
    stopped = 1;
}
