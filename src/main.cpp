#include "bitboard.hpp"
#include "engine.hpp"
#include "bitboard_functions.hpp"
#include "constants.hpp"
#include <cstdint>


int main() {
    
    Engine gameState = Engine();
    Bitboard board = gameState.board;

    uint64_t test = 0ULL;

    test = set_bit(test, h5);

    for (int i=0;i<64;i++)
    {
        print_bitboard(board.knight_attacks[i]);
    }


    return 0;
}