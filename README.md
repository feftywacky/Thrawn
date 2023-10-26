# bitboard-nnue-chess-engine
chess engine in c++ that uses a bitboard data structure and a nnue for evaluation

to run on cmd:
g++ -std=c++17 -c main.cpp engine.cpp bitboard.cpp bitboard_functions.cpp constants.cpp
g++ -o chess main.o engine.o bitboard.o bitboard_functions.o constants.o
chess

couple important notes:
print_bitboard() prints with black on bottom
pprint_bb() prints with white on bottom
