# bitboard-nnue-chess-engine
chess engine in c++ that uses a bitboard data structure and a nnue for evaluation

compiler:<br>
> g++ (x86_64-posix-seh-rev0, Built by MinGW-W64 project) 7.3.0

to run:<br>
```bash
g++ -std=c++17 -c main.cpp engine.cpp bitboard.cpp bitboard_functions.cpp constants.cpp
g++ -o chess main.o engine.o bitboard.o bitboard_functions.o constants.o
./chess
```

couple important notes:<br>
print_bitboard() prints with black on bottom<br>
pprint_bb() prints with white on bottom<br>
