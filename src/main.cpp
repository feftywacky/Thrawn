#include "bitboard.h"
#include "engine.h"
#include "bitboard_helpers.h"
#include "constants.h"
#include "fen.h"
#include "move_helpers.h"
#include "perft.h"
#include "uci.h"
#include "evaluation.h"
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
        cout<<"score: "<<evaluate()<<endl;
    }
    else
        uci_loop();

    return 0;
}