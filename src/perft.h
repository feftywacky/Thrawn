#ifndef PERFT_H
#define PERFT_H

#include "engine.h"

extern long leaf_nodes;

void perft_search(int depth, Engine& engine, Bitboard& board);

void perft_test(int depth, Engine& engine, Bitboard& board);

#endif