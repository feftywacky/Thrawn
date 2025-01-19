#ifndef FEN_H
#define FEN_H

#include <string>
#include "bitboard.h"
#include "position.h"

using namespace std;

void parse_fen(thrawn::Position& pos, const char* fen);

#endif