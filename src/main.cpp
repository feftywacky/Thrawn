#include "bitboard.h"
#include "bitboard_helpers.h"
#include "fen.h"
#include "move_helpers.h"
#include "perft.h"
#include "uci.h"
#include "search.h"
#include "transposition_table.h"
#include "misc.h"
#include <iostream>

using namespace std;

int main() {
    
    init_all();

    bool dev_mode = true;

    if (dev_mode)
    {
        cout<<"developer mode"<<endl;
        parse_fen("r3kb1r/3n1pp1/p6p/2pPp2q/Pp2N3/3B2PP/1PQ2P2/R3K2R w KQkq -");
        // parse_fen(position_2);
        print_board(colour_to_move);

        //search_position(12);
        search_position_threaded(12,2);

    }
    else
        uci_loop();
    
    delete[] hashmap;

    return 0;
}