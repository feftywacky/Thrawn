#define NNUE_H
#ifdef NNUE_H

extern int nnue_pieces[12];
extern int nnue_squares[64];

void init_nnue(const char* nnue_filename);

int evaluate_nnue(int player,int* pieces,int* squares);

int evaluate_fen_nnue(const char* fen);

void parse_nnue_args(int *pieces, int *squares);

#endif