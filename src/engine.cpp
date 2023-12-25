#include "engine.h"
#include "bitboard.h"
#include "constants.h"
#include "bitboard_helpers.h"
#include "move_helpers.h"
#include "zobrist.h"
#include <iostream>
#include <vector>
#include <string>
#include <map>
#include <set>
#include <bitset>

using namespace std;

vector<int> generate_moves()
{
    vector<int> moves;

    // the squares where the pieces started from, and where it will go
    int source;
    int target;

    // bitboard of the current piece, and its attacks
    uint64_t curr; 
    uint64_t attacks;

    // loop over all piece types for both black and white
    for (int piece = P; piece<=k; piece++)
    {
        uint64_t curr = piece_bitboards[piece];

        // to distinguish betweem white and black specific moves
        // includes pawns and castling
        if (colour_to_move == white)
        {
            // generate pawn moves
            if (piece==P)
            {
                // double pawn moves, pawn promotion, enpassant
                parse_white_pawn_moves(curr, moves);
            }

            // castling
            if (piece == K)
            {
                parse_white_castle_moves(moves);
            }
        }

        // for black pieces
        else
        {
            if (piece==p)
            {
                parse_black_pawn_moves(curr, moves);
            }

            // castling
            if (piece == k)
            {
               parse_black_castle_moves(moves);
            }
        }

        // generate for the rest of the pieces that are not colour specific
        // knight
        if ( (colour_to_move == white) ? piece == N : piece == n )
        {
            parse_knight_moves(curr, piece, moves);
        }

        // bishop
        if ( (colour_to_move == white) ? piece == B : piece == b )
        {
            parse_bishop_moves(curr, piece, moves);
        }
        
        // rook
        if ( (colour_to_move == white) ? piece == R : piece == r )
        {
           parse_rook_moves(curr, piece, moves);
        }

        // queen
        if ( (colour_to_move == white) ? piece == Q : piece == q )
        {
            parse_queen_moves(curr, piece, moves);
        }

        // king
        if ( (colour_to_move == white) ? piece == K : piece == k )
        {
            parse_king_moves(curr, piece, moves);
        }

    } // end of looping through all pieces

    return moves;


}

void parse_white_pawn_moves(uint64_t& curr, vector<int>& moves)
{
    while (curr)
    {
        int source = get_lsb_index(curr);
        int target = source - 8; // go up one square

        if (target>=a8 && !get_bit(occupancies[both], target))
        {
            // pawn promotion by going up one square (NOT TAKING A PIECE)
            if (source>=a7 && source<=h7)
            {
                moves.push_back(parse_move(source, target, P, Q, 0, 0, 0, 0));
                moves.push_back(parse_move(source, target, P, R, 0, 0, 0, 0));
                moves.push_back(parse_move(source, target, P, N, 0, 0, 0, 0));
                moves.push_back(parse_move(source, target, P, B, 0, 0, 0, 0));
            }

            // one square and two square pawn moves
            else
            {
                // one square
                moves.push_back(parse_move(source, target, P, 0, 0, 0, 0, 0));

                // two square
                if (source>=a2 && source<=h2 && !get_bit(occupancies[both], target-8))
                {
                    moves.push_back(parse_move(source, target-8, P, 0, 0, 1, 0, 0));
                }
            }
        }

        uint64_t attacks = pawn_attacks[colour_to_move][source]  & occupancies[black];

        while (attacks) // while attacks squares are present on the board
        {   
            target = get_lsb_index(attacks);

            if (source>=a7 && source<=h7) // pawn promotions by capturing a piece
            {
                moves.push_back(parse_move(source, target, P, Q, 1, 0, 0, 0));
                moves.push_back(parse_move(source, target, P, R, 1, 0, 0, 0));
                moves.push_back(parse_move(source, target, P, N, 1, 0, 0, 0));
                moves.push_back(parse_move(source, target, P, B, 1, 0, 0, 0));
            }

            // diagonal pawn capture
            else
            {
                moves.push_back(parse_move(source, target, P, 0, 1, 0, 0, 0));
            }

            pop_bit(attacks, target);
        }

        // enpassant
        if (enpassant!=null_sq)
        {
            
            uint64_t enpassant_attacks = pawn_attacks[colour_to_move][source] & (1ULL << enpassant);
            if (enpassant_attacks)
            {
                int enpassant_target = get_lsb_index(enpassant_attacks);
                moves.push_back(parse_move(source, enpassant_target, P, 0, 1, 0, 1, 0));
            }
        }

        // remove ls1b for looping through all the bits
        pop_bit(curr, source);
    }
}

