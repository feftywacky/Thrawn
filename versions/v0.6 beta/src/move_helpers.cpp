#include <iostream>
#include <vector>
#include "constants.h"

/* 
 * METHODS BELOW ENCODE AND DECODED MOVE NUMBER 
*/

// parses move into a single int
int parse_move(int source, int target, int piece, int promoted_piece, int capture, int double_pawn_move, int enpassant, int castling)
{
    return source | (target<<6) | (piece<<12) | (promoted_piece<<16) | (capture<<20) | (double_pawn_move<<21) | (enpassant<<22) | (castling<<23);
}

// following methods unpacks and retrives specific information from move

int get_move_source(int move)
{
    return move & 0x3f;
}

int get_move_target(int move)
{
    return (move & 0xfc0) >> 6;
}

int get_move_piece(int move)
{
    return (move & 0xf000) >> 12;
}

int get_promoted_piece(int move)
{
    return (move & 0xf0000) >> 16;
}

int get_is_capture_move(int move)
{
    return move & 0x100000;
}

int get_is_double_pawn_move(int move)
{
    return move & 0x200000;
}

int get_is_move_enpassant(int move)
{
    return move & 0x400000;
}

int get_is_move_castling(int move)
{
    return move & 0x800000;
}



// PRINTING 


void print_move(const int& move)
{
    if (get_promoted_piece(move))
    {
    std::cout << square_to_coordinates[get_move_source(move)]
              << square_to_coordinates[get_move_target(move)]
              << promoted_pieces.at(get_promoted_piece(move));
    }
    else
    {
        std::cout << square_to_coordinates[get_move_source(move)]
              << square_to_coordinates[get_move_target(move)];
    }
}

void print_move_list(const std::vector<int>& move_list) 
{
    if (move_list.size()==0)
        std::cout<<"NO MOVES AVAILABLE"<<std::endl;

    for (int move : move_list) {
        std::cout << square_to_coordinates[get_move_source(move)]
                  << square_to_coordinates[get_move_target(move)]
                  << promoted_pieces.at(get_promoted_piece(move))
                  << " piece: " << ascii_pieces[get_move_piece(move)]
                  << " " << (get_is_capture_move(move) ? 1 : 0)
                  << " " << (get_is_double_pawn_move(move) ? 1 : 0)
                  << " " << (get_is_move_enpassant(move) ? 1 : 0)
                  << " " << (get_is_move_castling(move) ? 1 : 0)
                  << "" << std::endl;
    }
}


