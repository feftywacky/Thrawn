#include "bitboard.hpp"
#include <iostream>
#include <vector>
#include <string>
#include <map>
#include <set>
#include <bitset>
#include <cstring> 

using namespace std;


// contructor
Bitboard::Bitboard()
{
    // init start game pieces
    init_white_pieces();
    init_black_pieces();

    occupancies[0] = get_white_occupancy();
    occupancies[1] = get_black_occupancy();
    occupancies[2] = get_both_occupancy();

    colour_to_move = white;
    enpassant = null_sq;

    castle_rights |= wks;
    castle_rights |= wqs;
    castle_rights |= bks;
    castle_rights |= bqs;

    // resize attack rook table since it's a vector
    rook_attacks.resize(64, std::vector<uint64_t>(4096, 0));

    init_piece_attacks();
    init_sliding_attacks(bishop);
    init_sliding_attacks(rook);
    // init_magic_nums();
}

// GET OCCUPANCY BITBOARDS
uint64_t Bitboard::get_white_occupancy()
{
    uint64_t res = 0ULL;
    for(int i=0;i<6;i++)
        res |= piece_bitboards[i];
    return res;
}

uint64_t Bitboard::get_black_occupancy()
{
    uint64_t res = 0ULL;
    for(int i=6;i<12;i++)
        res |= piece_bitboards[i];
    return res;
}

uint64_t Bitboard::get_both_occupancy()
{
    uint64_t res = 0ULL;
    res |= get_white_occupancy();
    res |= get_black_occupancy();
    return res;
}


// PRE-COMPUTE PIECE ATTACK BITBOARDS


// pawns
uint64_t Bitboard::get_pawn_attack_from_sq(Side side, int square)
{
    uint64_t attacks = 0ULL;
    uint64_t bitboard = 0ULL;

    set_bit(bitboard, square);

    if (side == white) 
    {
        if ((bitboard >> 7) & not_a_file) 
            attacks |= (bitboard >> 7);
        if ((bitboard >> 9) & not_h_file) 
            attacks |= (bitboard >> 9);
    }
    else 
    {
        if ((bitboard << 7) & not_h_file) 
            attacks |= (bitboard << 7);
        if ((bitboard << 9) & not_a_file) 
            attacks |= (bitboard << 9);
    }

    return attacks;
}

// knights
uint64_t Bitboard::get_knight_attack_from_sq(int square)
{
    uint64_t attacks = 0ULL;
    uint64_t bitboard = 0ULL;

    set_bit(bitboard, square);

    if ((bitboard >> 17) & not_h_file) 
        attacks |= (bitboard >> 17);
    if ((bitboard >> 15) & not_a_file) 
        attacks |= (bitboard >> 15);
    if ((bitboard >> 10) & not_hg_file) 
        attacks |= (bitboard >> 10);
    if ((bitboard >> 6) & not_ab_file) 
        attacks |= (bitboard >> 6);
    if ((bitboard << 17) & not_a_file) 
        attacks |= (bitboard << 17);
    if ((bitboard << 15) & not_h_file) 
        attacks |= (bitboard << 15);
    if ((bitboard << 10) & not_ab_file) 
        attacks |= (bitboard << 10);
    if ((bitboard << 6) & not_hg_file) 
        attacks |= (bitboard << 6);
    
    return attacks;
}

uint64_t Bitboard::get_king_attack_from_sq(int square)
{
    uint64_t attacks = 0ULL;
    uint64_t bitboard = 0ULL;
    set_bit(bitboard, square);

    if (bitboard>>8)
        attacks |= (bitboard>>8);
    if ((bitboard>>9) & not_h_file)
        attacks |= (bitboard>>9);
    if ((bitboard>>7) & not_a_file)
        attacks |= (bitboard>>7);
    if ((bitboard >> 1) & not_h_file) 
        attacks |= (bitboard >> 1);

    if (bitboard<<8)
        attacks |= (bitboard<<8);
    if ((bitboard<<9) & not_a_file)
        attacks |= (bitboard<<9);
    if ((bitboard<<7) & not_h_file)
        attacks |= (bitboard<<7);
    if ((bitboard << 1) & not_a_file) 
        attacks |= (bitboard << 1);

    return attacks;
}

