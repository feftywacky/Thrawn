#include "fen.h"
#include "bitboard.h"
#include "bitboard_helpers.h"
#include "constants.h"
#include <string>

using namespace std;

class Bitboard;

const char* empty_board = "8/8/8/8/8/8/8/8 w - - ";
const char* start_position = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1 ";
const char* position_2 = "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1 ";

void parse_fen(const char* fen)
{
    // reset piece bitboards and occupancies
    for (int i = 0; i < piece_bitboards.size(); i++)
        piece_bitboards[i] = 0ULL;

    for (int i = 0; i < occupancies.size(); i++)
        occupancies[i] = 0ULL;
    
    // reset gameState variables
    colour_to_move = white;
    enpassant = null_sq;
    castle_rights = 0;

    // loop to parse pieces and empty squares from fen
    for (int r=0;r<8;r++)
    {
        for (int c=0;c<8;c++)
        {
            int square = r*8+c;

            // check if current fen char is a letter and init its corresponding piece
            if ( (*fen >= 'a' && *fen<='z') || (*fen>='A' && *fen<='Z') )
            {
                int piece = char_pieces.at(*fen);
                set_bit(piece_bitboards[piece], square);
                *fen++;
            }

            // check for numbers in fen which represents # of empty squares
            if (*fen>='0' && *fen<='9')
            {
                int empty_squares = *fen-'0';
                int piece = -1;

                // loops over all the pieces (white pawn -> black king)
                for (int i=P;i<=k;i++)
                {
                    if (get_bit(piece_bitboards[i], square))
                        piece = i;
                }
                
                // going back one if no piece is on the square
                if (piece==-1)
                    c--;
                c+=empty_squares;
                fen++;
            }

            // goes to next rank
            if (*fen == '/')
                fen++;
        }
    }

    // go to parse colour_to_move value in fen string
    fen++;

    // parsing colour_to_move
    *fen == 'w' ? (colour_to_move = white) : (colour_to_move = black);

    // go to parse castling rights value in fen string
    fen += 2;

    // parsing castle rights
    while(*fen != ' ')
    {
        if (*fen == 'K')
            castle_rights |= wks;
        else if (*fen == 'Q')
            castle_rights |= wqs;
        else if (*fen == 'k')  
            castle_rights |= bks;
        else if (*fen == 'q')
            castle_rights |= bqs;
        else if (*fen == '-')
            break;
        
        fen++;
    }

    // go to parse enpassant square value in fen string
    fen++;
    if (*fen == ' ')
        fen++;
    
    // parsing enpassant square
    if (*fen != '-')
    {
        int col = *fen - 'a';
        cout<<"fen1: "<<*fen-'a'<<endl;
        // to get to row
        *fen++;
        cout<<"fen2: "<<*fen<<endl;
        int row = 8- (*fen -'0');

        enpassant = row*8+col;

    }
    else 
        enpassant = null_sq;
    
    // setting white, black, and both occupancies
    occupancies[0] = get_white_occupancy();
    occupancies[1] = get_black_occupancy();
    occupancies[2] = get_both_occupancy();
}