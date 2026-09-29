#include <catch2/catch_test_macros.hpp>
#include "attacks.h"
#include "bitboard.h"

TEST_CASE("Knight attack table", "[attacks]") {
    init_attack_tables();

    /*Centre squares expect 8 attacks*/
    REQUIRE(popcount(KNIGHT_ATTACKS[static_cast<int>(Square::E4)]) == 8);

    /*Corner squares expect 2 attacks*/
    REQUIRE(popcount(KNIGHT_ATTACKS[static_cast<int>(Square::A1)]) == 2);

    /*Check that the actual attacked squares are correct for A1*/
    REQUIRE(test_bit(KNIGHT_ATTACKS[static_cast<int>(Square::A1)], Square::B3));
    REQUIRE(test_bit(KNIGHT_ATTACKS[static_cast<int>(Square::A1)], Square::C2));
}

TEST_CASE("King attack table", "[attacks]") {
    init_attack_tables();

    /*Centre squares expect 8 attacks*/
    REQUIRE(popcount(KING_ATTACKS[static_cast<int>(Square::E4)]) == 8);

    /*Corner squares expect 3 attacks*/
    REQUIRE(popcount(KING_ATTACKS[static_cast<int>(Square::A1)]) == 3);

    /*Check that the actual attacked squares are correct for A1*/
    REQUIRE(test_bit(KING_ATTACKS[static_cast<int>(Square::A1)], Square::A2));
    REQUIRE(test_bit(KING_ATTACKS[static_cast<int>(Square::A1)], Square::B1));
    REQUIRE(test_bit(KING_ATTACKS[static_cast<int>(Square::A1)], Square::B2));
}

TEST_CASE("Pawn attack table", "[attacks]") {
    init_attack_tables();

    /*Centre squares expect 2 attacks*/
    REQUIRE(popcount(PAWN_ATTACKS[static_cast<int>(Color::White)][static_cast<int>(Square::E4)]) == 2);
    REQUIRE(popcount(PAWN_ATTACKS[static_cast<int>(Color::Black)][static_cast<int>(Square::E4)]) == 2);

    /*Edge squares expect 1 attack*/
    REQUIRE(popcount(PAWN_ATTACKS[static_cast<int>(Color::White)][static_cast<int>(Square::A2)]) == 1);
    REQUIRE(popcount(PAWN_ATTACKS[static_cast<int>(Color::Black)][static_cast<int>(Square::H7)]) == 1);

    /*Check that the actual attacked squares are correct for A2 and H7*/
    REQUIRE(test_bit(PAWN_ATTACKS[static_cast<int>(Color::White)][static_cast<int>(Square::A2)], Square::B3));
    REQUIRE(test_bit(PAWN_ATTACKS[static_cast<int>(Color::Black)][static_cast<int>(Square::H7)], Square::G6));
}

TEST_CASE("The Carry-Rippler pattern for bishop on e4", "[attacks]") {
    init_attack_tables();

    Bitboard mask = BISHOP_MASKS[28];
    Bitboard subset = 0;
    int count = 0;
    do {
        count++;
        subset = (subset - mask) & mask;
    } while (subset != 0);

    REQUIRE(count == 512);
}

TEST_CASE("The Carry-Rippler pattern for rook on e4", "[attacks]") {
    init_attack_tables();

    Bitboard mask = ROOK_MASKS[28];
    Bitboard subset = 0;
    int count = 0;
    do {
        count++;
        subset = (subset - mask) & mask;
    } while (subset != 0);

    REQUIRE(count == 1024);
}

TEST_CASE("The Carry-Rippler pattern for rook on a1", "[attacks]") {
    init_attack_tables();

    Bitboard mask = ROOK_MASKS[0];
    Bitboard subset = 0;
    int count = 0;
    do {
        count++;
        subset = (subset - mask) & mask;
    } while (subset != 0);

    REQUIRE(count == 4096);
}

/*The following two tests only work if you pull the bitboard attacks out of the namespace in attacks.cpp*/

TEST_CASE("Bishop attacks from e4", "[attacks]") {
    init_attack_tables();

    Bitboard blockers = (1ULL << 42) | (1ULL << 21);  /*Blockers on c6 and f3*/

    Bitboard expected = (1ULL <<  1) |  // b1
                        (1ULL << 10) |  // c2
                        (1ULL << 19) |  // d3
                        (1ULL << 21) |  // f3 (blocker, but still attacked)
                        (1ULL << 35) |  // d5
                        (1ULL << 37) |  // f5
                        (1ULL << 42) |  // c6 (blocker, but still attacked)
                        (1ULL << 46) |  // g6
                        (1ULL << 55) ;  // h7
                        
    REQUIRE(bishop_attacks_classical(28, blockers) == expected);
}

TEST_CASE("Rook attacks from e4", "[attacks]") {
    init_attack_tables();

    Bitboard blockers = (1ULL << 12) | (1ULL << 31);  /*Blockers on e2 and h4*/

    Bitboard expected = (1ULL << 24) |  // a4
                        (1ULL << 25) |  // b4
                        (1ULL << 26) |  // c4
                        (1ULL << 27) |  // d4
                        (1ULL << 29) |  // f4
                        (1ULL << 30) |  // g4
                        (1ULL << 31) |  // h4 (blocker, but still attacked)
                        (1ULL << 12) |  // e2 (blocker, but still attacked)
                        (1ULL << 20) |  // e3
                        (1ULL << 36) |  // e5
                        (1ULL << 44) |  // e6
                        (1ULL << 52) |  // e7
                        (1ULL << 60) ;  // e8
                        
    REQUIRE(rook_attacks_classical(28, blockers) == expected);
}

TEST_CASE("Magic bishop lookup matches classical", "[attacks]") {
    init_attack_tables();

    /*Same blockers on c6 and f3 as before*/
    Bitboard blockers  = (1ULL << 42) | (1ULL << 21)            ; 
    Bitboard classical = bishop_attacks_classical(28, blockers) ;
    Bitboard magic     = get_bishop_attacks(28, blockers)       ;

    REQUIRE(classical == magic);
}

TEST_CASE("Magic rook lookup matches classical", "[attacks]") {
    init_attack_tables();

    /*Same blockers on e2 and h4 as before*/
    Bitboard blockers  = (1ULL << 12) | (1ULL << 31)            ; 
    Bitboard classical = rook_attacks_classical(28, blockers) ;
    Bitboard magic     = get_rook_attacks(28, blockers)       ;

    REQUIRE(classical == magic);
}
