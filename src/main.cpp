#include "bitboard.hpp"
#include "engine.hpp"
#include "bitboard_functions.hpp"
#include "constants.hpp"
#include <cstdint>


int main() {
    
    Engine gameState = Engine();
    Bitboard board = gameState.board;

    uint64_t test = 0ULL;
    uint64_t blocker = 0ULL;

    blocker = set_bit(blocker, c4);
    blocker = set_bit(blocker, d7);
    blocker = set_bit(blocker, h4);
    blocker = set_bit(blocker, d2);

    test = set_bit(test, e4);

    print_bitboard(board.rook_attack_runtime_gen(d4,0ULL));
    print_bitboard(blocker);
    print_bitboard(board.rook_attack_runtime_gen(d4,blocker));

    // print_bitboard(test);
    // print_bitboard(test>>1);
    // print_bitboard(test<<1);
    // for (int i=0;i<64;i++)
    //     print_bitboard(board.get_rook_attack_from_sq(i));

    return 0;
}