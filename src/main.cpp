#include "bitboard.h"
#include "bitboard_helpers.h"
#include "fen.h"
#include "move_helpers.h"
#include "perft.h"
#include "uci.h"
#include "search.h"
#include "threading.h"       
#include "globals.h"
#include "transposition_table.h"
#include "misc.h"
#include "position.h"
#include <iostream>

using namespace std;

int main() {
    init_all();
    bool dev_mode = false;
    
    if (dev_mode)
    {
        cout<<"dev mode"<<endl;
        //perft_run_unit_tests();
        parse_fen(pos,start_position);
        //search_pos_single(pos,12);
        search_position_threaded(pos,12,4);
    }
    else
    {
        uci_loop(pos);
    }

    delete pos;

    return 0;
}
