#ifndef FEN_H
#define FEN_H

#include <string>
#include "bitboard.h"
#include "position.h"

using namespace std;

// FEN position test cases
extern const char* empty_board;
extern const char* start_position;
extern const char* position_2;
extern const char* position_3;

void parse_fen(thrawn::Position& pos, const char* fen);

#endif