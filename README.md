# Atheon

A chess engine built from scratch in C++17. Named after [Atheon, Time's Conflux](https://www.destinypedia.com/Atheon,_Time%27s_Conflux) from the Destiny franchise — the Vex timeline simulation mirrors a chess search tree.

Atheon implements the [UCI protocol](https://www.chessprogramming.org/UCI) and plays legal, reasonably strong chess against any UCI-compatible GUI (CuteChess, Arena, Lichess Bot API, etc.).

## Features

### Board Representation
- **Bitboard-based** position representation using 12 bitboards (one per piece-colour)
- **Mailbox** redundancy array for O(1) piece-on-square lookups
- **Zobrist hashing** for fast position identification and transposition table keying
- Full FEN parsing and generation

### Move Generation
- Precomputed **knight, king, and pawn attack tables**
- **Magic bitboards** for sliding piece (bishop, rook, queen) attack generation — magic numbers are computed at startup using a seeded PRNG
- Handles all special moves: castling, en passant, pawn double push, and all four promotion types
- Legality filtering via make/unmake with king-in-check verification
- Perft-validated against standard test positions (starting position, Kiwipete, etc.)

### Search
- **Negamax** with **alpha-beta pruning**
- **Iterative deepening** with **aspiration windows**
- **Principal Variation Search (PVS)** — zero-window search for non-PV moves
- **Quiescence search** to resolve tactical sequences at leaf nodes
- **Transposition table** (Zobrist-keyed, configurable size, default 64 MB)
- **Null move pruning** (R=3, with non-pawn material guard)
- **Late Move Reductions (LMR)** for quiet moves searched late in the move list
- **Check extensions** — depth extended by 1 when the side to move is in check
- **Move ordering**: TT move → MVV-LVA captures → killer moves (2 per ply) → history heuristic
- Time-controlled search with periodic check (every 2048 nodes)

### Evaluation
- **Tapered evaluation** blending middlegame and endgame scores by game phase (24-point scale)
- **PeSTO piece-square tables** for all six piece types, separate MG/EG tables
- **Piece mobility** — pseudo-legal move count bonuses via lookup tables (knights, bishops, rooks, queens)
- **Pawn structure**: doubled pawn penalty, isolated pawn penalty
- **Passed pawns**: rank-scaled MG/EG bonuses, blocked passer penalty, free passer bonus, pawn support bonus, rook-behind-passer bonus
- **Knight outposts** — bonus for knights on ranks 4–6, supported by own pawn, no enemy pawn can attack
- **Bishop pair bonus** (MG and EG)
- **Rook on open/semi-open files**
- **King safety**: pawn shield (near/far/missing), open file penalties near king, attacker count with nonlinear scaling via a safety table
- **Tempo bonus** for the side to move

### UCI Support
- Full `position` command (startpos, FEN, with move sequences)
- `go depth N`, `go movetime N`, `go wtime/btime/winc/binc/movestogo`
- Time management: move-time allocation based on remaining time, increment, and estimated moves remaining
- `info` output with depth, score, nodes, NPS, time, and principal variation
- `ucinewgame` clears the transposition table

## Building

### Requirements
- C++17 compiler (MSVC, GCC, or Clang)
- CMake 3.14+

### Build (Windows / MSVC)
```powershell
mkdir build
cd build
cmake ..
cmake --build . --config Release
```
The executable will be at `build\Release\chess_engine.exe`.

### Build (Linux / macOS)
```bash
mkdir build && cd build
cmake ..
cmake --build . --config Release
```

## Usage

### With a Chess GUI
1. Open CuteChess, Arena, or another UCI-compatible GUI
2. Add Atheon as a new engine, pointing to the executable
3. Play a game or run an engine-vs-engine match

### From the Command Line
Atheon communicates via stdin/stdout using the UCI protocol:
```
uci
> id name Atheon
> id author Conan Trindade Steel
> uciok

isready
> readyok

position startpos
go depth 10
> info depth 1 score cp 30 nodes 21 nps 21000 time 1 pv e2e4
> ...
> info depth 10 score cp 35 nodes 284919 nps 1424595 time 200 pv e2e4 e7e5 g1f3 ...
> bestmove e2e4
```

## Project Structure

```
src/
  main.cpp         Entry point — initialises tables, runs UCI loop
  types.h          Core types: Bitboard, Move, Color, PieceType, Square, Piece
  bitboard.h/.cpp  Bitboard utilities: popcount, lsb, set/clear/test bit, print
  board.h/.cpp     Position class: FEN, make/unmake move, Zobrist hash updates
  attacks.h/.cpp   Attack tables: knights, kings, pawns, magic bitboards for sliders
  movegen.h/.cpp   Legal move and capture generation
  search.h/.cpp    Negamax, quiescence, iterative deepening, PVS, LMR, null move
  eval.h/.cpp      Tapered evaluation with all positional features
  tt.h/.cpp        Transposition table (hash map with replacement)
  zobrist.h/.cpp   Zobrist key initialisation and constants
  uci.h/.cpp       UCI protocol handler and time management
  perft.h/.cpp     Perft testing for move generation validation
```

## Architecture Decisions

- **Magic bitboards over classical/Kogge-Stone**: magic numbers are computed fresh at startup with a fixed seed for reproducibility, rather than using hardcoded values. This adds a few milliseconds to startup but keeps the code self-contained.
- **Make/unmake over copy-make**: the position is mutated in place and restored via `UndoInfo`, which is faster but requires careful bookkeeping. Every `make_move` has a matching `unmake_move`.
- **PeSTO tables as the PST baseline**: widely used and well-tuned values from the PeSTO project, providing a strong positional foundation without manual tuning.
- **Handcrafted evaluation only**: NNUE was explored during a related Python competition engine project but not integrated into Atheon. The classical eval covers material, piece-square tables, mobility, pawn structure, passed pawns, king safety, and several positional bonuses.

## Strength

The engine has not been formally rated against a calibrated pool, but based on its feature set (alpha-beta + PVS + LMR + null move + TT + handcrafted eval with king safety and mobility), estimated strength is in the range of **1800–2200 Elo** on CCRL-equivalent scales. This is comparable to engines like TSCP, Vice, and other educational engines with similar search and evaluation features.

## Licence

GPLv3

## Author

Conan Trindade Steel
