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
        parse_fen(start_position);
        print_board(colour_to_move);
        search_position(10);
    }
    else
        uci_loop();

    return 0;
}