// does not include squares on the edge of the board
uint64_t Bitboard::get_bishop_attack_from_sq(int square)
{
    uint64_t attacks = 0ULL;

    int row; int col;
    int curr_row = square/8; 
    int curr_col = square%8;

    // mask relevant bishop occupancy bits

    // bottom right diagonal 
    for (row = curr_row+1, col = curr_col+1; row<=6 && col<=6; row++,col++)
        attacks |= (1ULL << row*8 + col);
    
    // bottom left diagonal 
    for (row = curr_row+1, col = curr_col-1; row<=6 && col>=1; row++, col--)
        attacks |= (1ULL << row*8 + col);

    // top right diagonal
    for (row = curr_row-1, col = curr_col+1; row>=1 && col<=6; row--, col++)
        attacks |= (1ULL << row*8 + col);
    
    // top left diagonal
    for (row = curr_row-1, col = curr_col-1; row>=1 && col>=1; row--, col--)
        attacks |= (1ULL << row*8 + col);

    return attacks;    
}

// does not distinguish the colour of the blocker
uint64_t Bitboard::bishop_attack_runtime_gen(int square, uint64_t blockers)
{
    uint64_t attacks = 0ULL;

    int row; int col;
    int curr_row = square/8; 
    int curr_col = square%8;

    // generate bishop attacks

    // bottom right diagonal 
    for (row = curr_row+1, col = curr_col+1; row<=7 && col<=7; row++,col++)
    {
        attacks |= (1ULL << row*8 + col);
        if ( (1ULL << row*8 + col) & blockers )
            break;
    }
    
    // bottom left diagonal 
    for (row = curr_row+1, col = curr_col-1; row<=7 && col>=0; row++, col--)
    {
        attacks |= (1ULL << row*8 + col);
         if ( (1ULL << row*8 + col) & blockers )
            break;
    }

    // top right diagonal
    for (row = curr_row-1, col = curr_col+1; row>=0 && col<=7; row--, col++)
    {
        attacks |= (1ULL << row*8 + col);
         if ( (1ULL << row*8 + col) & blockers )
            break;
    }

    // top left diagonal
    for (row = curr_row-1, col = curr_col-1; row>=0 && col>=0; row--, col--)
    {
        attacks |= (1ULL << row*8 + col);
         if ( (1ULL << row*8 + col) & blockers )
            break;
    }

    return attacks; 
}

uint64_t Bitboard::get_rook_attack_from_sq(int square)
{
    uint64_t attacks = 0ULL;

    int row; int col;
    int curr_row = square/8; 
    int curr_col = square%8;

    // up
    for (row = curr_row-1; row>=1; row--)
        attacks |= (1ULL << row*8 + curr_col);

    // down
    for (row = curr_row+1; row<=6; row++)
        attacks |= (1ULL << row*8 + curr_col);

    // left
    for (col = curr_col-1; col >=1; col--)
        attacks |= (1ULL << curr_row*8 + col);

    // right
    for (col = curr_col+1; col<=6; col++)
        attacks |= (1ULL << curr_row*8 + col);

    return attacks;
}

