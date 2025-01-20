#ifndef POSITION_H
#define POSITION_H

#include <array>
#include <vector>
#include <cstdint>

const int MAX_DEPTH = 64; 

namespace thrawn {

// Holds all data needed to restore a position
struct BoardState {
    std::array<uint64_t, 12> piece_bitboards;
    std::array<uint64_t, 3>  occupancies;
    int colour_to_move;
    int enpassant;
    int castle_rights;
    uint64_t zobristKey;
    int fifty_move;
};

class Position {
public:
    //============= CURRENT BOARD STATE =============//

    std::array<uint64_t, 12> piece_bitboards;
    std::array<uint64_t, 3>  occupancies;
    int colour_to_move;
    int enpassant;
    int castle_rights;
    uint64_t zobristKey;
    int fifty_move;

    std::array<uint64_t, 1028> repetition_table; 
    int repetition_index;

    //============= ATTACK AND HASHING TABLES =============//

    std::array<std::array<uint64_t, 64>, 2> pawn_attacks;
    std::array<uint64_t, 64> knight_attacks;
    std::array<uint64_t, 64> king_attacks;

    std::array<uint64_t, 64> bishop_masks;
    std::array<std::array<uint64_t, 512>, 64> bishop_attacks;
    std::array<uint64_t, 64> rook_masks;
    std::vector<std::vector<uint64_t>> rook_attacks;

    uint64_t piece_hashkey[12][64];
    uint64_t enpassant_hashkey[64];
    uint64_t castling_hashkey[16];
    uint64_t colour_to_move_hashkey;

    //============= UNDO STACK =============//
    BoardState undo_stack[MAX_DEPTH];

    //============= CONSTRUCTORS & METHODS =============//
    Position();

    void copyBoard(int ply);
    void restoreBoard(int ply);
};

} // namespace thrawn

#endif
