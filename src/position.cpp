#include "position.h"
#include "zobrist_hashing.h"
#include "bitboard.h"

// Default constructor
thrawn::Position::Position() : rook_attacks(64, std::vector<uint64_t>(4096)), repetition_index(0) {
    init_leaping_attacks(*this);
    init_sliding_attacks(*this,bishop);
    init_sliding_attacks(*this,rook);
    init_hashkeys(*this);
}