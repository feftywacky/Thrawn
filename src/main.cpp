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

    Bitboard board;


    print_board(board, board.colour_to_move);
    for(int i=0;i<3;i++)
    {
        cout<<i<<endl;
        print_bitboard(board.occupancies[i]);
    }


    
    return 0;
}