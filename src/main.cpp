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

    parse_fen(board, tricky_position);
    print_board(board, board.colour_to_move);

    perft_test(2, engine, board);


    return 0;
}