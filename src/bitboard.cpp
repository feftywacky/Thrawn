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
    // white
    whitePawns = 0x000000000000FF00ULL;
    whiteRooks = 0x0000000000000081ULL;
    whiteKnights = 0x0000000000000042ULL;
    whiteBishops = 0x0000000000000024ULL;
    whiteQueens = 0x0000000000000008ULL;
    whiteKings = 0x0000000000000010ULL;

    // black
    blackPawns = 0x00FF000000000000ULL;
    blackRooks = 0x8100000000000000ULL;
    blackKnights = 0x4200000000000000ULL;
    blackBishops = 0x2400000000000000ULL;
    blackQueens = 0x0800000000000000ULL;
    blackKings = 0x1000000000000000ULL; 

    init_piece_attacks();
    init_sliding_attacks(bishop);
    init_sliding_attacks(rook);
    // init_magic_nums();
}


void Bitboard::printBB()
{
    // Print the chessboard state with a simplified representation
    // For each square, indicate the presence of a piece with 'P', 'R', 'N', 'B', 'Q', 'K', or '-'
    // 'P' represents a white pawn, 'p' represents a black pawn, and so on
    for (int rank = 7; rank >= 0; rank--) {
        for (int file = 0; file < 8; file++) {
            uint64_t mask = 1ULL << (rank * 8 + file);
            
            char piece = '-';
            
            if (whitePawns & mask) piece = 'P';
            if (whiteRooks & mask) piece = 'R';
            if (whiteKnights & mask) piece = 'N';
            if (whiteBishops & mask) piece = 'B';
            if (whiteQueens & mask) piece = 'Q';
            if (whiteKings & mask) piece = 'K';
            if (blackPawns & mask) piece = 'p';
            if (blackRooks & mask) piece = 'r';
            if (blackKnights & mask) piece = 'n';
            if (blackBishops & mask) piece = 'b';
            if (blackQueens & mask) piece = 'q';
            if (blackKings & mask) piece = 'k';

            std::cout << piece << ' ';
        }
        std::cout << std::endl;
    }
}




// GET CERTAIN BITBOARDS
uint64_t Bitboard::get_white_pieces()
{
    return whitePawns | whiteRooks | whiteKnights | whiteBishops | whiteQueens | whiteKings;
}

uint64_t Bitboard::get_black_pieces()
{
    return blackPawns | blackRooks | blackKnights | blackBishops | blackQueens | blackKings;
}


// PRE-COMPUTE PIECE ATTACK BITBOARDS


// pawns
uint64_t Bitboard::get_pawn_attack_from_sq(Side side, int square)
{
    uint64_t attacks = 0ULL;
    uint64_t bitboard = 0ULL;

    bitboard = set_bit(bitboard, square);

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

    bitboard = set_bit(bitboard, square);

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
    bitboard = set_bit(bitboard, square);

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
        bishop_masks[square] = get_bishop_attack_from_sq(square);
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

inline uint64_t Bitboard::get_rook_attacks(int square, uint64_t occupancy)
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





