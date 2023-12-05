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

    // position 1
    parse_fen(board, start_position);
    print_board(board, board.colour_to_move);
    perft_test(6, engine, board);

    // position 2
    parse_fen(board, "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq -");
    print_board(board, board.colour_to_move);
    perft_test(5, engine, board);

    // position 3
    parse_fen(board, "8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - ");
    print_board(board, board.colour_to_move);
    perft_test(6, engine, board);

    // position 4
    parse_fen(board, "r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1");
    print_board(board, board.colour_to_move);
    perft_test(6, engine, board);

    // position 5
    parse_fen(board, "rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ - 1 8");
    print_board(board, board.colour_to_move);
    perft_test(5, engine, board);

    // position 6
    parse_fen(board, "r4rk1/1pp1qppp/p1np1n2/2b1p1B1/2B1P1b1/P1NP1N2/1PP1QPPP/R4RK1 w - - 0 10");
    print_board(board, board.colour_to_move);
    perft_test(5, engine, board);

    return 0;
}