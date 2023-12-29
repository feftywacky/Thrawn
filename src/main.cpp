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
#include "nnue_wrapper.h"
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
        parse_fen(start_position);
        print_board(colour_to_move);

        
        init_nnue("nn-62ef826d1a6d.nnue");
        cout<<evaluate_fen_nnue(start_position)<<endl;

        int pieces[33];
        int squares[33];

        parse_nnue_args(pieces, squares);
        cout<<evaluate_nnue(colour_to_move, pieces, squares)<<endl;

        int nnue_score = evaluate();
        cout<<nnue_score<<endl;
    }
    else
        uci_loop();
    
    delete[] hashmap;

    return 0;
}