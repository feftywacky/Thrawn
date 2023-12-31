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
        parse_fen("k2r4/p2q1p1p/5Pp1/5b2/8/5Q1P/PPnN1PP1/2RK3R b - - 18 31");
        // parse_fen(position_2);
        print_board(colour_to_move);
        // cout<<"nnue: "<<evaluate_fen_nnue("k2r4/p2q1p1p/5Pp1/5b2/8/5Q1P/PPnN1PP1/2RK3R b - - 18 31");
        
        // int best_move;
        // parse_fen(position_2);
        // print_board(colour_to_move);
        // vector<int> moves = generate_moves();
        // print_move_scores(moves);
        // moves[0] = best_move;
        // sort_moves(moves,best_move);
        // cout<<"************************************"<<"\n";
        // print_move_scores(moves);

        search_position(8);
    }
    else
        uci_loop();
    
    delete[] hashmap;

    return 0;
}