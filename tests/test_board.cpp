#include <catch2/catch_test_macros.hpp>
#include "board.h"

TEST_CASE("FEN round-trip starting position", "[board]") {
    Position pos;
    std::string fen = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
    pos.set(fen);
    REQUIRE(pos.fen() == fen);
}

TEST_CASE("FEN round-trip en passant", "[board]") {
    Position pos;
    std::string fen = "rnbqkbnr/pppppppp/8/8/4P3/8/PPPP1PPP/RNBQKBNR b KQkq e3 0 1";
    pos.set(fen);
    REQUIRE(pos.fen() == fen);
}

TEST_CASE("FEN round-trip partial castling rights", "[board]") {
    Position pos;
    std::string fen = "r3k2r/pppppppp/8/8/8/8/PPPPPPPP/R3K2R w Kq - 0 1";
    pos.set(fen);
    REQUIRE(pos.fen() == fen);
}
