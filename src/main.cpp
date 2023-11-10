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

    print_bitboard(blocker);
    cout<<"index: "<<get_lsb_index(blocker)<<"   coord: "<<square_to_coordinates[get_lsb_index(blocker)]<<"\n";


    uint64_t test = 0ULL;
    test = set_bit(test, get_lsb_index(blocker));
    print_bitboard(test);


    return 0;
}