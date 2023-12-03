#include "perft.h"
#include "engine.h"
#include "move_helpers.h"
#include "constants.h"
#include "bitboard_helpers.h"
#include <chrono>

long leaf_nodes = 0;

void perft_search (int depth, Engine &engine, Bitboard& board)
{
    if (depth == 0)
    {
        leaf_nodes++;
        return;
    }  

    vector<int> moves = engine.generate_moves();

    for (int move : moves)
    {
        board.copyBoard();   
        // cout<<"driver copy board"<<endl;
        // print_board(board, board.colour_to_move); 

        if (!engine.make_move(move, all_moves))
            continue;

        
        perft_search(depth-1, engine, board);

        board.restoreBoard();
        // cout<<"driver restore board"<<endl;
        // print_board(board, board.colour_to_move); 
    }

}

void perft_test(int depth, Engine& engine, Bitboard& board)
{

    vector<int> moves = engine.generate_moves();
    auto start = std::chrono::high_resolution_clock::now();
    for (int move : moves)
    {
        board.copyBoard();
        cout<<"test copy board"<<endl;
        print_board(board, board.colour_to_move); 

        if (!engine.make_move(move, all_moves))
            continue;

        long cumulative_nodes = leaf_nodes;

        perft_search(depth-1, engine, board);

        long old_nodes = leaf_nodes - cumulative_nodes;


        // cout<<"____________________"<<endl;
        // print_board(board, board.colour_to_move); 
        board.restoreBoard();
        cout<<"test restore board"<<endl;
        print_board(board, board.colour_to_move);

        // Print move
        std::cout << "     move: " << square_to_coordinates[get_move_source(move)]
                  << square_to_coordinates[get_move_target(move)]
                  << (get_promoted_piece(move) ? promoted_pieces.at(get_promoted_piece(move)) : ' ')
                  << "  nodes: " << old_nodes << std::endl;
    }

    // Print results
    auto duration = std::chrono::high_resolution_clock::now() - start;
    std::cout << "\n    Depth: " << depth << "\n";
    std::cout << "    Nodes: " << leaf_nodes << "\n";
    std::cout << "     Time: " << std::chrono::duration_cast<std::chrono::milliseconds>(duration).count() << "\n\n";
     
}

