#include "bitboard.hpp"
#include "engine.hpp"
#include "bitboard_functions.hpp"
#include "constants.hpp"
#include "fen.hpp"
#include <cstdint>
#include <iostream>

using namespace std;

int main() {
    // Engine gameState = Engine();
    // Bitboard board = gameState.board;

    Bitboard board;

    parse_fen(board, tricky_position);
    print_board(board, white);
    for(int i=0;i<64;i++)
    {
        if (board.is_square_under_attack(i, white))
            cout<<"square: "<<i<<" is attacked"<<endl;
    }   

    return 0;
}