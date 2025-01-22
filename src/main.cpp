#include "bitboard.h"
#include "bitboard_helpers.h"
#include "fen.h"
#include "move_helpers.h"
#include "perft.h"
#include "uci.h"
#include "search.h"
#include "threading.h"       // new
#include "transposition_table.h"
#include "misc.h"
#include "position.h"
#include <iostream>

using namespace std;

int main() {
    thrawn::Position pos;
    thrawn::Position pos2;
    init_all();

    bool dev_mode = false;
    
    if (dev_mode)
    {
        // parse_fen(pos2,start_position);
        // search_position_singlethreaded(pos2,12);

        parse_fen(pos,start_position);
        search_position_threaded(pos,14,4);

        
    }
    else
    {
        uci_loop(pos);
    }

    delete[] hashmap;
    return 0;
}
