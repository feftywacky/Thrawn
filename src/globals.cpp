#include "globals.h"
#include "constants.h"

TranspositionTable* tt = new TranspositionTable();
thrawn::Position* pos = new thrawn::Position();
ThreadData threadDatas[MAX_THREADS];