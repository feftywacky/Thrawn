#include "bitboard.hpp"
#include "engine.hpp"
#include "bitboard_functions.hpp"
#include "constants.hpp"
#include <cstdint>


int main() {
    
    Engine gameState = Engine();
    Bitboard board = gameState.board;

    uint64_t test = 0ULL;

    test = set_bit(test, e4);

    // print_bitboard(test);
    // print_bitboard(test>>1);
    // print_bitboard(test<<1);

    for (int i=0;i<64;i++)
    {
        print_bitboard(board.bishop_attacks[i]);
    }


    return 0;
}