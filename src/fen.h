#ifndef FEN_H
#define FEN_H

#include <string>
#include "bitboard.h"

using namespace std;

class Bitboard;

// FEN position test cases
extern const char* empty_board;
extern const char* start_position;
extern const char* tricky_position;

void parse_fen(Bitboard& board, const char* fen);



#endif