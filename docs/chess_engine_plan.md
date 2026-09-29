# Chess Engine Development Plan

## Status Tracker

| Phase | Description | Status |
|-------|-------------|--------|
| 0 | C++ Foundation & Project Setup | Complete |
| 1 | Board Representation | Complete |
| 2 | Move Generation | Complete |
| 3 | UCI Protocol (Minimal) | Complete |
| 4 | Search (v1) | Complete |
| 5 | Evaluation (v1) | Complete |
| 6 | First Playable Engine | Complete |
| 7 | Search Improvements | Complete |
| 8 | Evaluation Improvements | Complete |
| 9 | Time Management & UCI Completion | Complete |
| 10 | Testing, Profiling & Strength Measurement | Not Started |
| 11 | NNUE Integration | Not Started |

---

## Phase 0 — C++ Foundation & Project Setup

**Goal:** Set up the repo structure, build system, and make sure you can compile and run a "hello world" with tests.

**Why this matters:** Getting the toolchain working before writing chess code removes a whole class of frustration. CMake, compiler flags, and test frameworks are one-time setup costs that pay off immediately.

### Deliverables
- Project directory structure (see below)
- CMakeLists.txt that compiles and runs
- A test framework (Catch2 recommended — header-only, minimal setup) with one passing test
- A Makefile or build script shortcut for convenience

### Suggested Directory Structure
```
chess-engine/
├── CMakeLists.txt
├── src/
│   ├── main.cpp            # Entry point, UCI loop
│   ├── types.h              # Core types, enums (Piece, Color, Square, etc.)
│   ├── bitboard.h / .cpp    # Bitboard utilities
│   ├── board.h / .cpp        # Position representation
│   ├── movegen.h / .cpp      # Move generation
│   ├── search.h / .cpp       # Search algorithm
│   ├── eval.h / .cpp          # Position evaluation
│   └── uci.h / .cpp           # UCI protocol handler
├── tests/
│   ├── test_main.cpp
│   ├── test_bitboard.cpp
│   ├── test_board.cpp
│   ├── test_movegen.cpp
│   └── test_perft.cpp
└── docs/
    └── chess_engine_plan.md   # This file
```

### Key C++ Concepts Introduced
- CMake basics (targets, linking, compiler flags)
- Header vs source files, include guards / `#pragma once`
- `constexpr`, `enum class`, `std::array`
- Compiling with warnings enabled (`-Wall -Wextra -Wpedantic`)

---

## Phase 1 — Board Representation

**Goal:** Represent a chess position in memory using bitboards. Be able to load a position from a FEN string and print it to the console.

**Why bitboards:** A bitboard is a 64-bit integer where each bit corresponds to a square on the board. This maps perfectly to the 64 squares of a chessboard and enables extremely fast operations using bitwise logic (AND, OR, XOR, bit shifts). Virtually all competitive engines use bitboards. The alternative — an 8×8 array — is simpler to understand but dramatically slower for move generation and evaluation.

### Deliverables
- `types.h`: Enums for `Color` (White, Black), `PieceType` (Pawn, Knight, Bishop, Rook, Queen, King), `Square` (A1=0 through H8=63), `Piece` (combining color + type)
- `bitboard.h/.cpp`: Utility functions — print a bitboard, set/clear/test bits, population count (`popcount`), least-significant-bit scan (`lsb`)
- `board.h/.cpp`: The `Position` class holding 12 bitboards (one per piece-color), side to move, castling rights, en passant square, halfmove clock, fullmove counter
- FEN parser: `Position::set(const std::string& fen)` — loads any legal position
- FEN generator: `Position::fen() const` — outputs the current position as a FEN string
- Board printer: `Position::print() const` — ASCII board to stdout for debugging
- Unit tests for FEN round-tripping (parse then generate, compare to input)

### Key Concepts
- Bitwise operations: AND (`&`), OR (`|`), XOR (`^`), NOT (`~`), shifts (`<<`, `>>`)
- Little-endian rank-file mapping (A1 = bit 0, H8 = bit 63) — this is the most common convention
- FEN format: `rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1`

