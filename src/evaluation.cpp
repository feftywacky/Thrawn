#include "evaluation.h"
#include "constants.h"
#include "bitboard.h"
#include "bitboard_helpers.h"
#include "nnue.h"
#include "search.h"

// [game phase][piece]
const int material_score[2][12] = 
{
    // pawn, knight, bishop, rook, queen, king
    // white, black
    // middlegame score
    {82, 337, 365, 477, 1025, 0, -82, -337, -365, -477, -1025,  0},
    // endgame score
    {94, 281, 297, 512,  936, 0, -94, -281, -297, -512, -936, 0}
};

// [game phase][piece][sq]
const int position_score[2][6][64] = 
{
    // MIDDLEGAME
    {
        // pawn
        {
            0,   0,   0,   0,   0,   0,  0,   0,
            98, 134,  61,  95,  68, 126, 34, -11,
            -6,   7,  26,  31,  65,  56, 25, -20,
            -14,  13,   6,  21,  23,  12, 17, -23,
            -27,  -2,  -5,  12,  17,   6, 10, -25,
            -26,  -4,  -4, -10,   3,   3, 33, -12,
            -35,  -1, -20, -23, -15,  24, 38, -22,
            0,   0,   0,   0,   0,   0,  0,   0,
        },
        
        // knight
        {-167, -89, -34, -49,  61, -97, -15, -107,
        -73, -41,  72,  36,  23,  62,   7,  -17,
        -47,  60,  37,  65,  84, 129,  73,   44,
        -9,  17,  19,  53,  37,  69,  18,   22,
        -13,   4,  16,  13,  28,  19,  21,   -8,
        -23,  -9,  12,  10,  19,  17,  25,  -16,
        -29, -53, -12,  -3,  -1,  18, -14,  -19,
        -105, -21, -58, -33, -17, -28, -19,  -23},
        
        // bishop
        {-29,   4, -82, -37, -25, -42,   7,  -8,
        -26,  16, -18, -13,  30,  59,  18, -47,
        -16,  37,  43,  40,  35,  50,  37,  -2,
        -4,   5,  19,  50,  37,  37,   7,  -2,
        -6,  13,  13,  26,  34,  12,  10,   4,
        0,  15,  15,  15,  14,  27,  18,  10,
        4,  15,  16,   0,   7,  21,  33,   1,
        -33,  -3, -14, -21, -13, -12, -39, -21,},
        
        // rook
        { 32,  42,  32,  51,  63,   9,  31,  43,
            27,  32,  58,  62,  80,  67,  26,  44,
            -5,  19,  26,  36,  17,  45,  61,   -8, -20,
            -36, -26, -12, -1,  9, -7,   6, -23,
            -45, -25, -16, -17,  3,  0,  -5, -33,
            -44, -16, -20,  -9, -1, 11,  -6, -71,
            -19, -13,   1,  17, 16,  7, -37, -26},
        
        // queen
        {-28,   0,  29,  12,  59,  44,  43,  45,
            -24, -39,  -5,   1, -16,  57,  28,  54,
            -13, -17,   7,   8,  29,  56,  47,  57,
            -27, -27, -16, -16,  -1,  17,  -2,   1,
            -9, -26,  -9, -10,  -2,  -4,   3,  -3,
            -14,   2, -11,  -2,  -5,   2,  14,   5,
            -35,  -8,  11,   2,   8,  15,  -3,   1,
            -1, -18,  -9,  10, -15, -25, -31, -50},
        
        // king
        {-65,  23,  16, -15, -56, -34,   2,  13,
            29,  -1, -20,  -7,  -8,  -4, -38, -29,
            -9,  24,   2, -16, -20,   6,  22, -22,
            -17, -20, -12, -27, -30, -25, -14, -36,
            -49,  -1, -27, -39, -46, -44, -33, -51,
            -14, -14, -22, -46, -44, -30, -15, -27,
            1,   7,  -8, -64, -43, -16,   9,   8,
            -15,  36,  12, -54,   8, -28,  24,  14},
    },
    
    // ENDGAME
    {
        // pawn
        {0,   0,   0,   0,   0,   0,   0,   0,
            178, 173, 158, 134, 147, 132, 165, 187,
            94, 100,  85,  67,  56,  53,  82,  84,
            32,  24,  13,   5,  -2,   4,  17,  17,
            13,   9,  -3,  -7,  -7,  -8,   3,  -1,
            4,   7,  -6,   1,   0,  -5,  -1,  -8,
            13,   8,   8,  10,  13,   0,   2,  -7,
            0,   0,   0,   0,   0,   0,   0,   0},

        // knight
        {-58, -38, -13, -28, -31, -27, -63, -99,
            -25,  -8, -25,  -2,  -9, -25, -24, -52,
            -24, -20,  10,   9,  -1,  -9, -19, -41,
            -17,   3,  22,  22,  22,  11,   8, -18,
            -18,  -6,  16,  25,  16,  17,   4, -18,
            -23,  -3,  -1,  15,  10,  -3, -20, -22,
            -42, -20, -10,  -5,  -2, -20, -23, -44,
            -29, -51, -23, -15, -22, -18, -50, -64},

        // bishop
        {-14, -21, -11,  -8, -7,  -9, -17, -24,
            -8,  -4,   7, -12, -3, -13,  -4, -14,
            2,  -8,   0,  -1, -2,   6,   0,   4,
            -3,   9,  12,   9, 14,  10,   3,   2,
            -6,   3,  13,  19,  7,  10,  -3,  -9,
            -12,  -3,   8,  10, 13,   3,  -7, -15,
            -14, -18,  -7,  -1,  4,  -9, -15, -27,
            -23,  -9, -23,  -5, -9, -16,  -5, -17},

        // rook
        { 13, 10, 18, 15, 12,  12,   8,   5,
            11, 13, 13, 11, -3,   3,   8,   3,
            7,  7,  7,  5,  4,  -3,  -5,  -3,
            4,  3, 13,  1,  2,   1,  -1,   2,
            3,  5,  8,  4, -5,  -6,  -8, -11,
            -4,  0, -5, -1, -7, -12,  -8, -16,
            -6, -6,  0,  2, -9,  -9, -11,  -3,
            -9,  2,  3, -1, -5, -13,   4, -20},

        // queen
        {-9,  22,  22,  27,  27,  19,  10,  20,
            -17,  20,  32,  41,  58,  25,  30,   0,
            -20,   6,   9,  49,  47,  35,  19,   9,
            3,  22,  24,  45,  57,  40,  57,  36,
            -18,  28,  19,  47,  31,  34,  39,  23,
            -16, -27,  15,   6,   9,  17,  10,   5,
            -22, -23, -30, -16, -16, -23, -36, -32,
            -33, -28, -22, -43,  -5, -32, -20, -41},

        // king
        {-74, -35, -18, -18, -11,  15,   4, -17,
            -12,  17,  14,  17,  17,  38,  23,  11,
            10,  17,  23,  15,  20,  45,  44,  13,
            -8,  22,  24,  27,  26,  33,  26,   3,
            -18,  -4,  21,  24,  27,  23,   9, -11,
            -19,  -3,  11,  21,  23,  16,   7,  -9,
            -27, -11,   4,  13,  14,   4,  -5, -17,
            -53, -34, -21, -11, -28, -14, -24, -43}
    }
};

