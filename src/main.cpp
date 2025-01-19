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

    bool dev_mode = true;
    
    if (dev_mode)
    {
        cout<<endl;
        cout<<"developer mode"<<endl;
        cout<<endl;
        perft_run_unit_tests();
    }
    else
        uci_loop(pos);
    
    delete[] hashmap;

    return 0;
}