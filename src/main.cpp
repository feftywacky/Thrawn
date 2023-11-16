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


    board.piece_bitboards[P] = set_bit(board.piece_bitboards[P], e2);
    
    // print white pawn bitboard
    print_bitboard(board.piece_bitboards[P]);
    
    // print piece
    std::cout << "piece: " << ascii_pieces[P] << std::endl;
    std::cout << "piece: " << ascii_pieces[char_pieces.at('K')] << std::endl;
    std::cout << "piece: " << unicode_pieces[P] << std::endl;
    std::cout << "piece: " << unicode_pieces[char_pieces.at('K')] << std::endl;


    
    return 0;
}