### Validation
- Parse the starting position FEN → print → verify visually
- Parse several known positions (e.g., Kiwipete, various endgames) → regenerate FEN → compare strings

---

## Phase 2 — Move Generation

**Goal:** Generate all legal moves for any position. This is the most critical and bug-prone phase of the entire project.

**Why this is hard:** Chess has many special-case rules (castling, en passant, promotion, pins, checks, discovered checks). A bug here will silently corrupt everything built on top. Perft testing is the antidote.

### Sub-phases

#### 2A — Attack Tables & Non-Sliding Pieces
- Precompute knight attack tables (64 entries, one bitboard per square)
- Precompute king attack tables (same approach)
- Precompute pawn attack tables (64 entries × 2 colors)
- Generate pseudo-legal moves for knights, kings, and pawns (including double push, but not yet en passant or castling)
- The `Move` type: encode from-square, to-square, move flags (promotion, capture, en passant, castling) — a 16-bit integer is the standard compact encoding

#### 2B — Sliding Pieces (Magic Bitboards)
- Implement magic bitboards for bishops and rooks (queen = bishop + rook)
- This is the most technically complex part of the engine. Magic bitboards use a precomputed lookup table indexed by a "magic number" multiplied with the blocker pattern, enabling O(1) sliding-piece attack generation
- Precompute magic numbers or use known good ones from the Chess Programming Wiki
- Generate pseudo-legal moves for bishops, rooks, and queens

