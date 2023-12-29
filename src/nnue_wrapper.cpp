/*
WRAPPER FILE FOR NNUE 
*/
#include "nnue.h"
#include "nnue_wrapper.h"
#include "bitboard.h"
#include <cstdint>

// refer to for nnue.h for NNUE piece codes and constants.h for Thrawn piece codes
// parsing piece codes from Thrawn to NNUE
int nnue_pieces[12] = { 6, 5, 4, 3, 2, 1, 12, 11, 10, 9, 8, 7 };

// Thrawn uses a8=0 - h8=7 and a1=56 - h1=63
// refer to nnue.h for nnue square indices
// parsing square indices from Thrawn to NNUE
int nnue_squares[64] = {
    a1, b1, c1, d1, e1, f1, g1, h1,
	a2, b2, c2, d2, e2, f2, g2, h2,
	a3, b3, c3, d3, e3, f3, g3, h3,
	a4, b4, c4, d4, e4, f4, g4, h4,
	a5, b5, c5, d5, e5, f5, g5, h5,
	a6, b6, c6, d6, e6, f6, g6, h6,
	a7, b7, c7, d7, e7, f7, g7, h7,
	a8, b8, c8, d8, e8, f8, g8, h8
};

void init_nnue(const char* nnue_filename)
{   
    return nnue_init(nnue_filename);
}

int evaluate_nnue(int player,int* pieces, int* squares)
{
    return nnue_evaluate(player, pieces, squares);
}

int evaluate_fen_nnue(const char* fen)
{
    return nnue_evaluate_fen(fen);
}

void parse_nnue_args(int *pieces, int *squares)
{
    uint64_t bitboard;
    int square;
    int index = 2;

    for (int piece = P; piece <= k; piece++)
    {
        bitboard = piece_bitboards[piece];
        
        while (bitboard)
        {            
            square = get_lsb_index(bitboard);
            
            // print debug
            // std::cout << "piece: " << piece << "  piece: " << unicode_pieces[piece]
            //   << "  square index: " << square << "  square: " << square_to_coordinates[square] << std::endl;
            
            // parsing white king piece code to stockfish piece code
            if (piece == K)
            {
                pieces[0] = nnue_pieces[piece];
                squares[0] = nnue_squares[square];
            }
            
            else if (piece == k)
            {
                pieces[1] = nnue_pieces[piece];
                squares[1] = nnue_squares[square];
            }
            
            else
            {
                pieces[index] = nnue_pieces[piece];
                squares[index] = nnue_squares[square];
                index++;    
            }
            
            pop_bit(bitboard, square);
        }
    }
    
    // end arrays with terminating zero
    pieces[index] = 0;
    squares[index] = 0;
}