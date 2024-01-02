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
#include "nnue.h"
#include "misc.h"
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
        parse_fen("7b/2P1k3/NP4P1/P3p3/2K1B3/8/8/8 w - - 1 82");
        // parse_fen(position_2);
        print_board(colour_to_move);

    }
    else
        uci_loop();
    
    delete[] hashmap;

    return 0;
}