#include "bitboard.hpp"
#include "engine.hpp"
#include "bitboard_functions.hpp"
#include "constants.hpp"
#include <cstdint>


int main() {
    
    Engine gameState = Engine();
    Bitboard board = gameState.board;


    uint64_t blocker = 0ULL;


    // blocker = set_bit(blocker, d7);
    // blocker = set_bit(blocker, d2);
    // blocker = set_bit(blocker, d1);
    // blocker = set_bit(blocker, b4);
    // blocker = set_bit(blocker, g4);

    
    for (int i=0;i<64;i++)
        printf(" 0x%llxULL\n", rook_magic_nums[i]);
    cout<<"__________________________________________________________________"<<endl;
    for (int i=0;i<64;i++)
        printf(" 0x%llxULL\n", bishop_magic_nums[i]);




    return 0;
}