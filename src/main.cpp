#include "bitboard.hpp"
#include "engine.hpp"
#include "bitboard_functions.hpp"
#include "constants.hpp"
#include "fen.hpp"
#include <cstdint>
#include <iostream>

using namespace std;

int main() {
    Engine engine = Engine();
    Bitboard& board = engine.board;

    // parse_fen(board, start_position);
    // print_board(board, board.colour_to_move);

    int move = engine.parse_move(d7, d8, null_sq, 0, 0, 0, 0, 0);
    
    int source_square = engine.get_move_source(move);
    int target_square = engine.get_move_target(move);
    int piece = engine.get_move_piece(move);
    int promoted_piece = engine.get_promoted_piece(move);
    
    std::cout << "Source square: " << square_to_coordinates[source_square] << std::endl;
    std::cout << "Target square: " << square_to_coordinates[target_square] << std::endl;
    std::cout << "Piece: " << ascii_pieces[piece] << std::endl;
    std::cout << "Promoted piece: " << ascii_pieces[promoted_piece] << std::endl;
    std::cout << "Capture flag: " << (engine.get_is_cpature_move(move) ? 1 : 0) << std::endl;
    std::cout << "Double pawn push flag: " << (engine.get_is_double_pawn_move(move) ? 1 : 0) << std::endl;
    std::cout << "En passant flag: " << (engine.get_is_move_enpassant(move) ? 1 : 0) << std::endl;
    std::cout << "Castling flag: " << (engine.get_is_move_castling(move) ? 1 : 0) << std::endl;

    return 0;
}