void parse_white_castle_moves(vector<int>& moves)
{
    if (castle_rights & wks)
    {
        if (!get_bit(occupancies[both], f1) && !get_bit(occupancies[both], g1))
        {
            // make sure can't castle through check
            // if (!is_square_under_attack(e8, white) && !is_square_under_attack(f1, black) && !is_square_under_attack(g1, black))
            if (!is_square_under_attack(e1, black) && !is_square_under_attack(f1, black))
                moves.push_back(parse_move(e1, g1, K, 0, 0, 0, 0, 1));
        }
    }
    if (castle_rights & wqs)
    {
        if (!get_bit(occupancies[both], b1) && !get_bit(occupancies[both], c1) && !get_bit(occupancies[both], d1))
        {
            // make sure can't castle through check
            // if (!is_square_under_attack(e1, black) && !is_square_under_attack(c1, black) && !is_square_under_attack(d1, black))
            if (!is_square_under_attack(e1, black) && !is_square_under_attack(d1, black))
                moves.push_back(parse_move(e1, c1, K, 0, 0, 0, 0, 1));
        }
    }
}

void parse_black_pawn_moves(uint64_t& curr, vector<int>& moves)
{
    while(curr) // while white pawns are present on the board
    {
        int source = get_lsb_index(curr);
        int target = source + 8; // go down one square

        if (target<=h1 && !get_bit(occupancies[both], target))
        {
            // pawn promotion by going down one square (NOT TAKING A PIECE)
            if (source>=a2 && source<=h2)
            {
                moves.push_back(parse_move(source, target, p, q, 0, 0, 0, 0));
                moves.push_back(parse_move(source, target, p, r, 0, 0, 0, 0));
                moves.push_back(parse_move(source, target, p, n, 0, 0, 0, 0));
                moves.push_back(parse_move(source, target, p, b, 0, 0, 0, 0));
            }

            // one square and two square pawn moves
            else
            {
                // one square
                moves.push_back(parse_move(source, target, p, 0, 0, 0, 0, 0));

                // two square
                if (source>=a7 && source<=h7 && !get_bit(occupancies[both], target+8))
                    moves.push_back(parse_move(source, target+8, p, 0, 0, 1, 0, 0));
            }
        }

        uint64_t attacks = pawn_attacks[colour_to_move][source] & occupancies[white];

        while (attacks) // while attacks squares are present on the board
        {   
            target = get_lsb_index(attacks);

            if (source>=a2 && source<=h2) // pawn promotion by capturing piece
            {
                moves.push_back(parse_move(source, target, p, q, 1, 0, 0, 0));
                moves.push_back(parse_move(source, target, p, r, 1, 0, 0, 0));
                moves.push_back(parse_move(source, target, p, n, 1, 0, 0, 0));
                moves.push_back(parse_move(source, target, p, b, 1, 0, 0, 0));
            }

            // diagonal pawn capture
            else
            {
                moves.push_back(parse_move(source, target, p, 0, 1, 0, 0, 0));
            }

            pop_bit(attacks, target);
        }

        // enpassant
        if (enpassant!=null_sq)
        {   
            uint64_t enpassant_attacks = pawn_attacks[colour_to_move][source] & (1ULL << enpassant);
            if (enpassant_attacks)
            {
                int enpassant_target = get_lsb_index(enpassant_attacks);
                moves.push_back(parse_move(source, enpassant_target, p, 0, 1, 0, 1, 0));
            }
        }

        // remove ls1b for looping through all the bits
        pop_bit(curr, source);
    }
}

