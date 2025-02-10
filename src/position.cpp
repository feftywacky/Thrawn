#include "position.h"
#include "zobrist_hashing.h"
#include "bitboard.h"

// Default constructor
thrawn::Position::Position() : repetition_index(0), ply(0) {
    init_leaping_attacks(this);
    init_sliding_attacks(this,bishop);
    init_sliding_attacks(this,rook);
    init_hashkeys(this);
}