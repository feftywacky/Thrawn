#include "bitboard.hpp"
#include "engine.hpp"
#include "bitboard_functions.hpp"
#include "constants.hpp"
#include "fen.hpp"
#include <cstdint>
#include <iostream>

using namespace std;

int main() {
    // Engine gameState = Engine();
    // Bitboard board = gameState.board;

    Bitboard board;

    parse_fen(board, tricky_position);
    print_board(board, board.colour_to_move);
    print_bitboard(board.occupancies[2]);
    parse_fen(board, start_position);
    print_board(board, board.colour_to_move);
    print_bitboard(board.occupancies[2]);

    
    return 0;
}