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

    // parse_fen(board, tricky_position);
    // print_board(board, board.colour_to_move);
    // print_bitboard(board.occupancies[2]);
    // parse_fen(board, start_position);
    // print_board(board, board.colour_to_move);
    // print_bitboard(board.occupancies[2]);

    uint64_t occupancy = 0ULL;
    
    // set occupancy
    set_bit(occupancy, b6);
    set_bit(occupancy, d6);
    set_bit(occupancy, f6);
    set_bit(occupancy, b4);
    set_bit(occupancy, g4);
    print_bitboard(occupancy);
    print_bitboard(board.get_queen_attacks(d4, occupancy));

    return 0;
}