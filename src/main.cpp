#include "bitboard.hpp"
#include "engine.hpp"
#include "bitboard_functions.hpp"
#include "constants.hpp"
#include "fen.hpp"
#include "move_helpers.hpp"
#include <cstdint>
#include <iostream>

using namespace std;

int main() {
    Engine engine = Engine();
    Bitboard& board = engine.board;

    // parse_fen(board, start_position);
    // print_board(board, board.colour_to_move);

    int move = parse_move(d7, d8, k, Q, 0, 1, 1, 0);
    int move2 = parse_move(e4, e3, P, Q, 1, 0, 0, 1);

    vector<int> x;
    x.push_back(move);
    x.push_back(move2);
    
    print_move_list(x);

    return 0;
}