#ifndef PERFT_H
#define PERFT_H

#include "move_generator.h"
#include "position.h"

extern long leaf_nodes;

void perft_search(thrawn::Position& pos, int depth);

int perft_test(thrawn::Position& pos, int depth);

void perft_run_unit_tests();

#endif
