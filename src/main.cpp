#include "bitboard.hpp"
#include "engine.hpp"
#include "bitboard_functions.hpp"
#include "constants.hpp"
#include "fen.hpp"
#include <cstdint>
#include <iostream>

using namespace std;

int main() {
    Engine engine = Engine();
    Bitboard& board = engine.board;

    // Bitboard board;

    parse_fen(board, start_position);
    print_board(board, board.colour_to_move);

    return 0;
}