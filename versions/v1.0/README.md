Thrawn v1.0 RELEASE
~1900 ELO

new changes
- improved evaluation with game phases and mobilty/safety scores
- dynamic memory allocation for transposition table

old updates
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