void parse_black_castle_moves(vector<int>& moves)
{
    if (castle_rights & bks)
    {
        if (!get_bit(occupancies[both], f8) && !get_bit(occupancies[both], g8))
        {
            // pruend by make_move() for g8
            // if (!is_square_under_attack(e8, white) && !is_square_under_attack(f8, white) && !is_square_under_attack(g8, white))
            if (!is_square_under_attack(e8, white) && !is_square_under_attack(f8, white))
                moves.push_back(parse_move(e8, g8, k, 0, 0, 0, 0, 1));
        }
    }
    if (castle_rights & bqs)
    {
        if (!get_bit(occupancies[both], b8) && !get_bit(occupancies[both], c8) && !get_bit(occupancies[both], d8))
        {
            // pruend by make_move() 
            // if (!is_square_under_attack(e8, white) && !is_square_under_attack(c8, white) && !is_square_under_attack(d8, white))
            if (!is_square_under_attack(e8, white) && !is_square_under_attack(d8, white))
                moves.push_back(parse_move(e8, c8, k, 0, 0, 0, 0, 1));
        }
    }
}

void parse_knight_moves(uint64_t& curr, const int& piece, vector<int>& moves)
{
    while (curr)
    {
        int source = get_lsb_index(curr);

        uint64_t attacks = knight_attacks[source] & ( (colour_to_move==white) ? ~occupancies[white] : ~occupancies[black]);

        while (attacks)
        {
            int target = get_lsb_index(attacks);
            
            // non-capture move
            if ( !get_bit( (colour_to_move==white) ? occupancies[black] : occupancies[white], target ) )
            {
                moves.push_back(parse_move(source, target, piece, 0, 0, 0, 0, 0));
            }

            else
            {
                moves.push_back(parse_move(source, target, piece, 0, 1, 0, 0, 0));
            }

            pop_bit(attacks, target);
        }
        pop_bit(curr, source);
    }
}

void parse_bishop_moves(uint64_t& curr, const int& piece, vector<int>& moves)
{
    while(curr)
    {
        int source = get_lsb_index(curr);

        uint64_t attacks = get_bishop_attacks(source, occupancies[both]) & ( (colour_to_move==white) ? ~occupancies[white] : ~occupancies[black]);
        while (attacks)
        {
            
            int target = get_lsb_index(attacks);
            
            // non-capture move
            if ( !get_bit( (colour_to_move==white) ? occupancies[black] : occupancies[white], target ) )
            {
                moves.push_back(parse_move(source, target, piece, 0, 0, 0, 0, 0));
            }

            else
            {
                moves.push_back(parse_move(source, target, piece, 0, 1, 0, 0, 0));
            }

            pop_bit(attacks, target);
            
        }
        pop_bit(curr, source);
    } 
}

void parse_rook_moves(uint64_t& curr, const int& piece, vector<int>& moves)
{
    while(curr)
    {
        int source = get_lsb_index(curr);

        uint64_t attacks = get_rook_attacks(source, occupancies[both]) & ( (colour_to_move==white) ? ~occupancies[white] : ~occupancies[black]);

        while (attacks)
        {
            int target = get_lsb_index(attacks);
            
            // non-capture move
            if ( !get_bit( (colour_to_move==white) ? occupancies[black] : occupancies[white], target ) )
            {
                moves.push_back(parse_move(source, target, piece, 0, 0, 0, 0, 0));
            }

            else
            {
                moves.push_back(parse_move(source, target, piece, 0, 1, 0, 0, 0));
            }

            pop_bit(attacks, target);
        }
        pop_bit(curr, source);
    }
}

