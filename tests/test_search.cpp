#include <catch2/catch_test_macros.hpp>
#include "search.h"
#include "eval.h"
#include "movegen.h"
#include "attacks.h"
#include "bitboard.h"

TEST_CASE("Free black queen negamax check", "[search]") {
    init_attack_tables();
    
    Position pos;
    pos.set("rnb1kbnr/pppp1ppp/8/4p1q1/4P1Q1/8/PPPP1PPP/RNB1KBNR w KQkq - 0 1");
    int depth = 1 ;
    int ply   = 0 ;
    int alpha =  NEGATIVE_INFINITY ;
    int beta  = -NEGATIVE_INFINITY ;
    
    int best_score = negamax(pos, depth, ply, alpha, beta) ;
    REQUIRE(best_score == 900);
}

TEST_CASE("White bishop for black pawn negamax check", "[search]") {
    init_attack_tables();
    
    Position pos;
    pos.set("rnbqkbnr/ppp2ppp/8/3pp3/2B1P3/5N2/PPPP1PPP/RNBQK2R b KQkq - 1 3");
    int depth = 1 ;
    int ply   = 0 ;
    int alpha =  NEGATIVE_INFINITY ;
    int beta  = -NEGATIVE_INFINITY ;
    
    int best_score = negamax(pos, depth, ply, alpha, beta) ;
    REQUIRE(best_score == 230);
}

TEST_CASE("Defended white queen quiescence check", "[search]") {
    init_attack_tables();

    Position pos;
    pos.set("rnb1kbnr/pppp1ppp/8/4p1q1/4P1Q1/7P/PPPP1PP1/RNB1KBNR b KQkq - 0 3");
    int depth = 1 ;
    int ply   = 0 ;
    int alpha =  NEGATIVE_INFINITY ;
    int beta  = -NEGATIVE_INFINITY ;

    int best_score = negamax(pos, depth, ply, alpha, beta) ;
    REQUIRE(best_score == 0);
}
