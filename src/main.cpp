#include "bitboard.h"
#include "bitboard_helpers.h"
#include "fen.h"
#include "move_helpers.h"
#include "perft.h"
#include "uci.h"
#include "search.h"
#include "transposition_table.h"
#include "misc.h"
#include "position.h"
#include <iostream>

using namespace std;

int main() {
    thrawn::Position pos;
    init_all();

    bool dev_mode = false;

    if (dev_mode)
    {
        cout<<"developer mode"<<endl;

        thrawn::Position pos1;

        parse_fen(pos1, "r3kb1r/3n1pp1/p6p/2pPp2q/Pp2N3/3B2PP/1PQ2P2/R3K2R w KQkq -");
        // parse_fen(position_2);
        print_board(pos1, pos1.colour_to_move);

        //search_position(12);
//        search_position_threaded(12,2);

    }
    else
        uci_loop(pos);
    
    delete[] hashmap;

    return 0;
}