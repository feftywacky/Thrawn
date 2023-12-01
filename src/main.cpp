#include "bitboard.hpp"
#include "engine.hpp"
#include "bitboard_functions.hpp"
#include "constants.hpp"
#include "fen.hpp"
#include "move_helpers.hpp"
#include <cstdint>
#include <iostream>

using namespace std;

int main() {
    Engine engine = Engine();
    Bitboard& board = engine.board;

    parse_fen(board, "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1 ");
    print_board(board, board.colour_to_move);

    engine.generate_moves();
    vector<int> moves = engine.moves;

    cout<<"********************"<<endl;
    for(int move : moves)
    {
        board.copyBoard();
        engine.make_move(move, all_moves);
        cout<<"********************"<<endl;
        print_bitboard(board.occupancies[both]); 
        cout<<get_move_piece(move)<<endl; 
        cout<<"********************"<<endl; 
        cout<<""<<endl; 
        cout<<""<<endl;
        cout<<""<<endl;
        cout<<""<<endl;
        board.restoreBoard();
    }

    return 0;
}