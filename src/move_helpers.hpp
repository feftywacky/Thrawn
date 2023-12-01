#ifndef MOVE_HELPERS_HPP
#define MOVE_HELPERS_HPP

#include <vector>
  
int parse_move(int source, int target, int piece, int promoted_piece, int capture, int double_pawn_move, int enpassant, int castling);
int get_move_source(int move);
int get_move_target(int move);
int get_move_piece(int move);
int get_promoted_piece(int move);
int get_is_capture_move(int move);
int get_is_double_pawn_move(int move);
int get_is_move_enpassant(int move);
int get_is_move_castling(int move);

void print_move(const int& move);
void print_move_list(const std::vector<int>& moves);


#endif