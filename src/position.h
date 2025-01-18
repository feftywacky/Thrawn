#ifndef POSITION_H
#define POSITION_H

#include <array>
#include <vector>
#include <cstdint>
#include <string>

namespace thrawn {

class Position {
private:
    // Backup state for undo operations
    std::array<uint64_t, 12> piece_bitboards_copy;
    std::array<uint64_t, 3> occupancies_copy;
    int colour_to_move_copy;
    int enpassant_copy;
    int castle_rights_copy;
    uint64_t zobristKey_copy;
    int fifty_move_copy;

public:
    // Piece bitboards for each piece type and color
    std::array<uint64_t, 12> piece_bitboards;

    // Occupancy bitboards
    std::array<uint64_t, 3> occupancies; // white, black, both

    // Side to move
    int colour_to_move;

    // En passant square
    int enpassant;

    // Castling rights
    int castle_rights;

    // Attack bitboards for pawns, knights, and kings
    std::array<std::array<uint64_t, 64>, 2> pawn_attacks;
    std::array<uint64_t, 64> knight_attacks;
    std::array<uint64_t, 64> king_attacks;

    // Attack masks for bishops and rooks
    std::array<uint64_t, 64> bishop_masks;
    std::array<std::array<uint64_t, 512>, 64> bishop_attacks;
    std::array<uint64_t, 64> rook_masks;
    std::vector<std::vector<uint64_t>> rook_attacks;

    // Zobrist hashing keys
    uint64_t piece_hashkey[12][64];
    uint64_t enpassant_hashkey[64];
    uint64_t castling_hashkey[16];
    uint64_t colour_to_move_hashkey;

    // Zobrist key for the current position
    uint64_t zobristKey;

    // Fifty-move rule counter
    int fifty_move;

    // Default constructor
    Position();

    // Methods
//    void parse_fen(const std::string& fen);
//    void update_occupancies();

    // Copy and restore board state
    void copyBoard();
    void restoreBoard();
};

}
#endif
