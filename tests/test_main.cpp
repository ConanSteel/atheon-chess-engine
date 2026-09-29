#include <catch2/catch_session.hpp>
#include <iostream>
#include "types.h"
#include "board.h"
#include "perft.h"
#include "attacks.h"
#include "search.h"
#include "uci.h"
#include "eval.h"
#include "zobrist.h"
#include "tt.h"


int main(int argc, char* argv[]) {
    init_attack_tables() ;
    init_eval()          ;
    Zobrist::init()      ;
    tt.resize(64)        ;
    return Catch::Session().run(argc, argv);
}
