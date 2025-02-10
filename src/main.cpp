#include "bitboard.h"
#include "bitboard_helpers.h"
#include "fen.h"
#include "move_helpers.h"
#include "perft.h"
#include "uci.h"
#include "search.h"
#include "threading.h"       
#include "globals.h"
#include "evaluation.h"
#include "transposition_table.h"
#include "misc.h"
#include "position.h"
#include <iostream>

using namespace std;

int main() {
    init_all();
    bool dev = false;
    
    if (dev)
    {
        cout<<"dev mode"<<endl;
    }
    else
    {
        uci_loop(pos);
    }

    delete pos;
    delete tt;
    return 0;
}
