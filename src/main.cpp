#include "bitboard.hpp"
#include "engine.hpp"
#include "bitboard_functions.hpp"
#include "constants.hpp"
#include <cstdint>
#include <iostream>

using namespace std;

int main() {
    // Engine gameState = Engine();
    // Bitboard board = gameState.board;

    Bitboard board = Bitboard();


    uint64_t occupancy = 0ULL;
    
    // set blocker pieces on board
    occupancy = set_bit(occupancy, c5);
    occupancy = set_bit(occupancy, f2);
    occupancy = set_bit(occupancy, g7);
    occupancy = set_bit(occupancy, b2);
    occupancy = set_bit(occupancy, g5);
    occupancy = set_bit(occupancy, e2);
    occupancy = set_bit(occupancy, e7);
    
    // print occupancies
    print_bitboard(occupancy);
    
    // print rook attacks
    print_bitboard(board.get_rook_attacks(e5, occupancy));
    
    // print bishop attacks
    print_bitboard(board.get_bishop_attacks(d4, occupancy));


    
    return 0;
}