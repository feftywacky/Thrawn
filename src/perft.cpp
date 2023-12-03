#include "perft.h"
#include "engine.h"

long leaf_nodes = 0;

void perft_search (int depth, Engine &engine, Bitboard &board)
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

        // skip non-legal moves
        if (!engine.make_move(move, all_moves))
            continue;
        
        perft_search(depth-1, engine, board);

        board.restoreBoard();
    }

}