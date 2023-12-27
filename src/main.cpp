#include "bitboard.h"
#include "move_generator.h"
#include "bitboard_helpers.h"
#include "constants.h"
#include "fen.h"
#include "move_helpers.h"
#include "perft.h"
#include "uci.h"
#include "evaluation.h"
#include "search.h"
#include "zobrist_hashing.h"
#include "transposition_table.h"
#include <cstdint>
#include <iostream>
#include <chrono>

using namespace std;

int main() {

    init_all();

    bool dev_mode = true;

    if (dev_mode)
    {
        cout<<"developer mode"<<endl;
        parse_fen("3r4/1p1r2pk/3N1p1p/P1B4P/P3p3/4P1P1/1pbR1PK1/4R3 b - - 1 49");
        print_board(colour_to_move);

        search_position(11);
        // cout<<get_gamePhase_score()<<endl;
        // cout<<evaluate()<<endl;
        // search_position(10);
    }
    else
        uci_loop();

    return 0;
}