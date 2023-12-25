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
#include "zobrist_hashing.h"
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
        parse_fen("r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1");
        print_board(colour_to_move);

        // perft_search(1);
        perft_test(5);  

    }
    else
        uci_loop();

    return 0;
}