void parse_queen_moves(uint64_t& curr, const int& piece, vector<int>& moves)
{
    while (curr)
    {
        int source = get_lsb_index(curr);

        uint64_t attacks = get_queen_attacks(source, occupancies[both]) & ( (colour_to_move==white) ? ~occupancies[white] : ~occupancies[black]);

        while (attacks)
        {
            int target = get_lsb_index(attacks);
            
            // non-capture move
            if ( !get_bit( (colour_to_move==white) ? occupancies[black] : occupancies[white], target ) )
            {
                moves.push_back(parse_move(source, target, piece, 0, 0, 0, 0, 0));
            }

            else
            {
                moves.push_back(parse_move(source, target, piece, 0, 1, 0, 0, 0));
            }

            pop_bit(attacks, target);
        }
        pop_bit(curr, source);
    }
}

void parse_king_moves(uint64_t& curr, const int& piece, vector<int>& moves)
{
    while (curr)
    {
        int source = get_lsb_index(curr);

        uint64_t attacks = king_attacks[source] & ( (colour_to_move==white) ? ~occupancies[white] : ~occupancies[black]);

        while (attacks)
        {
            int target = get_lsb_index(attacks);
            
            // non-capture move
            if ( !get_bit( (colour_to_move==white) ? occupancies[black] : occupancies[white], target ) )
            {
                moves.push_back(parse_move(source, target, piece, 0, 0, 0, 0, 0));
            }

            else
            {
                moves.push_back(parse_move(source, target, piece, 0, 1, 0, 0, 0));
            }

            pop_bit(attacks, target);
        }
        pop_bit(curr, source);
    }
}

