#include "bitboard.h"
#include "engine.h"
#include "bitboard_helpers.h"
#include "constants.h"
#include "fen.h"
#include "move_helpers.h"
#include "perft.h"
#include "uci.h"
#include "evaluation.h"
#include "search.h"
#include <cstdint>
#include <iostream>
#include <chrono>

using namespace std;

int main() {

    init_all();

    bool dev_mode = false;

    if (dev_mode)
    {
        cout<<"developer mode"<<endl;
        parse_fen("3rr1k1/pb2nppp/1ppq1n2/3pN3/3P1P2/P2B4/1PQ1NPPP/2R2RK1 w - - 0 16");
        print_board(colour_to_move);

        auto start_time = std::chrono::high_resolution_clock::now();
        search_position(8);
        auto end_time = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time);

        // Convert microseconds to seconds
        double seconds = duration.count() / 1e6;

        std::cout << "search_position execution time: " << seconds << " seconds" << std::endl;

    }
    else
        uci_loop();

    return 0;
}