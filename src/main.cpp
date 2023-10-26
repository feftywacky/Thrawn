#include "bitboard.hpp"
#include "engine.hpp"
#include "bitboard_functions.hpp"
#include "constants.hpp"
#include <cstdint>

int main() {
    
    Engine gameState = Engine();
    Bitboard board = gameState.board;

    // uint64_t test = 0ULL;

    // test = set_bit(test, e4);

    

    // print_bitboard(board.pawn_attacks[white][10]);
    // print_bitboard(board.pawn_attacks[white][10]);
    // print_bitboard(board.pawn_attacks[white][60]);
    for (int square = 0; square < 64; square++)
        print_bitboard(board.pawn_attacks[white][square]);

    return 0;
}