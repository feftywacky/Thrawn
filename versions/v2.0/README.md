Thrawn v2.0 + NNUE 
~2500+ ELO

IMPORTANT NOTE: nnue file must be placed in the same directory as the executable or engine will play random moves

new changes
- fixed zero evaluation blundering bug
- embedded NNUE
- no-hashmove reduction
- reverse futility pruning / static null move pruning
- late move pruning

old updates
- razoring
- hash move ordering
- improved evaluation with game phases and mobilty/safety scores
- dynamic memory allocation for transposition table
- improved search speed from fixing various bugs
- detecting 3-fold repetition
- zobrist hashing
- transposition tables
- added uci commands for timing control (search no longer fixed depth)
- null move pruning
- late move reduction
- iterative deepening (also adjusting aspiration window)
- principal variation search
- move history (quiet and killer)
- basic move ordering
- quiescence search
- negamax alpha beta search
- checkmate and stalemate
- basic static eval using piece square tables
