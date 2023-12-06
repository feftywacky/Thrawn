#include "evaluation.h"
#include "constants.h"
#include "bitboard.h"

const std::array<int, 12> material_score = 
{
    82,      // white pawn
    337,      // white knight
    365,      // white bishop
    477,      // white rook64
    1025,      // white queen
    10000,      // white king
    -82,      // black pawn
    -337,      // black knight
    -365,      // black bishop 
    -477,      // black rook 
    -1025,      // black queen 
    -10000,      // black king
};

const std::array<int, 64> wKnight_mg = {
    -167, -89, -34, -49,  61, -97, -15, -107,
    -73, -41,  72,  36,  23,  62,   7,  -17,
    -47,  60,  37,  65,  84, 129,  73,   44,
    -9,  17,  19,  53,  37,  69,  18,   22,
    -13,   4,  16,  13,  28,  19,  21,   -8,
    -23,  -9,  12,  10,  19,  17,  25,  -16,
    -29, -53, -12,  -3,  -1,  18, -14,  -19,
    -105, -21, -58, -33, -17, -28, -19,  -23
};

const std::array<int, 64> wBishop_mg = {
    -29,   4, -82, -37, -25, -42,   7,  -8,
    -26,  16, -18, -13,  30,  59,  18, -47,
    -16,  37,  43,  40,  35,  50,  37,  -2,
     -4,   5,  19,  50,  37,  37,   7,  -2,
     -6,  13,  13,  26,  34,  12,  10,   4,
      0,  15,  15,  15,  14,  27,  18,  10,
      4,  15,  16,   0,   7,  21,  33,   1,
    -33,  -3, -14, -21, -13, -12, -39, -21
};

const std::array<int, 64> wRook_mg = {
    32,  42,  32,  51, 63,  9,  31,  43,
    27,  32,  58,  62, 80, 67,  26,  44,
    -5,  19,  26,  36, 17, 45,  61,  16,
    -24, -11,   7,  26, 24, 35,  -8, -20,
    -36, -26, -12,  -1,  9, -7,   6, -23,
    -45, -25, -16, -17,  3,  0,  -5, -33,
    -44, -16, -20,  -9, -1, 11,  -6, -71,
    -19, -13,   1,  17, 16,  7, -37, -26
};

const std::array<int, 64> wQueen_mg = {
    -28,   0,  29,  12,  59,  44,  43,  45,
    -24, -39,  -5,   1, -16,  57,  28,  54,
    -13, -17,   7,   8,  29,  56,  47,  57,
    -27, -27, -16, -16,  -1,  17,  -2,   1,
     -9, -26,  -9, -10,  -2,  -4,   3,  -3,
    -14,   2, -11,  -2,  -5,   2,  14,   5,
    -35,  -8,  11,   2,   8,  15,  -3,   1,
     -1, -18,  -9,  10, -15, -25, -31, -50
};

const std::array<int, 64> wKing_mg = {
    -65,  23,  16, -15, -56, -34,   2,  13,
     29,  -1, -20,  -7,  -8,  -4, -38, -29,
     -9,  24,   2, -16, -20,   6,  22, -22,
    -17, -20, -12, -27, -30, -25, -14, -36,
    -49,  -1, -27, -39, -46, -44, -33, -51,
    -14, -14, -22, -46, -44, -30, -15, -27,
      1,   7,  -8, -64, -43, -16,   9,   8,
    -15,  36,  12, -54,   8, -28,  24,  14
};

const std::array<int, 64> wPawn_mg = {
     0,   0,   0,   0,   0,   0,  0,   0,
    98, 134,  61,  95,  68, 126, 34, -11,
    -6,   7,  26,  31,  65,  56, 25, -20,
    -14,  13,   6,  21,  23,  12, 17, -23,
    -27,  -2,  -5,  12,  17,   6, 10, -25,
    -26,  -4,  -4, -10,   3,   3, 33, -12,
    -35,  -1, -20, -23, -15,  24, 38, -22,
     0,   0,   0,   0,   0,   0,  0,   0
};

const std::array<int, 128> mirror_score =
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


int evaluate()
{
    int score = 0;
    int piece;
    int square;
    uint64_t curr_piece_bb;

    for (int i=P;i<=k;i++)
    {
        curr_piece_bb = piece_bitboards[i];

        while(curr_piece_bb)
        {
            piece = i;
            square = get_lsb_index(curr_piece_bb);

            // material scores
            score += material_score[piece];

            // positional scores
            if (piece == P)
                score += wPawn_mg[square];
            else if (piece == N)
                score += wKnight_mg[square];
            else if (piece == B)
                score += wBishop_mg[square];
            else if (piece == R)
                score += wRook_mg[square];
            else if (piece == Q)
                score += wQueen_mg[square];
            else if (piece == K)
                score += wKing_mg[square];

            else if (piece == p)
                score -= wPawn_mg[mirror_score[square]];
            else if (piece == n)
                score -= wPawn_mg[mirror_score[square]];
            else if (piece == b)
                score -= wPawn_mg[mirror_score[square]];
            else if (piece == r)
                score -= wPawn_mg[mirror_score[square]];
            else if (piece == q)
                score -= wPawn_mg[mirror_score[square]];
            else if (piece == k)
                score -= wPawn_mg[mirror_score[square]];

            pop_bit(curr_piece_bb, square);
        }

    }

    return (colour_to_move==white) ? score : -score;
}