int make_move(int move, int move_type)
{
    if (move_type == all_moves)
    {
        copyBoard();

        // move parsing
        int source = get_move_source(move);
        int target = get_move_target(move);
        int piece = get_move_piece(move);
        int promoted_piece = get_promoted_piece(move);
        int is_capture_move = get_is_capture_move(move);
        int double_pawn_move = get_is_double_pawn_move(move);
        int enpassant_move = get_is_move_enpassant(move);
        int castling = get_is_move_castling(move);

        // move piece
        pop_bit(piece_bitboards[piece], source);
        set_bit(piece_bitboards[piece], target);

        position_hashkey ^= piece_hashkey[piece][source]; // update hash to exclude source 
        position_hashkey ^= piece_hashkey[piece][target]; // update hash to include target

        // if capture move, remove the piece being captured from its corresponding bitboard
        // ie. if white pawn captures black kngiht, remove black knight from black knight bitboard
        if (is_capture_move)
        {
            int start_piece;
            int end_piece;
            
            (colour_to_move==white) ? start_piece = p : start_piece = P;
            (colour_to_move==white) ? end_piece = k : end_piece = K;

            for(int i=start_piece; i<=end_piece;i++)
            {
                if (get_bit(piece_bitboards[i], target))
                {
                    pop_bit(piece_bitboards[i], target);
                    
                    // update hashkey to exclude captured piece
                    position_hashkey ^= piece_hashkey[i][target]; 
                    break;
                }
            }
        }

        // handle pawn promotions
        if (promoted_piece)
        {
            // erase the pawn from the target square
            // pop_bit(piece_bitboards[(colour_to_move == white) ? P : p], target);
            if (colour_to_move == white)
            {
                pop_bit(piece_bitboards[P], target);
                position_hashkey ^= piece_hashkey[P][target];
            }
            else
            {
                pop_bit(piece_bitboards[p], target);
                position_hashkey ^= piece_hashkey[p][target];
            }
            
            set_bit(piece_bitboards[promoted_piece], target);
            position_hashkey ^= piece_hashkey[promoted_piece][target];
        }

        // handle enpassant capture
        if (enpassant_move)
        {
            // target + 8 is going down the board, and vice versa
            (colour_to_move==white) ? pop_bit(piece_bitboards[p], target + 8) : pop_bit(piece_bitboards[P], target - 8);
            
            if (colour_to_move==white)
            {
                pop_bit(piece_bitboards[p], target + 8);
                position_hashkey ^= piece_hashkey[p][target + 8];
            }
            else
            {
                pop_bit(piece_bitboards[P], target - 8);
                position_hashkey ^= piece_hashkey[P][target- 8];
            }
        }

        if (enpassant!=null_sq)
        {
            position_hashkey ^= enpassant_hashkey[enpassant];
        }
        enpassant = null_sq;

        // set enpassant square when pawn double moves
        if (double_pawn_move)
        {
            // (colour_to_move==white) ? enpassant = target + 8 : enpassant = target - 8;

            if (colour_to_move == white)
            {   
                enpassant = target + 8;
                position_hashkey ^= enpassant_hashkey[target+8];
            }
            else
            {
                enpassant = target - 8;
                position_hashkey ^= enpassant_hashkey[target-8];
            }
        }

        // handle castling
        if (castling)
        {
            if (target == g1)
            {
                pop_bit(piece_bitboards[R], h1);
                set_bit(piece_bitboards[R], f1);

                position_hashkey ^= piece_hashkey[R][h1];  // remove rook from h1 from hash key
                position_hashkey ^= piece_hashkey[R][f1];  // put rook on f1 into a hash key
            }
            else if (target == c1)
            {
                pop_bit(piece_bitboards[R], a1);
                set_bit(piece_bitboards[R], d1);

                position_hashkey ^= piece_hashkey[R][a1];  // remove rook from a1 from hash key
                position_hashkey ^= piece_hashkey[R][d1];  // put rook on d1 into a hash key
            }
            else if (target == g8)
            {
                pop_bit(piece_bitboards[r], h8);
                set_bit(piece_bitboards[r], f8);

                position_hashkey ^= piece_hashkey[r][h8];  // remove rook from h8 from hash key
                position_hashkey ^= piece_hashkey[r][f8];  // put rook on f8 into a hash key
            }
            else if (target == c8)
            {
                pop_bit(piece_bitboards[r], a8);
                set_bit(piece_bitboards[r], d8);

                position_hashkey ^= piece_hashkey[r][a8];  // remove rook from a8 from hash key
                position_hashkey ^= piece_hashkey[r][d8];  // put rook on d8 into a hash key
            }         
        }
        position_hashkey ^= castling_hashkey[castle_rights]; // remove castling right hash

        // updating castling rights after every move
        castle_rights &= update_castling_right_values[source];
        castle_rights &= update_castling_right_values[target];

        position_hashkey ^= castling_hashkey[castle_rights]; // update castling right hash

        // update colour occupancies
        occupancies[white] = get_white_occupancy();
        occupancies[black] = get_black_occupancy();
        occupancies[both] = get_both_occupancy();

        // change sides
        colour_to_move ^= 1;

        position_hashkey ^= colour_to_move_hashkey;
        
        // uint64_t curr_hash = gen_hashkey(); // new hashkey after move made
        // if (curr_hash != position_hashkey)
        // {
        //     cout<<"make_move()"<<"\n";
        //     cout<<"move: ";
        //     print_move(move);
        //     cout<<"\n";
        //     print_board(colour_to_move);
        //     cout<<"correct hashkey: "<<std::hex<<curr_hash<<"\n";
        //     cin.get();
        // }

        // handle illegal moves. if move causes king to check, restore previous position and return illegal move
        if (is_square_under_attack((colour_to_move==white) ? get_lsb_index(piece_bitboards[k]) : get_lsb_index(piece_bitboards[K]), colour_to_move))
        {
            restoreBoard();
            return 0;
        }
        else 
            return 1;
    }
    
    else if (move_type == only_captures)
    {
        if (get_is_capture_move(move))
            make_move(move, all_moves);
        else    
            return 0;
    }
}
