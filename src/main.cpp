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
        parse_fen(pos,start_position);
        print_board(pos,pos.colour_to_move);
        search_position(pos,10);
    }
    else
        uci_loop(pos);
    
    delete[] hashmap;

    return 0;
}