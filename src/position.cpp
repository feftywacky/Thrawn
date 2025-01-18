#include "position.h"
#include "zobrist_hashing.h"
#include "bitboard.h"

// Default constructor
thrawn::Position::Position() : rook_attacks(64, std::vector<uint64_t>(4096)) {
    init_leaping_attacks(*this);
    init_sliding_attacks(*this,bishop);
    init_sliding_attacks(*this,rook);
    init_hashkeys(*this);
}

// Copy the board state
// Copy the board state
void thrawn::Position::copyBoard() {
    piece_bitboards_copy = piece_bitboards;
    occupancies_copy = occupancies;
    colour_to_move_copy = colour_to_move;
    enpassant_copy = enpassant;
    castle_rights_copy = castle_rights;
    zobristKey_copy = zobristKey;
    fifty_move_copy = fifty_move;
}

// Restore the board state
void thrawn::Position::restoreBoard() {
    piece_bitboards = piece_bitboards_copy;
    occupancies = occupancies_copy;
    colour_to_move = colour_to_move_copy;
    enpassant = enpassant_copy;
    castle_rights = castle_rights_copy;
    zobristKey = zobristKey_copy;
    fifty_move = fifty_move_copy;
}