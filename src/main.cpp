#include "bitboard.hpp"
#include "engine.hpp"
#include "bitboard_functions.hpp"
#include "constants.hpp"
#include <cstdint>


int main() {
    
    Engine gameState = Engine();
    Bitboard board = gameState.board;


    uint64_t blocker = 0ULL;


    blocker = set_bit(blocker, d7);
    blocker = set_bit(blocker, d2);
    blocker = set_bit(blocker, d1);
    blocker = set_bit(blocker, b4);
    blocker = set_bit(blocker, g4);

    cout<<get_random_number()<<"\n";
    cout<<get_random_number()<<"\n";
    cout<<get_random_number()<<"\n";
    cout<<get_random_number()<<"\n";
    cout<<get_random_number()<<"\n";
    cout<<get_random_number()<<"\n";


    return 0;
}