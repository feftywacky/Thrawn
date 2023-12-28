# Thrawn
IN DEVELOPMENT <br> <br>
Thrawn is UCI compliant chess engine (it does NOT have its own GUI) <br>
Requires a GUI engine such as CuteChess, Arena GUI, Scid vs PC etc.

## Features
- ~1900 ELO
- Various search optimizations
- Bitboard data structure
- Transposition tables
- PeSTO's evaluation + bonuses/penalties for piece mobility and safety
- Implements UCI protocol
- Time control

## Updates
- hash move ordering
- razoring pruning
- detecting threefold repetition
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

## TODO
- nnue
- opening book table

## REFERENCES
- https://www.chessprogramming.org/Main_Page
- https://web.archive.org/web/20071006042845/http://www.brucemo.com/compchess/programming/index.htm
- https://github.com/bluefeversoft/vice
- https://github.com/official-stockfish/Stockfish

