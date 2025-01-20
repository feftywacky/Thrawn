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

// Store current board into undo_stack[ply]
void thrawn::Position::copyBoard(int ply)
{
    undo_stack[ply].piece_bitboards = piece_bitboards;
    undo_stack[ply].occupancies     = occupancies;
    undo_stack[ply].colour_to_move  = colour_to_move;
    undo_stack[ply].enpassant       = enpassant;
    undo_stack[ply].castle_rights   = castle_rights;
    undo_stack[ply].zobristKey      = zobristKey;
    undo_stack[ply].fifty_move      = fifty_move;
}

// Restore board state from undo_stack[ply]
void thrawn::Position::restoreBoard(int ply)
{
    piece_bitboards = undo_stack[ply].piece_bitboards;
    occupancies     = undo_stack[ply].occupancies;
    colour_to_move  = undo_stack[ply].colour_to_move;
    enpassant       = undo_stack[ply].enpassant;
    castle_rights   = undo_stack[ply].castle_rights;
    zobristKey      = undo_stack[ply].zobristKey;
    fifty_move      = undo_stack[ply].fifty_move;
}