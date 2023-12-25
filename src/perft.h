#ifndef PERFT_H
#define PERFT_H

#include "move_generator.h"

extern long leaf_nodes;

void perft_search(int depth);

void perft_test(int depth);

#endif
