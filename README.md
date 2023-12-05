# bitboard-nnue-chess-engine
ONGOING/INCOMPLETE <br>
chess engine in c++ that uses a bitboard data structure and a nnue for evaluation

## Updates
- passes perft test! <3

## Development Environment
### Compiler:
```bash
g++ --version
```
> g++ (x86_64-posix-seh-rev0, Built by MinGW-W64 project) 7.3.0

### Build and Run:
standard compilation commands:
```bash
g++ -std=c++17 -c main.cpp engine.cpp bitboard.cpp bitboard_helpers.cpp constants.cpp fen.cpp move_helpers.cpp perft.cpp
g++ -o chess main.o engine.o bitboard.o bitboard_helpers.o constants.o fen.o move_helpers.o perft.o
chess.exe
```

RECOMMENDED <br>
run with compiler opimtizations (~8x faster):
```bash
g++ -std=c++17 -Ofast -c main.cpp engine.cpp bitboard.cpp bitboard_helpers.cpp constants.cpp fen.cpp move_helpers.cpp perft.cpp
g++ -o chess main.o engine.o bitboard.o bitboard_helpers.o constants.o fen.o move_helpers.o perft.o
chess.exe
``` 

## Features
- Implements UCI protocol
- Uses bitboard data structure for fast move generation
- Parses any fen notation to setup board

## TODO
- static evaluation function
- minimax/negamax with alpha beta pruning
- quiescience search
- move ordering
- pass perft test
- add inline keyword in header files

## BUGS/POTENTIAL ISSUES
- none for now 
