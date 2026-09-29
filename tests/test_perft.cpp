#include <catch2/catch_test_macros.hpp>
#include "attacks.h"
#include "perft.h"
#include "board.h"

TEST_CASE("Perft from starting position", "[perft]") {
    init_attack_tables();
    
    Position pos;
    pos.set("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");

    SECTION("depth 1") { REQUIRE(perft(pos, 1) == 20     ) ; }
    SECTION("depth 2") { REQUIRE(perft(pos, 2) == 400    ) ; }
    SECTION("depth 3") { REQUIRE(perft(pos, 3) == 8902   ) ; }
    SECTION("depth 4") { REQUIRE(perft(pos, 4) == 197281 ) ; }
}

TEST_CASE("Perft from Kiwipete", "[perft]") {
    init_attack_tables();
    
    Position pos;
    pos.set("r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq -");

    SECTION("depth 1") { REQUIRE(perft(pos, 1) == 48      ) ; }
    SECTION("depth 2") { REQUIRE(perft(pos, 2) == 2039    ) ; }
    SECTION("depth 3") { REQUIRE(perft(pos, 3) == 97862   ) ; }
    SECTION("depth 4") { REQUIRE(perft(pos, 4) == 4085603 ) ; }
}

TEST_CASE("Perft from position 3", "[perft]") {
    init_attack_tables();
    
    Position pos;
    pos.set("8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - -");

    SECTION("depth 1") { REQUIRE(perft(pos, 1) == 14     ) ; }
    SECTION("depth 2") { REQUIRE(perft(pos, 2) == 191    ) ; }
    SECTION("depth 3") { REQUIRE(perft(pos, 3) == 2812   ) ; }
    SECTION("depth 4") { REQUIRE(perft(pos, 4) == 43238  ) ; }
    SECTION("depth 5") { REQUIRE(perft(pos, 5) == 674624 ) ; }
}

TEST_CASE("Perft from position 4", "[perft]") {
    init_attack_tables();
    
    Position pos;
    pos.set("r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1");

    SECTION("depth 1") { REQUIRE(perft(pos, 1) == 6        ) ; }
    SECTION("depth 2") { REQUIRE(perft(pos, 2) == 264      ) ; }
    SECTION("depth 3") { REQUIRE(perft(pos, 3) == 9467     ) ; }
    SECTION("depth 4") { REQUIRE(perft(pos, 4) == 422333   ) ; }
    SECTION("depth 5") { REQUIRE(perft(pos, 5) == 15833292 ) ; }
}

TEST_CASE("Perft from position 5", "[perft]") {
    init_attack_tables();
    
    Position pos;
    pos.set("rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ -");

    SECTION("depth 1") { REQUIRE(perft(pos, 1) == 44       ) ; }
    SECTION("depth 2") { REQUIRE(perft(pos, 2) == 1486     ) ; }
    SECTION("depth 3") { REQUIRE(perft(pos, 3) == 62379    ) ; }
    SECTION("depth 4") { REQUIRE(perft(pos, 4) == 2103487  ) ; }
    SECTION("depth 5") { REQUIRE(perft(pos, 5) == 89941194 ) ; }
}

