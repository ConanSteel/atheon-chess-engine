#include <catch2/catch_test_macros.hpp>
#include "eval.h"

TEST_CASE("Starting position, eval expecting 0", "[eval]") {
    Position pos;
    std::string fen = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
    pos.set(fen);
    REQUIRE(evaluate(pos) == 0);
}

TEST_CASE("No black knight, white to move", "[eval]") {
    Position pos;
    std::string fen = "r1bqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
    pos.set(fen);
    REQUIRE(evaluate(pos) == 297);
}

TEST_CASE("No black knight, black to move", "[eval]") {
    Position pos;
    std::string fen = "r1bqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR b KQkq - 0 1";
    pos.set(fen);
    REQUIRE(evaluate(pos) == -297);
}

TEST_CASE("Doubled and isolated pawns", "[eval]") {
    Position pos;
    /*White: Ke1, pawns e2+e3 (doubled and isolated)*/
    /*Black: Ke8, pawn e7 (isolated)                 */
    /*Phase = 0, pure endgame                        */
    std::string fen = "4k3/4p3/8/8/8/4P3/4P3/4K3 w - - 0 1";
    pos.set(fen);
    REQUIRE(evaluate(pos) == 70);
}

TEST_CASE("Passed pawn on e6", "[eval]") {
    Position pos;
    /*White: Ke1, passed pawn on e6 (also isolated)*/
    /*Black: Ke8, no pawns                         */
    /*Phase = 0, pure endgame                      */
    std::string fen = "4k3/8/4P3/8/8/8/8/4K3 w - - 0 1";
    pos.set(fen);
    REQUIRE(evaluate(pos) == 281);
}
