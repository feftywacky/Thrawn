#include "bitboard.h"
#include "engine.h"
#include "bitboard_helpers.h"
#include "constants.h"
#include "fen.h"
#include "move_helpers.h"
#include "perft.h"
#include "uci.h"
#include "evaluation.h"
#include "search.h"
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
        parse_fen("5Rk1/p1P5/8/2P2p1p/4p3/4PpPP/5P1K/r7 b - - 0 1");
        print_board(colour_to_move);
        vector<int> moves = generate_moves();
        for(int move : moves)
        {
            if (get_move_source(move)==g8 && get_move_target(move)==f8)
                make_move(move, all_moves);
        }

        print_board(colour_to_move);

        search_position(4);

        moves = generate_moves();
        for(int move : moves)
        {
            if (get_move_source(move)==c7 && get_move_target(move)==c8 && get_promoted_piece(move)==Q)
                make_move(move, all_moves);
        }
        print_board(colour_to_move);

        // print_bitboard(occupancies[white]);
        search_position(4);

        // moves.clear();
        moves = generate_moves();
        for(int move : moves)
        {
            if (get_move_source(move)==f8 && get_move_target(move)==g7)
            {
                make_move(move, all_moves);
            }
        }
        print_board(colour_to_move);

        // print_bitboard(piece_bitboards[P]);
        // print_bitboard(piece_bitboards[Q]);

        // moves.clear();
        moves = generate_moves();
        print_move_list(moves);



    }
    else
        uci_loop();

    return 0;
}