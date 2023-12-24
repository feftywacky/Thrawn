# ThrawnEngine
IN DEVELOPMENT <br>
ThrawnEngine is UCI Compliant (does NOT contain its own GUI)

chess engine in c++ that uses a bitboard data structure and a piece square tables evaluation (nnue in the future)

## Features
- ~1900 ELO
- Time control
- Various search optimizations
- Uses bitboard data structure for fast move generation
- Piece-square tables for evaluation
- Implements UCI protocol

## Updates
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
- improve evaluation function
- tansposition tables/zobrist hashing, implement 3 move repitition
- nnue

## BUGS/POTENTIAL ISSUES
- none for now 
