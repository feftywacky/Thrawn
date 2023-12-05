#ifndef FEN_H
#define FEN_H

#include <string>
#include "bitboard.h"

using namespace std;

// FEN position test cases
extern const char* empty_board;
extern const char* start_position;
extern const char* tricky_position;

void parse_fen(const char* fen);



#endif