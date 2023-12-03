#include "bitboard.h"
#include "engine.h"
#include "bitboard_helpers.h"
#include "constants.h"
#include "fen.h"
#include "move_helpers.h"
#include "perft.h"
#include <cstdint>
#include <iostream>
#include <chrono>
#include <random>

using namespace std;

int main() {
    Engine engine = Engine();
    Bitboard& board = engine.board;

    parse_fen(board, start_position);
    print_board(board, board.colour_to_move);
    
    auto start = std::chrono::high_resolution_clock::now();
    perft_search(2, engine, board);
    cout<<"leaf nodes: "<<leaf_nodes<<"\n";
    auto stop = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start);
    std::cout << "Time taken by function: " << duration.count() << " milliseconds" << std::endl;

    // for(int i=0;i<10;i++)
    // {
    //     vector<int> moves = engine.generate_moves();
    //     std::random_device rd;
    //     std::mt19937 gen(rd());

    //     // Choose a random index
    //     std::uniform_int_distribution<> dis(0, moves.size() - 1);
    //     int randomIndex = dis(gen);

    //     engine.make_move(moves[randomIndex], all_moves);
    //     print_board(board, board.colour_to_move);
    // }


    return 0;
}