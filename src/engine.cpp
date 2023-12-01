#include "engine.hpp"
#include "bitboard.hpp"
#include "constants.hpp"
#include "bitboard_functions.hpp"
#include "move_helpers.hpp"
#include <iostream>
#include <vector>
#include <string>
#include <map>
#include <set>
#include <bitset>

using namespace std;

// contructor
Engine::Engine() : board()
{
    moves = {};
}

void Engine::generate_moves()
{
    // clear previous moves
    moves.clear();

    // the squares where the pieces started from, and where it will go
    int source;
    int target;

    // bitboard of the current piece, and its attacks
    uint64_t curr; 
    uint64_t attacks;

    // loop over all piece types for both black and white
    for (int piece = P; piece<=k; piece++)
    {
        uint64_t curr = board.piece_bitboards[piece];

        // to distinguish betweem white and black specific moves
        // includes pawns and castling
        if (board.colour_to_move == white)
        {
            // generate pawn moves
            if (piece==P)
            {
                while(curr) // while white pawns are present on the board
                {
                    source = get_lsb_index(curr);
                    target = source - 8; // go up one square

                    if (target>=a8 && !get_bit(board.occupancies[both], target))
                    {
                        // pawn promotion by going up one square (NOT TAKING A PIECE)
                        if (source>=a7 && source<=h7)
                        {
                            moves.push_back(parse_move(source, target, piece, Q, 0, 0, 0, 0));
                            moves.push_back(parse_move(source, target, piece, R, 0, 0, 0, 0));
                            moves.push_back(parse_move(source, target, piece, N, 0, 0, 0, 0));
                            moves.push_back(parse_move(source, target, piece, B, 0, 0, 0, 0));
                        }

                        // one square and two square pawn moves
                        else
                        {
                            // one square
                            moves.push_back(parse_move(source, target, piece, 0, 0, 0, 0, 0));

                            // two square
                            if (source>=a2 && source<=h2 && !get_bit(board.occupancies[both], target-8))
                            {
                                moves.push_back(parse_move(source, target-8, piece, 0, 1, 0, 0, 0));
                            }
                        }
                    }

                    attacks = board.pawn_attacks[board.colour_to_move][source]  & board.occupancies[black];

                    while (attacks) // while attacks squares are present on the board
                    {   
                        target = get_lsb_index(attacks);

                        if (source>=a7 && source<=h7) // pawn promotions by capturing a piece
                        {
                            moves.push_back(parse_move(source, target, piece, Q, 1, 0, 0, 0));
                            moves.push_back(parse_move(source, target, piece, R, 1, 0, 0, 0));
                            moves.push_back(parse_move(source, target, piece, N, 1, 0, 0, 0));
                            moves.push_back(parse_move(source, target, piece, B, 1, 0, 0, 0));
                        }

                        // diagonal pawn capture
                        else
                        {
                            moves.push_back(parse_move(source, target, piece, 0, 1, 0, 0, 0));
                        }

                        pop_bit(attacks, target);
                    }

                    // enpassant
                    if (board.enpassant!=null_sq)
                    {
                        uint64_t enpassant_attacks = board.pawn_attacks[board.colour_to_move][source] & (1ULL << board.enpassant);
                        if (enpassant_attacks)
                        {
                            int enpassant_target = get_lsb_index(enpassant_attacks);
                            moves.push_back(parse_move(source, enpassant_target, piece, 0, 1, 0, 1, 0));
                        }
                    }

                    // remove ls1b for looping through all the bits
                    pop_bit(curr, source);
                } 
            }

            // castling
            if (piece == K)
            {
                if (board.castle_rights & wks)
                {
                    if (!get_bit(board.occupancies[both], f1) && !get_bit(board.occupancies[both], g1))
                    {
                        // make sure can't castle through check
                        if (!board.is_square_under_attack(f1, black) && !board.is_square_under_attack(g1, black))
                            moves.push_back(parse_move(e1, g1, piece, 0, 0, 0, 0, 1));
                    }
                }
                if (board.castle_rights & wqs)
                {
                    if (!get_bit(board.occupancies[both], b1) && !get_bit(board.occupancies[both], c1) && !get_bit(board.occupancies[both], d1))
                    {
                        // make sure can't castle through check
                        if (!board.is_square_under_attack(b1, black) && !board.is_square_under_attack(c1, black) && !board.is_square_under_attack(d1, black))
                            moves.push_back(parse_move(e1, c1, piece, 0, 0, 0, 0, 1));
                    }
                }
            }
        }

        // for black pieces
        else
        {
            if (piece==p)
            {
                while(curr) // while white pawns are present on the board
                {
                    source = get_lsb_index(curr);
                    target = source + 8; // go down one square

                    if (target<=h1 && !get_bit(board.occupancies[both], target))
                    {
                        // pawn promotion by going down one square (NOT TAKING A PIECE)
                        if (source>=a2 && source<=h2)
                        {
                            moves.push_back(parse_move(source, target, piece, Q, 0, 0, 0, 0));
                            moves.push_back(parse_move(source, target, piece, R, 0, 0, 0, 0));
                            moves.push_back(parse_move(source, target, piece, N, 0, 0, 0, 0));
                            moves.push_back(parse_move(source, target, piece, B, 0, 0, 0, 0));
                        }

                        // one square and two square pawn moves
                        else
                        {
                            // one square
                            moves.push_back(parse_move(source, target, piece, 0, 0, 0, 0, 0));

                            // two square
                            if (source>=a7 && source<=h7 && !get_bit(board.occupancies[both], target+8))
                                moves.push_back(parse_move(source, target+8, piece, 0, 0, 0, 0, 0));
                        }
                    }

                    attacks = board.pawn_attacks[board.colour_to_move][source] & board.occupancies[white];

                    while (attacks) // while attacks squares are present on the board
                    {   
                        target = get_lsb_index(attacks);

                        if (source>=a2 && source<=h2) // pawn promotion by capturing piece
                        {
                            moves.push_back(parse_move(source, target, piece, Q, 1, 0, 0, 0));
                            moves.push_back(parse_move(source, target, piece, R, 1, 0, 0, 0));
                            moves.push_back(parse_move(source, target, piece, N, 1, 0, 0, 0));
                            moves.push_back(parse_move(source, target, piece, B, 1, 0, 0, 0));
                        }

                        // diagonal pawn capture
                        else
                        {
                            moves.push_back(parse_move(source, target, piece, 0, 1, 0, 0, 0));
                        }

                        pop_bit(attacks, target);
                    }

                    // enpassant
                    if (board.enpassant!=null_sq)
                    {
                        uint64_t enpassant_attacks = board.pawn_attacks[board.colour_to_move][source] & (1ULL << board.enpassant);
                        if (enpassant_attacks)
                        {
                            int enpassant_target = get_lsb_index(enpassant_attacks);
                            moves.push_back(parse_move(source, enpassant_target, piece, 0, 1, 0, 1, 0));
                        }
                    }

                    // remove ls1b for looping through all the bits
                    pop_bit(curr, source);
                }
            }

            // castling
            if (piece == k)
            {
                if (board.castle_rights & bks)
                {
                    if (!get_bit(board.occupancies[both], f8) && !get_bit(board.occupancies[both], g8))
                    {
                        if (!board.is_square_under_attack(f8, white) && !board.is_square_under_attack(g8, white))
                            moves.push_back(parse_move(e8, g8, piece, 0, 0, 0, 0, 1));
                    }
                }
                if (board.castle_rights & bqs)
                {
                    if (!get_bit(board.occupancies[both], b8) && !get_bit(board.occupancies[both], c8) && !get_bit(board.occupancies[both], d8))
                    {
                        if (!board.is_square_under_attack(b8, white) && !board.is_square_under_attack(c8, white) && !board.is_square_under_attack(d8, white))
                            moves.push_back(parse_move(e8, c8, piece, 0, 0, 0, 0, 1));
                    }
                }
            }
        }

        
        // generate for the rest of the pieces that are not colour specific
        
        // knight
        if ( (board.colour_to_move == white) ? piece == N : piece == n )
        {

            while (curr)
            {
                source = get_lsb_index(curr);

                attacks = board.knight_attacks[source] & ( (board.colour_to_move==white) ? ~board.occupancies[white] : ~board.occupancies[black]);

                while (attacks)
                {
                    target = get_lsb_index(attacks);
                    
                    // non-capture move
                    if ( !get_bit( (board.colour_to_move==white) ? board.occupancies[black] : board.occupancies[white], target ) )
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

        // bishop
        if ( (board.colour_to_move == white) ? piece == B : piece == b )
        {
            while(curr)
            {
                source = get_lsb_index(curr);

                attacks = board.get_bishop_attacks(source, board.occupancies[both]) & ( (board.colour_to_move==white) ? ~board.occupancies[white] : ~board.occupancies[black]);
                while (attacks)
                {
                    
                    target = get_lsb_index(attacks);
                    
                    // non-capture move
                    if ( !get_bit( (board.colour_to_move==white) ? board.occupancies[black] : board.occupancies[white], target ) )
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
        
        // rook
        if ( (board.colour_to_move == white) ? piece == R : piece == r )
        {
            while(curr)
            {
                source = get_lsb_index(curr);

                attacks = board.get_rook_attacks(source, board.occupancies[both]) & ( (board.colour_to_move==white) ? ~board.occupancies[white] : ~board.occupancies[black]);

                while (attacks)
                {
                    target = get_lsb_index(attacks);
                    
                    // non-capture move
                    if ( !get_bit( (board.colour_to_move==white) ? board.occupancies[black] : board.occupancies[white], target ) )
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

        // queen
        if ( (board.colour_to_move == white) ? piece == Q : piece == q )
        {
            while (curr)
            {
                source = get_lsb_index(curr);

                attacks = board.get_queen_attacks(source, board.occupancies[both]) & ( (board.colour_to_move==white) ? ~board.occupancies[white] : ~board.occupancies[black]);

                while (attacks)
                {
                    target = get_lsb_index(attacks);
                    
                    // non-capture move
                    if ( !get_bit( (board.colour_to_move==white) ? board.occupancies[black] : board.occupancies[white], target ) )
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

        // king
        if ( (board.colour_to_move == white) ? piece == K : piece == k )
        {
            while (curr)
            {
                source = get_lsb_index(curr);

                attacks = board.king_attacks[source] & ( (board.colour_to_move==white) ? ~board.occupancies[white] : ~board.occupancies[black]);

                while (attacks)
                {
                    target = get_lsb_index(attacks);
                    
                    // non-capture move
                    if ( !get_bit( (board.colour_to_move==white) ? board.occupancies[black] : board.occupancies[white], target ) )
                    {
                        moves.push_back(parse_move(source, target, piece, 0, 1, 0, 0, 0));
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

        
    } // end of looping through all pieces


}

int Engine::make_move(int move, int move_type)
{
    if (move_type == all_moves)
    {
        board.copyBoard();

        // move parsing
        int source = get_move_source(move);
        int target = get_move_target(move);
        int piece = get_move_piece(move);
        int promoted_piece = get_promoted_piece(move);
        int is_capture_move = get_is_capture_move(move);
        int double_pawn_move = get_is_double_pawn_move(move);
        int enpassant = get_is_move_enpassant(move);
        int castling = get_is_move_castling(move);

        // move piece
        pop_bit(board.piece_bitboards[piece], source);
        set_bit(board.piece_bitboards[piece], target);

        // if capture move, remove the piece being captured from its corresponding bitboard
        // ie. if white pawn captures black kngiht, remove black knight from black knight bitboard
        if (is_capture_move)
        {
            int start_piece, end_piece;
            
            (board.colour_to_move==white) ? start_piece = p : start_piece = P;
            (board.colour_to_move==white) ? end_piece = k : end_piece = K;

            for(int i=start_piece; i<=end_piece;i++)
            {
                if (get_bit(board.piece_bitboards[i], target))
                {
                    pop_bit(board.piece_bitboards[i], target);
                    break;
                }
            }
        }

        // handle pawn promotions
        if (promoted_piece)
        {
            // erase the pawn from the target square
            pop_bit(board.piece_bitboards[(board.colour_to_move == white) ? P : p], target);
            
            set_bit(board.piece_bitboards[promoted_piece], target);
        }

        // handle enpassant capture
        if (enpassant)
        {
            // target + 8 is going down the board, and vice versa
            (board.colour_to_move) ? pop_bit(board.piece_bitboards[p], target + 8) : pop_bit(board.piece_bitboards[P], target - 8);
        }
        board.enpassant = null_sq;

        // set enpassant square when pawn double moves
        if (double_pawn_move)
        {
            (board.colour_to_move) ? board.enpassant = target + 8 : board.enpassant = target + 8;
        }

        // handle castling
        if (castling)
        {
            if (target == g1)
            {
                pop_bit(board.piece_bitboards[R], h1);
                set_bit(board.piece_bitboards[R], f1);
            }
            else if (target == c1)
            {
                pop_bit(board.piece_bitboards[R], a1);
                set_bit(board.piece_bitboards[R], d1);
            }
            else if (target == g8)
            {
                pop_bit(board.piece_bitboards[r], h8);
                set_bit(board.piece_bitboards[r], f8);
            }
            else if (target == c8)
            {
                pop_bit(board.piece_bitboards[r], a8);
                set_bit(board.piece_bitboards[r], d8);
            }         
        }

        // updating castling rights after every move
        board.castle_rights &= update_castling_right_values[source];
        board.castle_rights &= update_castling_right_values[target];

        // update colour occupancies
        board.occupancies[white] = board.get_white_occupancy();
        board.occupancies[black] = board.get_black_occupancy();
        board.occupancies[both] = board.get_both_occupancy();

        // change sides
        board.colour_to_move ^= 1;

        // handle illegal moves. if move causes king to check, restore previous position and return illegal move
        if (board.is_square_under_attack((board.colour_to_move==white) ? get_lsb_index(board.piece_bitboards[k]) : get_lsb_index(board.piece_bitboards[K]), board.colour_to_move))
        {
            board.restoreBoard();
            return 0;

        }
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

