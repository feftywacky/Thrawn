# Thrawn
IN DEVELOPMENT <br> <br>
Thrawn is UCI compliant chess engine that equires a GUI application such as CuteChess, Arena GUI, Scid vs PC etc.


## TODO
- [ ] LOTS of general code clean up
- [x] transposition table bug?
- [x] hash move ordering giving worst results bug
- [x] fixed zero evaluation bug
- [ ] nnue incremental update
- [ ] opening book table

## Features
- ~2500+ ELO
- NNUE for evaluation
- Various search optimizations
- Bitboard data structure
- Transposition tables
- Time allocation
- Implements UCI protocol

## Updates
- no-hashmove reduction
- reverse futility pruning / static null move pruning
- late move pruning
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

for Thrawn v2.0:
```bash
cd src
g++ -std=c++17 -Ofast -flto -DUSE_AVX2 -mavx2 -DUSE_SSE41 -msse4.1 -DUSE_SSE3 -msse3 -DUSE_SSE2 -msse2 -DUSE_SSE -msse -o Thrawn *.cpp
Thrawn.exe
``` 

for Thrawn v1.1 and older:
```bash
cd src
g++ -std=c++17 -Ofast -flto -o Thrawn *.cpp
Thrawn.exe
``` 

## REFERENCES
### General
- https://www.chessprogramming.org/Main_Page
- https://github.com/bluefeversoft/vice
- https://github.com/official-stockfish/Stockfish
- https://github.com/mkd/gargantua 
### Search Algorithms
- https://web.archive.org/web/20071006042845/http://www.brucemo.com/compchess/programming/index.htm
### NNUE
- https://hxim.github.io/Stockfish-Evaluation-Guide/ <br>
- nnue probe framework: https://github.com/dshawul/nnue-probe
- nnue dataset https://tests.stockfishchess.org/nns

