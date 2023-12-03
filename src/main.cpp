#include "bitboard.h"
#include "engine.h"
#include "bitboard_helpers.h"
#include "constants.h"
#include "fen.h"
#include "move_helpers.h"
#include "perft.h"
#include <cstdint>
#include <iostream>
#include <chrono>
#include <random>

using namespace std;

int main() {
    Engine engine = Engine();
    Bitboard& board = engine.board;

    parse_fen(board, start_position);
    print_board(board, board.colour_to_move);
    
    // vector<int> moves = engine.generate_moves();
    // engine.make_move(parse_move(b1,c3,N,0,0,0,0,0), all_moves);

    // print_board(board, board.colour_to_move);

    // moves = engine.generate_moves();
    // print_move_list(moves);


    perft_test(2, engine, board);

    return 0;
}