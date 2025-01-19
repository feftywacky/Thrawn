#include "perft.h"
#include "move_generator.h"
#include "move_helpers.h"
#include "constants.h"
#include "bitboard_helpers.h"
#include "zobrist_hashing.h"
#include "position.h"
#include "fen.h"
#include <chrono>

long leaf_nodes;

void perft_search (thrawn::Position& pos, int depth)
{
    
    if (depth == 0)
    {
        leaf_nodes++;
        return;
    }  

    vector<int> moves = generate_moves(pos);
    
    for (int move : moves)
    {
        pos.copyBoard(depth);   


        if (!make_move(pos, move, all_moves,depth))
            continue;

        perft_search(pos, depth-1);
        
        pos.restoreBoard(depth);

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

int perft_test(thrawn::Position& pos, int depth)
{

    vector<int> moves = generate_moves(pos);
    auto start = std::chrono::high_resolution_clock::now();
    for (int move : moves)
    {
        
        pos.copyBoard(depth);
        

        if (!make_move(pos, move, all_moves,depth))
            continue;

        long cumulative_nodes = leaf_nodes;
        
        perft_search(pos, depth-1);

        long old_nodes = leaf_nodes - cumulative_nodes;

        pos.restoreBoard(depth);

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
    
    int result = leaf_nodes;
    leaf_nodes = 0;

    return result;
}

void perft_run_unit_tests()
{
    thrawn::Position p;
    int output_nodes = 0;

    struct Test {
        const char* fen;
        int depth;
        int expected_nodes;
    };

    Test tests[] = {
        {start_position, 6, 119060324},
        {position_2, 5, 193690690},
        {position_3, 7, 178633661},
        {position_4, 5, 15833292},
        {position_5, 5, 89941194},
        {position_6, 5, 164075551},
    };

    int pass = 0;

    for (int i = 0; i < sizeof(tests) / sizeof(Test); i++) {
        parse_fen(p, tests[i].fen);
        output_nodes = perft_test(p, tests[i].depth);

        if (output_nodes != tests[i].expected_nodes) {
            std::cout << "Test " << i + 1 << " Failed\n";
            std::cout << "  FEN: " << tests[i].fen << "\n";
            std::cout << "  Output nodes: " << output_nodes << "\n";
            std::cout << "  Expected nodes: " << tests[i].expected_nodes << "\n";
        } else {
            pass++;
            std::cout << "Test " << i + 1 << " Passed. Nodes: " << output_nodes << "\n";
        }
        std::cout<<std::endl;
    }

    if(pass==6)
        std::cout << "All test cases passed! " << std::endl;
        
    std::cout<<std::endl;
}