// does not distinguish the colour of the blocker
uint64_t Bitboard::rook_attack_runtime_gen(int square, uint64_t blockers)
{
    uint64_t attacks = 0ULL;

    int row; int col;
    int curr_row = square/8; 
    int curr_col = square%8;

    // up
    for (row = curr_row-1; row>=0; row--)
    {
        attacks |= (1ULL << row*8 + curr_col);
        if ( (1ULL << row*8 + curr_col) & blockers )
            break;
    }

    // down
    for (row = curr_row+1; row<=7; row++)
    {
        attacks |= (1ULL << row*8 + curr_col);
        if ( (1ULL << row*8 + curr_col) & blockers )
            break;
    }

    // left
    for (col = curr_col-1; col >=0; col--)
    {
        attacks |= (1ULL << curr_row*8 + col);
        if ( (1ULL << curr_row*8 + col) & blockers)
            break;
    }

    // right
    for (col = curr_col+1; col<=7; col++)
    {
        attacks |= (1ULL << curr_row*8 + col);
        if ( (1ULL << curr_row*8 + col) & blockers)
            break;
    }

    return attacks;
}

uint64_t Bitboard::set_occupancy(int index, int bits_in_mask, uint64_t attack_mask)
{
    uint64_t occupancy = 0ULL;

    for (int i=0;i<bits_in_mask;i++)
    {
        int square = get_lsb_index(attack_mask);
        attack_mask = clear_bit(attack_mask, square);

        // make sure occupancy is on board
        if (index & (1 << i))
            // populate occupancy map
            occupancy |= (1ULL << square);
    }
    return occupancy;
}

// MAGIC BITBOARD
uint64_t Bitboard::find_magic_num(int square, int relevant_bits, int bishop)
{
    array<uint64_t, 4096> occupancies;
    array<uint64_t, 4096> attacks;
    array<uint64_t, 4096> used_attacks;
    
    uint64_t attack_mask = bishop ? get_bishop_attack_from_sq(square) : get_rook_attack_from_sq(square);

    // init occupancy indices
    int occupancy_index = 1 << relevant_bits;

    for (int i=0;i<occupancy_index;i++)
    {
        occupancies[i] = set_occupancy(i, relevant_bits, attack_mask);
        attacks[i] = bishop ? bishop_attack_runtime_gen(square, occupancies[i]) : rook_attack_runtime_gen(square, occupancies[i]);
    }
    
    // testing magic numbers
    for (int random_count = 0; random_count < 100000000; random_count++) 
    {
         uint64_t magic_num = gen_magic_num();

        // skip inappropriate magic numbers
        if (count_bits((attack_mask * magic_num) & 0xFF00000000000000ULL) < 6) continue;

        // init used attacks
        used_attacks.fill(0ULL);

        // init index & fail flag
        int index, fail;

        // test magic index loop
        for (index = 0, fail = 0; !fail && index < occupancy_index; index++) 
        {
            // init magic index
            int magic_index = (int)((occupancies[index] * magic_num) >> (64 - relevant_bits));

            // if magic index works
            if (used_attacks[magic_index] == 0ULL)
                // init used attacks
                used_attacks[magic_index] = attacks[index];

            else if (used_attacks[magic_index] != attacks[index])
                // magic index doesn't work
                fail = 1;
        }

        // if magic number works
        if (!fail)
            return magic_num;
    }

    // if magic number doesn't work
    std::cout << "Magic number fails!" << std::endl;
    return 0ULL;
}


void Bitboard::init_magic_nums()
{
    for (int square = 0; square < 64; square++)
        // init rook magic numbers
        rook_magic_nums[square] = find_magic_num(square, rook_relevant_bits[square], rook);

    for (int square = 0; square < 64; square++)
        // init bishop magic numbers
        bishop_magic_nums[square] = find_magic_num(square, bishop_relevant_bits[square], bishop);
}

