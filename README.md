# bitboard-nnue-chess-engine
chess engine in c++ that uses a bitboard data structure and a nnue for evaluation

to run on cmd:<br>
g++ -std=c++17 -c main.cpp engine.cpp bitboard.cpp bitboard_functions.cpp constants.cpp<br>
g++ -o chess main.o engine.o bitboard.o bitboard_functions.o constants.o
chess<br>

couple important notes:<br>
print_bitboard() prints with black on bottom<br>
pprint_bb() prints with white on bottom<br>
