#include "threading.h"
#include "search.h"         // for negamax, quiescence, etc.
#include "uci.h"            // for 'stopped', 'communicate()', etc.
#include "move_helpers.h"
#include "transposition_table.h"
#include "globals.h"
#include <thread>
#include <iostream>
#include <atomic>

// Global flag and timing variable.
std::atomic<bool> stop_threads(false);
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
}

/*
 * SMP_Thread default constructor.
 * This is needed for allocating an array of SMP_Thread objects on the stack.
 */
SMP_Thread::SMP_Thread() : rootPos(), td() {
    // Default construction of rootPos and td.
}

/*
 * SMP_Thread parameterized constructor.
 * Copies the given root position.
 */
SMP_Thread::SMP_Thread(const thrawn::Position* pos) : rootPos(*pos), td() {
}

/**
 * smp_worker_thread_func
 *
 * Each worker thread uses its own SMP_Thread object (which contains a local copy
 * of the root position and its search data) to perform iterative deepening.
 */
void smp_worker_thread_func(SMP_Thread* threadObj, int threadID, int maxDepth)
{
    // Use the local copy of the position and the thread's search data.
    thrawn::Position* pos = &threadObj->rootPos;
    ThreadData* td = &threadObj->td;

    int alpha = -INFINITY;
    int beta  =  INFINITY;
    int score = 0;
    
    // Perform iterative deepening from depth 1 to maxDepth.
    for (int curr_depth = 1; curr_depth <= maxDepth; curr_depth++)
    {
        if (stop_threads.load() || stopped == 1)
            break;
        
        td->follow_pv_flag = true;
        
        // Perform the search at the current depth.
        score = negamax(pos, td, curr_depth, alpha, beta, 0, true);

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
        
        // Only thread 0 prints the search info.
        if (threadID == 0)
        {   
            if (td->pv_depth[0])
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
                for (int i = 0; i < td->pv_depth[0]; i++)
                {
                    print_move(td->pv_table[0][i]);
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
 * Creates up to MAX_THREADS SMP_Thread objects (each with its own copy of the root position)
 * on the stack and spawns a worker thread for each. After all threads finish, the best move is printed.
 */
void search_position_threaded(const thrawn::Position* rootPos, int maxDepth, int numThreads)
{
    // Reset stop flags and counters.
    stop_threads.store(false);
    stopped = 0;
    nodes = 0;
    globalSearchStartTime = get_time_ms();

    tt->incrementAge();

    // Limit the number of threads to MAX_THREADS.
    if (numThreads > MAX_THREADS)
        numThreads = MAX_THREADS;

    // Allocate SMP_Thread objects on the stack.
    std::vector<SMP_Thread> threadObjs;
    threadObjs.reserve(numThreads);
    for (int i = 0; i < numThreads; i++)
    {
        threadObjs.emplace_back(rootPos);
    }

    // Create an array of std::thread objects (also on the stack).
    std::vector<std::thread> workerPool;
    workerPool.reserve(numThreads);
    for (int i = 0; i < numThreads; i++)
    {
        workerPool.emplace_back(smp_worker_thread_func, &threadObjs[i], i, maxDepth);
    }

    // Wait for all worker threads to complete.
    for (int i = 0; i < numThreads; i++)
    {
        workerPool[i].join();
    }

    // After all threads finish, output the best move (taken from thread 0’s search data).
    std::cout << "bestmove ";
    print_move(threadObjs[0].td.pv_table[0][0]);
    std::cout << std::endl;

    // Signal that the search is complete.
    stopped = 1;
}
