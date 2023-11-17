# bitboard-nnue-chess-engine
chess engine in c++ that uses a bitboard data structure and a nnue for evaluation

## Development Environment
### compiler:
```bash
g++ (x86_64-posix-seh-rev0, Built by MinGW-W64 project) 7.3.0
```

### Build and Run:
```bash
g++ -std=c++17 -c main.cpp engine.cpp bitboard.cpp bitboard_functions.cpp constants.cpp
g++ -o chess main.o engine.o bitboard.o bitboard_functions.o constants.o
chess
```

## Features
- Implements UCI protocol
- Uses bitboard data structure for fast move generation

## TODO
- fen parsing
- queen moves
- move generation
