#include "perft.h"
#include "move_generator.h"
#include "move_helpers.h"
#include "constants.h"
#include "bitboard_helpers.h"
#include "zobrist_hashing.h"
#include <chrono>

long leaf_nodes;

void perft_search (int depth)
{
    
    if (depth == 0)
    {
        leaf_nodes++;
        return;
    }  

    vector<int> moves = generate_moves();
    
    for (int move : moves)
    {
        copyBoard();   


        if (!make_move(move, all_moves))
            continue;

        perft_search(depth-1);
        
        restoreBoard();

        // uint64_t curr_hash = gen_hashkey(); // new hashkey after move made
        // if (curr_hash != zobristKey)
        // {
        //     cout<<"undo_move()"<<"\n";
        //     cout<<"move: ";
        //     print_move(move);
        //     cout<<"\n";
        //     print_board(colour_to_move);
        //     cout<<"correct hashkey: "<<std::hex<<curr_hash<<"\n";
        //     cin.get();

        // }

    }

}

void perft_test(int depth)
{

    vector<int> moves = generate_moves();
    auto start = std::chrono::high_resolution_clock::now();
    for (int move : moves)
    {
        
        copyBoard();
        

        if (!make_move(move, all_moves))
            continue;

        long cumulative_nodes = leaf_nodes;
        
        perft_search(depth-1);

        long old_nodes = leaf_nodes - cumulative_nodes;

        restoreBoard();

        // Print move
        std::cout << "     move: " << square_to_coordinates[get_move_source(move)]
                  << square_to_coordinates[get_move_target(move)]
                  << (get_promoted_piece(move) ? promoted_pieces.at(get_promoted_piece(move)) : ' ')
                  << "  nodes: " << old_nodes <<"\n";
    }

    // Print results
    auto duration = std::chrono::high_resolution_clock::now() - start;
    std::cout << "\n    Depth: " << depth << "\n";
    std::cout << "    Nodes: " << leaf_nodes << "\n";
    std::cout << "    Time: " << std::chrono::duration_cast<std::chrono::milliseconds>(duration).count()<< " ms" << "\n\n";
    
    leaf_nodes = 0;
}