const int mirror_score[128] =
{
	a1, b1, c1, d1, e1, f1, g1, h1,
	a2, b2, c2, d2, e2, f2, g2, h2,
	a3, b3, c3, d3, e3, f3, g3, h3,
	a4, b4, c4, d4, e4, f4, g4, h4,
	a5, b5, c5, d5, e5, f5, g5, h5,
	a6, b6, c6, d6, e6, f6, g6, h6,
	a7, b7, c7, d7, e7, f7, g7, h7,
	a8, b8, c8, d8, e8, f8, g8, h8
};

// file and rank masks
uint64_t file_masks[64];
uint64_t rank_masks[64];
uint64_t isolated_masks[64];
uint64_t wPassedPawn_masks[64];
uint64_t bPassedPawn_masks[64];

// game phase score
const int opening_phase_score = 6192;
const int endgame_phase_score = 518;

// penalties, bonuses, and scores
const int double_pawn_penalty_middlegame = -5;
const int double_pawn_penalty_endgame = -10;
const int isolated_pawn_penalty_middlegame = -5;
const int isolated_pawn_penalty_endgame = -15;
const int passed_pawn_bonus[8] = {0, 10, 25, 45, 70, 100, 150, 200}; 
const int semi_open_file_score = 10;
const int open_file_score = 15;
const int bishop_mobility_unit = 4;
const int queen_mobility_unit = 9;
const int bishop_mobility_opening = 5;
const int bishop_mobility_endgame = 5;
const int queen_mobility_opening = 1;
const int queen_mobility_endgame = 2;
const int king_shield_bonus = 5;

