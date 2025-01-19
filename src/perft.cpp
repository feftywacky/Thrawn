#include "perft.h"
#include "move_generator.h"
#include "move_helpers.h"
#include "constants.h"
#include "bitboard_helpers.h"
#include "zobrist_hashing.h"
#include "position.h"
#include "fen.h"
#include <chrono>
#include <iomanip>
#include <iostream>

long leaf_nodes;

void perft_search(thrawn::Position& pos, int depth) {
    if (depth == 0) {
        leaf_nodes++;
        return;
    }

    vector<int> moves = generate_moves(pos);

    for (int move : moves) {
        pos.copyBoard(depth);

        if (!make_move(pos, move, all_moves, depth))
            continue;

        perft_search(pos, depth - 1);

        pos.restoreBoard(depth);
    }
}

int perft_test(thrawn::Position& pos, int depth) {
    vector<int> moves = generate_moves(pos);
    size_t total_moves = moves.size();
    size_t moves_processed = 0;

    auto start = std::chrono::high_resolution_clock::now();

    std::cout << "Depth: " << depth << "\n";
    std::cout << "Progress: [";

    for (int move : moves) {
        pos.copyBoard(depth);

        if (!make_move(pos, move, all_moves, depth))
            continue;

        long cumulative_nodes = leaf_nodes;

        perft_search(pos, depth - 1);

        pos.restoreBoard(depth);
        moves_processed++;

        // Update the loading bar dynamically
        int bar_width = 30; // Width of the loading bar
        int progress = static_cast<int>((static_cast<float>(moves_processed) / total_moves) * bar_width);

        std::cout << "\rProgress: [";
        for (int i = 0; i < bar_width; ++i) {
            if (i < progress) {
                std::cout << "=";
            } else if (i == progress) {
                std::cout << ">";
            } else {
                std::cout << " ";
            }
        }
        std::cout << "] " << (moves_processed * 100 / total_moves) << "%";
        std::cout.flush();
    }
    std::cout << "\n";

    auto duration = std::chrono::high_resolution_clock::now() - start;
    auto duration_ms = std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
    double nodes_per_sec = (leaf_nodes * 1000.0) / duration_ms;

    std::cout << "\n===== Perft Results =====\n";
    std::cout << "Depth: " << depth << "\n";
    std::cout << "Total Nodes: " << leaf_nodes << "\n";
    std::cout << "Total Time: " << duration_ms << " ms\n";
    std::cout << std::fixed << std::setprecision(2);
    std::cout << "Nodes per Second: " << nodes_per_sec << "\n";
    std::cout << "=========================\n\n";

    int result = leaf_nodes;
    leaf_nodes = 0;

    return result;
}

void perft_run_unit_tests() {
    thrawn::Position p;
    int output_nodes = 0;

    struct Test {
        const char* fen;
        int depth;
        int expected_nodes;
    };

    Test tests[] = {
        {start_position, 6, 119060324},
        {position_2, 5, 193690690},
        {position_3, 7, 178633661},
        {position_4, 5, 15833292},
        {position_5, 5, 89941194},
        {position_6, 5, 164075551},
    };

    // ANSI escape codes for colors
    const std::string RESET = "\033[0m";
    const std::string GREEN = "\033[32m";
    const std::string RED = "\033[31m";

    int pass = 0;
    long total_nodes = 0;
    auto overall_start = std::chrono::high_resolution_clock::now();

    std::cout << "Running Perft Unit Tests\n";
    std::cout << "=========================\n";

    for (int i = 0; i < sizeof(tests) / sizeof(Test); i++) {
        parse_fen(p, tests[i].fen);

        std::cout << "Test " << i + 1 << ": FEN = " << tests[i].fen << "\n";
        std::cout << "Expected Nodes: " << tests[i].expected_nodes << "\n";

        auto test_start = std::chrono::high_resolution_clock::now();
        output_nodes = perft_test(p, tests[i].depth);
        auto test_duration = std::chrono::high_resolution_clock::now() - test_start;
        auto test_duration_ms = std::chrono::duration_cast<std::chrono::milliseconds>(test_duration).count();

        if (output_nodes != tests[i].expected_nodes) {
            std::cout << "  Result: " << RED << "Failed" << RESET << "\n";
            std::cout << "  Output Nodes: " << output_nodes << "\n";
        } else {
            pass++;
            std::cout << "  Result: " << GREEN << "Passed" << RESET << "\n";
        }

        total_nodes += output_nodes;
        std::cout << "  Test Time: " << test_duration_ms << " ms\n\n";
    }

    auto overall_duration = std::chrono::high_resolution_clock::now() - overall_start;
    auto overall_duration_ms = std::chrono::duration_cast<std::chrono::milliseconds>(overall_duration).count();
    double overall_nodes_per_sec = (total_nodes * 1000.0) / overall_duration_ms;

    std::cout << "=========================\n";
    std::cout << "Perft Unit Test Summary:\n";
    std::cout << "Passed: " << GREEN << pass << RESET << "/" << (sizeof(tests) / sizeof(Test)) << "\n";
    std::cout << "Total Nodes: " << total_nodes << "\n";
    std::cout << "Total Time: " << overall_duration_ms << " ms\n";
    std::cout << std::fixed << std::setprecision(2);
    std::cout << "Average Nodes per Second: " << overall_nodes_per_sec << "\n";
    std::cout << "=========================\n";
}