#### 2C — Special Moves & Legality
- En passant capture generation
- Castling move generation (checking all conditions: king/rook haven't moved, squares not attacked, not in check, squares not occupied)
- Promotion (4 variants: queen, rook, bishop, knight)
- Legality filtering: after generating pseudo-legal moves, filter out moves that leave the king in check. The simplest approach is make-move → check if own king is attacked → unmake-move
- `make_move()` and `unmake_move()` functions on `Position`

#### 2D — Perft Testing
- Implement `perft(depth)`: recursively count all leaf nodes at a given depth
- Validate against known perft results:
  - Starting position, depth 1–6
  - Kiwipete (`r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq -`), depth 1–5
  - Several other standard perft positions from the Chess Programming Wiki
- If counts don't match, implement `divide` (perft broken down by first move) to isolate the bug
- **Do not proceed to Phase 3 until all perft tests pass.** This is non-negotiable — search and eval built on buggy move generation will produce incomprehensible failures.

### Key Concepts
- Pseudo-legal vs legal moves
- Magic bitboards (or alternatively, classical approach / Kogge-Stone for simplicity, with magic as an upgrade)
- Make/unmake vs copy-make (tradeoff: speed vs simplicity)
- Perft as the gold-standard correctness test

---

## Phase 3 — UCI Protocol (Minimal)

**Goal:** Implement enough of the UCI protocol that the engine can communicate with a GUI (Arena, CuteChess). It doesn't need to play well yet — it just needs to respond correctly.

**What UCI looks like in practice:**
```
GUI → Engine: uci
Engine → GUI: id name YourEngine
                id author YourName
                uciok

GUI → Engine: isready
Engine → GUI: readyok

GUI → Engine: position startpos moves e2e4 e7e5
GUI → Engine: go depth 4
Engine → GUI: bestmove d2d4
```

### Deliverables
- UCI input loop in `main.cpp` (read stdin line by line, parse commands)
- Handle: `uci`, `isready`, `ucinewgame`, `position` (startpos and FEN, with moves), `go` (at minimum `go depth N`), `quit`
- Output: `id`, `uciok`, `readyok`, `bestmove`
- For now, `go` can return a random legal move — the real search comes in Phase 4
- Test by connecting to CuteChess or Arena and playing a game (it will play randomly, but it should not crash or hang)

### Key Concepts
- Stdin/stdout communication (no GUI code — the engine is a console program)
- Long algebraic notation for moves (e2e4, e1g1 for kingside castling, e7e8q for promotion)

---

## Phase 4 — Search (v1)

**Goal:** Implement a basic but correct game-tree search so the engine can find good moves, not just random ones.

### Deliverables
- **Negamax:** The standard recursive search framework (equivalent to minimax but cleaner in code). At each node, generate moves, recurse, negate the returned score, track the best
- **Alpha-beta pruning:** The key optimisation that makes deep search feasible. It prunes branches that cannot possibly affect the final decision. This typically doubles the effective search depth for free
- **Quiescence search:** At leaf nodes, don't just evaluate — continue searching captures and checks until the position is "quiet." Without this, the engine will have catastrophic tactical blindness (the "horizon effect")
- **Iterative deepening:** Instead of searching directly to depth N, search depth 1, then depth 2, then depth 3, etc. This sounds wasteful but enables time management and provides move ordering information. The overhead is minimal because tree size grows exponentially
- Wire search into UCI `go depth N` and `go movetime M`

### Validation
- Play test games against a weak engine (or yourself) — the engine should now play recognisable chess, even if weak
- Verify that increasing depth produces stronger play
- Confirm that the engine finds basic tactics (forks, pins, skewers) at sufficient depth

---

## Phase 5 — Evaluation (v1)

**Goal:** Give the engine a handcrafted evaluation function so it can distinguish good positions from bad ones beyond just material.

**Why material alone isn't enough:** An engine that only counts material won't understand pawn structure, king safety, piece activity, or positional concepts. Even a simple eval dramatically improves play.

### Deliverables
- **Material counting** with standard values (pawn=100, knight=320, bishop=330, rook=500, queen=900) — the exact values are tunable later
- **Piece-square tables (PSTs):** A 64-entry bonus/penalty table for each piece type, encoding positional knowledge. For example: knights are worth more on central squares, kings should stay on the back rank in the middlegame but centralise in the endgame. This is the single biggest eval improvement for the least code
- **Basic pawn structure:** Penalise doubled pawns and isolated pawns. Bonus for passed pawns
- **Tapered evaluation:** Blend middlegame and endgame scores based on remaining material. This handles the transition smoothly (e.g., king should centralise as material disappears)
- Tune by playing against known engines and adjusting weights

### Key Concepts
- Centipawn scale (100 cp = 1 pawn)
- Piece-square tables (the most bang-for-buck eval feature)
- Game phase detection and tapered eval

---

## Phase 6 — First Playable Engine

**Goal:** Integration milestone. The engine should now play complete games of chess via UCI, with real search and evaluation.

### Checklist
- All phases 0–5 complete and passing tests
- Can play a full game in CuteChess/Arena without crashing
- Handles all UCI commands needed for a complete game
- Estimated strength: roughly 1200–1500 Elo (comparable to a casual club player)
- Benchmark: run a self-play tournament of 100+ games to verify stability

---

## Phase 7 — Search Improvements

**Goal:** Make the search faster and deeper with well-known techniques. Each of these is independently valuable and can be added one at a time.

### Techniques (roughly in order of impact)
1. **Transposition table:** Cache previously searched positions in a hash table (Zobrist hashing). Avoids redundant work when the same position is reached via different move orders. Typically doubles effective search depth
2. **Move ordering:** Search the best moves first to maximise alpha-beta cutoffs. Order: hash move → captures (MVV-LVA) → killer moves → history heuristic → remaining
3. **Null move pruning:** "What if I do nothing?" If passing (illegal but hypothetical) still results in a beta cutoff, the position is so good we can prune. Huge speed gain in non-zugzwang positions
4. **Late move reductions (LMR):** Moves searched late in the move list (which are likely bad due to move ordering) are searched at reduced depth first. If they look promising, re-search at full depth
5. **Aspiration windows:** Start each iterative deepening iteration with a narrow alpha-beta window around the previous score. If the score falls outside, re-search with a wider window
6. **Principal variation search (PVS):** After searching the first (presumed best) move with a full window, search remaining moves with a zero window. Re-search with full window only if they beat the current best
7. **Check extensions:** Extend search depth by 1 when in check, to avoid missing forced tactical sequences

### Validation
- After each technique, run a before/after tournament (100+ games) to measure Elo gain
- Monitor nodes-per-second to ensure no regression
- Re-run perft to confirm move generation is unchanged

---

## Phase 8 — Evaluation Improvements

**Goal:** Strengthen the handcrafted evaluation with more chess knowledge.

### Features to Add (in order of typical impact)
1. **Mobility:** Count legal/pseudo-legal moves for each piece. More mobility = better
2. **King safety:** Pawn shield evaluation, open files near king, attacker count
3. **Passed pawn evaluation:** Distance to promotion, support from own pieces, blocked vs free
4. **Rook on open/semi-open files:** Rooks need open lines to be effective
5. **Bishop pair bonus:** Two bishops are worth more than their individual values suggest
6. **Outpost squares:** Knights on outpost squares (protected by own pawn, no enemy pawn can attack) are strong
7. **Space advantage:** Control of central squares
8. **Tempo bonus for side to move:** A small bonus for having the initiative

### Tuning
- Use Texel's tuning method or manual adjustment against test suites
- Play tournaments against engines of known strength (e.g., TSCP, Vice, CPW-Engine)
- Target strength: 1800–2200 Elo after phases 7 and 8

---

## Phase 9 — Time Management & UCI Completion

**Goal:** Handle real game time controls properly and implement the remaining UCI features.

### Deliverables
- Parse `go wtime N btime N winc N binc N movestogo N`
- Allocate time per move based on remaining time, increment, and estimated moves remaining
- Implement `go infinite`, `go nodes N`, `go mate N`
- Support `stop` (interrupt search and return best move found so far — requires search to be interruptible)
- Report `info` strings during search: `info depth D score cp S nodes N nps X time T pv e2e4 e7e5 ...`
- Support UCI options: `Hash` (transposition table size), `Threads` (for future use)
- Handle `ponderhit` and `go ponder` (think on opponent's time)

---

## Phase 10 — Testing, Profiling & Strength Measurement

**Goal:** Systematically measure and improve engine strength.

### Deliverables
- **Benchmark suite:** WAC (Win at Chess), STS (Strategic Test Suite), ECM (Encyclopaedia of Chess Middlegames) — measure how many positions the engine solves correctly at fixed time
- **Self-play tournament:** Round-robin at various time controls using CuteChess-CLI
- **Profiling:** Use `gprof` or `perf` to identify bottlenecks. Typical hot spots: move generation, evaluation, transposition table lookups
- **Opening book:** Optionally add a small opening book (Polyglot format) so the engine doesn't waste time in well-known theory
- **Endgame tablebases:** Optionally integrate Syzygy tablebases for perfect endgame play with ≤6 pieces

---

## Phase 11 — NNUE Integration

**Goal:** Replace (or supplement) the handcrafted evaluation with an NNUE (Efficiently Updatable Neural Network).

**Why NNUE:** NNUE evaluations are dramatically stronger than handcrafted eval. Stockfish gained roughly 80 Elo from switching to NNUE. The "efficiently updatable" property means the neural network can be updated incrementally after each move, rather than recomputed from scratch, making it fast enough for use inside a search.

### Approach
- Train a simple NNUE on self-play games or games from a database (e.g., CCRL games)
- Start with the HalfKP architecture (the original Stockfish NNUE architecture) — well-documented and understood
- Implement incremental updates in the accumulator during make/unmake
- Blend NNUE eval with classical eval during development to compare
- This phase is a significant undertaking and could be its own sub-project

### Prerequisites
- Phases 0–10 should be solid before attempting this
- Familiarity with basic neural network concepts (forward pass, training, quantisation)
- A training framework (likely Python + PyTorch for training, then export weights for C++ inference)

---

## Reference Resources

- **Chess Programming Wiki:** https://www.chessprogramming.org — the definitive reference for every topic above
- **Stockfish source:** https://github.com/official-stockfish/Stockfish — the strongest open-source engine, excellent reference implementation
- **"Chess Programming" by François Dominic Laramée** — accessible introductory series
- **Talk Chess forum:** https://talkchess.com — community of engine authors
- **Cute Chess:** https://cutechess.com — GUI and CLI for testing engines
- **CCRL:** https://www.computerchess.org.uk/ccrl/ — computer chess rating lists for benchmarking

---

## Notes and Decisions Log

*Record key decisions, tradeoffs, and lessons learned as the project progresses.*

| Date | Decision | Rationale |
|------|----------|-----------|
| | | |