void Bitboard::init_sliding_attacks(int isBishop)
{
    for (int square = 0;square<64;square++)
    {
        if (isBishop)
            bishop_masks[square] = get_bishop_attack_from_sq(square);
        else
            rook_masks[square] = get_rook_attack_from_sq(square);

        uint64_t curr_attack_mask = isBishop ? bishop_masks[square] : rook_masks[square];

        int relevant_bits_count = count_bits(curr_attack_mask);

        int occupancy_index = 1 << relevant_bits_count;   

        for (int i=0;i<occupancy_index;i++)
        {
            if (isBishop)
            {
                uint64_t occupancy = set_occupancy(i, relevant_bits_count, curr_attack_mask);
                int magic_index = (occupancy * bishop_magic_nums[square]) >> (64-bishop_relevant_bits[square]);
                bishop_attacks[square][magic_index] = bishop_attack_runtime_gen(square, occupancy);
            }
            else
            {
                uint64_t occupancy = set_occupancy(i, relevant_bits_count, curr_attack_mask);
                int magic_index = (occupancy * rook_magic_nums[square]) >> (64-rook_relevant_bits[square]);
                rook_attacks[square][magic_index] = rook_attack_runtime_gen(square, occupancy);
            }
        }
    }
}

uint64_t Bitboard::get_bishop_attacks(int square, uint64_t occupancy)
{
    // generate bishop attacks given current board occupancy
    occupancy &= bishop_masks[square];
    occupancy *= bishop_magic_nums[square];
    occupancy >>= 64-bishop_relevant_bits[square];

    return bishop_attacks[square][occupancy];
}

uint64_t Bitboard::get_rook_attacks(int square, uint64_t occupancy)
{
    // generate rook attacks given current board occupancy
    occupancy &= rook_masks[square];
    occupancy *= rook_magic_nums[square];
    occupancy >>= 64-rook_relevant_bits[square];

    return rook_attacks[square][occupancy];
}

void Bitboard::init_piece_attacks()
{
    for (int square = 0; square < BOARD_SIZE; square++) 
    {
        pawn_attacks[white][square] = get_pawn_attack_from_sq(white, square);
        pawn_attacks[black][square] = get_pawn_attack_from_sq(black, square);
        knight_attacks[square] = get_knight_attack_from_sq(square);
        king_attacks[square] = get_king_attack_from_sq(square);
    }
}

void Bitboard::init_white_pieces()
{
    // set white pawns
    set_bit(piece_bitboards[P], a2);
    set_bit(piece_bitboards[P], b2);
    set_bit(piece_bitboards[P], c2);
    set_bit(piece_bitboards[P], d2);
    set_bit(piece_bitboards[P], e2);
    set_bit(piece_bitboards[P], f2);
    set_bit(piece_bitboards[P], g2);
    set_bit(piece_bitboards[P], h2);
    
    // set white knights
    set_bit(piece_bitboards[N], b1);
    set_bit(piece_bitboards[N], g1);
    
    // set white bishops
    set_bit(piece_bitboards[B], c1);
    set_bit(piece_bitboards[B], f1);
    
    // set white rooks
    set_bit(piece_bitboards[R], a1);
    set_bit(piece_bitboards[R], h1);
    
    // set white queen & king
    set_bit(piece_bitboards[Q], d1);
    set_bit(piece_bitboards[K], e1);
}

void Bitboard::init_black_pieces()
{
    // set black pawns
    set_bit(piece_bitboards[p], a7);
    set_bit(piece_bitboards[p], b7);
    set_bit(piece_bitboards[p], c7);
    set_bit(piece_bitboards[p], d7);
    set_bit(piece_bitboards[p], e7);
    set_bit(piece_bitboards[p], f7);
    set_bit(piece_bitboards[p], g7);
    set_bit(piece_bitboards[p], h7);
    
    // set black knights
    set_bit(piece_bitboards[n], b8);
    set_bit(piece_bitboards[n], g8);
    
    // set black bishops
    set_bit(piece_bitboards[b], c8);
    set_bit(piece_bitboards[b], f8);
    
    // set black rooks
    set_bit(piece_bitboards[r], a8);
    set_bit(piece_bitboards[r], h8);
    
    // set black queen & king
    set_bit(piece_bitboards[q], d8);
    set_bit(piece_bitboards[k], e8);
}