const int get_rank_from_sq[64] =
{
    7, 7, 7, 7, 7, 7, 7, 7,
    6, 6, 6, 6, 6, 6, 6, 6,
    5, 5, 5, 5, 5, 5, 5, 5,
    4, 4, 4, 4, 4, 4, 4, 4,
    3, 3, 3, 3, 3, 3, 3, 3,
    2, 2, 2, 2, 2, 2, 2, 2,
    1, 1, 1, 1, 1, 1, 1, 1,
	0, 0, 0, 0, 0, 0, 0, 0
};

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

uint64_t set_eval_masks(int rankNum, int fileNum)
{
    uint64_t mask = 0ULL;

    for(int r=0;r<8;r++)
    {
        for(int f=0;f<8;f++)
        {
            int sq = r*8+f;
            
            if (rankNum == -1 && f==fileNum)
                set_bit(mask, sq);
            else if (fileNum == -1 && r==rankNum)
                set_bit(mask, sq);
        }
    }   
    return mask;
}

void init_eval_masks()
{
    // init file masks
    for(int r=0;r<8;r++)
    {
        for(int f=0;f<8;f++)
        {
            int sq = r*8+f;
            file_masks[sq] |= set_eval_masks(-1,f);
        }
    }

    // init rank masks
    for(int r=0;r<8;r++)
    {
        for(int f=0;f<8;f++)
        {
            int sq = r*8+f;
            rank_masks[sq] |= set_eval_masks(r,-1);
        }
    }

    // init isolated masks
    for(int r=0;r<8;r++)
    {
        for(int f=0;f<8;f++)
        {
            int sq = r*8+f;
            isolated_masks[sq] |= set_eval_masks(-1,f+1);
            isolated_masks[sq] |= set_eval_masks(-1,f-1);
        }
    }
    
    // init white pass pawn masks
    for(int r=0;r<8;r++)
    {
        for(int f=0;f<8;f++)
        {
            int sq = r*8+f;
            wPassedPawn_masks[sq] |= set_eval_masks(-1, f);
            wPassedPawn_masks[sq] |= set_eval_masks(-1, f+1);
            wPassedPawn_masks[sq] |= set_eval_masks(-1, f-1);

            for(int i=0;i<(8-r);i++)
                wPassedPawn_masks[sq] &= ~rank_masks[(7 - i) * 8 + f];
        }
    }

    // init black pass pawn masks
    for(int r=0;r<8;r++)
    {
        for(int f=0;f<8;f++)
        {
            int sq = r*8+f;
            bPassedPawn_masks[sq] |= set_eval_masks(-1, f);
            bPassedPawn_masks[sq] |= set_eval_masks(-1, f+1);
            bPassedPawn_masks[sq] |= set_eval_masks(-1, f-1);

            for(int i=0;i<r+1;i++)
                bPassedPawn_masks[sq] &= ~rank_masks[i*8+f];
        }
    }
}

int get_gamePhase_score()
{
    // gamePhase scoe determined by adding up all the material scores for each piece

    int gamePhase_score = 0;

    for (int piece = N;piece<=Q;piece++)
        gamePhase_score += count_bits(piece_bitboards[piece]) * material_score[opening][piece];
    for (int piece = n;piece<=q;piece++)
        gamePhase_score += count_bits(piece_bitboards[piece]) * -material_score[opening][piece];
    return gamePhase_score;
}

//position evaluation
int evaluate()
{   
    uint64_t bitboard;
    int square;

    // NNUE
    // piece codes converted from Thrawn to NNUE
    int pieces[33];
    // square indices converted from Thrawn to NNUE
    int squares[33];
    int index = 2;
    
    for (int piece = P; piece <= k; piece++)
    {
        bitboard = piece_bitboards[piece];
        
        while (bitboard)
        {            
            square = get_lsb_index(bitboard);

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

    // (100-fifty_move) / 100 
    // taken from Cfish for fifty move scaling
    return nnue_evaluate(colour_to_move, pieces, squares) * (100-fifty_move) / 100;
}