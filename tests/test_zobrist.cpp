#include <catch2/catch_test_macros.hpp>
#include "board.h"
#include "movegen.h"
#include "zobrist.h"


static void perft_hash_check(Position& pos, int depth) {
    if (depth == 0) return;

    Move moves[256];
    int num_moves = generate_legal_moves(pos, moves);
    UndoInfo undo;

    for (int i = 0; i < num_moves; i++) {
        pos.make_move(moves[i], undo);

        uint64_t incremental = pos.hash;
        uint64_t from_scratch = Zobrist::compute_hash(pos);

        if (incremental != from_scratch) {
            Square from_sq = move_from(moves[i]);
            Square to_sq   = move_to(moves[i]);
            uint16_t flags = move_flags(moves[i]);
            FAIL("Hash mismatch after move "
                 << static_cast<int>(from_sq) << "->"
                 << static_cast<int>(to_sq)
                 << " flags=" << flags
                 << " incremental=" << incremental
                 << " from_scratch=" << from_scratch);
        }

        perft_hash_check(pos, depth - 1);
        pos.unmake_move(moves[i], undo);
    }
}


TEST_CASE("Zobrist hash: startpos depth 3", "[zobrist]") {
    Position pos;
    pos.set("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
    perft_hash_check(pos, 3);
}

TEST_CASE("Zobrist hash: Kiwipete depth 3", "[zobrist]") {
    Position pos;
    pos.set("r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1");
    perft_hash_check(pos, 3);
}

TEST_CASE("Zobrist hash: promotions position depth 3", "[zobrist]") {
    Position pos;
    pos.set("r3k2r/Ppppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1");
    perft_hash_check(pos, 3);
}

TEST_CASE("Zobrist hash: en passant position depth 3", "[zobrist]") {
    Position pos;
    pos.set("rnbqkbnr/1ppppppp/8/pP6/8/8/P1PPPPPP/RNBQKBNR w KQkq a6 0 3");
    perft_hash_check(pos, 3);
}
