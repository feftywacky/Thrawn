#include "bitboard.hpp"
#include "engine.hpp"
#include "bitboard_functions.hpp"
#include "constants.hpp"
#include <cstdint>


int main() {
    std::cout<<"1"<<std::endl;
    Engine gameState = Engine();
    std::cout<<"2"<<std::endl;
    Bitboard board = gameState.board;




    uint64_t test = 0ULL;
    std::cout<<"3"<<std::endl;
    print_bitboard(test);
    print_bitboard(board.get_bishop_attacks(d4, test));


    // blocker = set_bit(blocker, d7);
    // blocker = set_bit(blocker, d2);
    // blocker = set_bit(blocker, d1);
    // blocker = set_bit(blocker, b4);
    // blocker = set_bit(blocker, g4);


    
    return 0;
}