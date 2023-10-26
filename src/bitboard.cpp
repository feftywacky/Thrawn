#include "bitboard.hpp"
#include <iostream>
#include <vector>
#include <string>
#include <map>
#include <set>
#include <bitset>


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

    init_pawn_attacks();
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

void Bitboard::init_pawn_attacks()
{
    for (int square = 0; square < BOARD_SIZE; square++) 
    {
        pawn_attacks[white][square] = get_pawn_attack_from_sq(white, square);
        pawn_attacks[black][square] = get_pawn_attack_from_sq(black, square);
    }
}







