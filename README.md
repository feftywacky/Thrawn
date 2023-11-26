# bitboard-nnue-chess-engine
chess engine in c++ that uses a bitboard data structure and a nnue for evaluation

## Development Environment
### Compiler:
```bash
g++ --version
```
> g++ (x86_64-posix-seh-rev0, Built by MinGW-W64 project) 7.3.0

### Build and Run:
```bash
g++ -std=c++17 -c main.cpp engine.cpp bitboard.cpp bitboard_functions.cpp constants.cpp fen.cpp move_helpers.cpp
g++ -o chess main.o engine.o bitboard.o bitboard_functions.o constants.o fen.o move_helpers.o 
chess.exe
```

## Features
- Implements UCI protocol
- Uses bitboard data structure for fast move generation

## TODO
- move generation (seperate into methods)
- pass perft test
- add inline keyword in header files
- allow user input to make move

## BUGS/POTENTIAL ISSUES
- is_square_under_attack() in bitboards.cpp also checks squares attacked by own colour, but might not be an issue because this method is only used to check if king is under check?
