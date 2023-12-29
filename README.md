# Thrawn
IN DEVELOPMENT <br> <br>
Thrawn is UCI compliant chess engine (it does NOT have its own GUI) <br>
Requires a GUI engine such as CuteChess, Arena GUI, Scid vs PC etc.

## TODO
- [] general code clean up
- [] transposition table bug?
- [] hash move ordering giving worst results bug
- [] fixed zero evaluation bug
- [] nnue incremental update
- [] opening book table

## Features
- ~2300+ ELO
- NNUE for evaluation
- Various search optimizations
- Bitboard data structure
- Transposition tables
- Time allocation
- Implements UCI protocol

## Updates
- fifty-move rule + 3-fold repetition detection
- NNUE evaluation
- hash move ordering
- razoring pruning
- transposition table / zobrist hashing
- null move pruning
- late move reduction
- iterative deepening (also adjusting aspiration window)
- principal variation search
- move history (quiet and killer)
- quiescence search
- negamax alpha beta search
- piece table evaluation
- uci compliant
- passes perft test! <3

## Compiling ThrawnEngine
currently only supports windows machines with 64-bit CPUs

### Compiler:
```bash
g++ --version
```
> g++ (x86_64-posix-seh-rev0, Built by MinGW-W64 project) 7.3.0

### Build and Run:
for gcc: 
```bash
cd src
g++ -std=c++17 -Ofast -flto -o thrawn *.cpp
thrawn.exe
``` 

## REFERENCES
# General
- https://www.chessprogramming.org/Main_Page
- https://github.com/bluefeversoft/vice
- https://github.com/official-stockfish/Stockfish
# Search Algorithms
- https://web.archive.org/web/20071006042845/http://www.brucemo.com/compchess/programming/index.htm
# NNUE
- https://hxim.github.io/Stockfish-Evaluation-Guide/
nnue probe library
- https://github.com/dshawul/nnue-probe
nnue dataset
- https://tests.stockfishchess.org/nns

