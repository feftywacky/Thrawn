#include "bitboard.h"
#include "engine.h"
#include "bitboard_functions.h"
#include "constants.h"
#include "fen.h"
#include "move_helpers.h"
#include <cstdint>
#include <iostream>

using namespace std;

int main() {
    Engine engine = Engine();
    Bitboard& board = engine.board;

    parse_fen(board, "r3k2r/8/8/8/8/8/8/R3K2R w KQkq - 0 1");
    print_board(board, board.colour_to_move);

    engine.generate_moves();
    vector<int> moves = engine.moves;

    cout<<"***********************************"<<endl;
    for(int move : moves)
    {
        board.copyBoard();
        engine.make_move(move, all_moves);

        print_board(board, board.colour_to_move);

        board.restoreBoard();
    }

    return 0;
}