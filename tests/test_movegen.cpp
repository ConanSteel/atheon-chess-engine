#include <catch2/catch_test_macros.hpp>
#include "movegen.h"
#include "attacks.h"

TEST_CASE("Starting position move count", "[movegen]") {
    init_attack_tables();

    Position pos;
    pos.set("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");

    Move move_list[256];
    int move_count = generate_moves(pos, move_list);

    REQUIRE(move_count == 20);
}

TEST_CASE("Pawn capture and king move position move count", "[movegen]") {
    init_attack_tables();

    Position pos;
    pos.set("rnbqkbnr/ppp1pppp/8/3p4/4P3/8/PPPP1PPP/RNBQKBNR w KQkq - 0 2");

    Move move_list[256];
    int move_count = generate_moves(pos, move_list);

    REQUIRE(move_count == 31);
}

TEST_CASE("Pawn promotion and king moves", "[movegen]") {
    init_attack_tables();

    Position pos;
    pos.set("8/P7/8/8/8/8/8/4K2k w - - 0 1");

    Move move_list[256];
    int move_count = generate_moves(pos, move_list);

    REQUIRE(move_count == 9);
}

TEST_CASE("Random sliding piece check", "[movegen]") {
    init_attack_tables();

    Position pos;
    pos.set("r1bqkbnr/pppp1ppp/2n5/4p3/2B1P3/5N2/PPPP1PPP/RNBQK2R b KQkq - 3 3");

    Move move_list[256];
    int move_count = generate_moves(pos, move_list);

    REQUIRE(move_count == 31);
}

TEST_CASE("is_square_attacked -- starting position", "[movegen]") {
    init_attack_tables();

    Position pos;
    pos.set("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");

    /*White pawns attack squares on rank 3*/
    CHECK(is_square_attacked(pos, static_cast<int>(Square::D3), Color::White) == true);
    /*No white piece attacks D4*/
    CHECK(is_square_attacked(pos, static_cast<int>(Square::D4), Color::White) == false);
    /*Black pawns attack squares on rank 6*/
    CHECK(is_square_attacked(pos, static_cast<int>(Square::E6), Color::Black) == true);
    /*No black piece attacks E1*/
    CHECK(is_square_attacked(pos, static_cast<int>(Square::E1), Color::Black) == false);
}

TEST_CASE("is_square_attacked -- central knight", "[movegen]") {
    init_attack_tables();

    Position pos;
    pos.set("r1bqkbnr/pppppppp/2n5/4N3/8/8/PPPPPPPP/RNBQKB1R w KQkq - 0 1");

    /*White knight on E5 attacks the following squares*/
    CHECK(is_square_attacked(pos, static_cast<int>(Square::D3), Color::White) == true);
    CHECK(is_square_attacked(pos, static_cast<int>(Square::F3), Color::White) == true);
    CHECK(is_square_attacked(pos, static_cast<int>(Square::D7), Color::White) == true);
    CHECK(is_square_attacked(pos, static_cast<int>(Square::F7), Color::White) == true);
    CHECK(is_square_attacked(pos, static_cast<int>(Square::C4), Color::White) == true);
    CHECK(is_square_attacked(pos, static_cast<int>(Square::G4), Color::White) == true);
    CHECK(is_square_attacked(pos, static_cast<int>(Square::C6), Color::White) == true);
    CHECK(is_square_attacked(pos, static_cast<int>(Square::G6), Color::White) == true);
    /*C6 is also attacked by black*/
    CHECK(is_square_attacked(pos, static_cast<int>(Square::C6), Color::Black) == true);
}

TEST_CASE("en passant -- white captures", "[movegen]") {
    init_attack_tables();
    
    Position pos;
    pos.set("rnbqkbnr/ppp1pppp/8/3pP3/8/8/PPPP1PPP/RNBQKBNR w KQkq d6 0 1");

    Move moves[256];
    int count = generate_moves(pos, moves);

    /*E5 to D6 ep capture*/
    bool found_ep = false;
    for (int i = 0; i < count; i++) {
        if (move_from(moves[i]) == Square::E5 &&
            move_to(moves[i])   == Square::D6 &&
            move_flags(moves[i]) == MOVE_EN_PASSANT_CAP) {
            found_ep = true;
        }
    }
    CHECK(found_ep == true);
}

TEST_CASE("en passant -- black captures", "[movegen]") {
    init_attack_tables();

    Position pos;
    pos.set("rnbqkbnr/pppppppp/8/8/3pP3/8/PPPP1PPP/RNBQKBNR b KQkq e3 0 1");

    Move moves[256];
    int count = generate_moves(pos, moves);

    /*E5 to D6 ep capture*/
    bool found_ep = false;
    for (int i = 0; i < count; i++) {
        if (move_from(moves[i]) == Square::D4 &&
            move_to(moves[i])   == Square::E3 &&
            move_flags(moves[i]) == MOVE_EN_PASSANT_CAP) {
            found_ep = true;
        }
    }
    CHECK(found_ep == true);
}

TEST_CASE("en passant -- not available from start", "[movegen]") {
    init_attack_tables();

    Position pos;
    pos.set("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");

    Move moves[256];
    int count = generate_moves(pos, moves);

    bool found_ep = false;
    for (int i = 0; i < count; i++) {
        if (move_flags(moves[i]) == MOVE_EN_PASSANT_CAP) {
            found_ep = true;
        }
    }
    CHECK(found_ep == false);
}

TEST_CASE("castling -- white both sides", "[movegen]") {
    init_attack_tables();
    
    Position pos;
    pos.set("r3k2r/pppppppp/8/8/8/8/PPPPPPPP/R3K2R w KQkq - 0 1");

    Move moves[256];
    int count = generate_moves(pos, moves);

    bool found_kside = false;
    bool found_qside = false;
    for (int i = 0; i < count; i++) {
        if (move_from(moves[i]) == Square::E1 && move_to(moves[i]) == Square::G1 &&
            move_flags(moves[i]) == MOVE_KSIDE_CASTLE)
            found_kside = true;
        if (move_from(moves[i]) == Square::E1 && move_to(moves[i]) == Square::C1 &&
            move_flags(moves[i]) == MOVE_QSIDE_CASTLE)
            found_qside = true;
    }
    CHECK(found_kside == true);
    CHECK(found_qside == true);
}

TEST_CASE("castling -- black both sides", "[movegen]") {
    init_attack_tables();
    
    Position pos;
    pos.set("r3k2r/pppppppp/8/8/8/8/PPPPPPPP/R3K2R b KQkq - 0 1");

    Move moves[256];
    int count = generate_moves(pos, moves);

    bool found_kside = false;
    bool found_qside = false;
    for (int i = 0; i < count; i++) {
        if (move_from(moves[i]) == Square::E8 && move_to(moves[i]) == Square::G8 &&
            move_flags(moves[i]) == MOVE_KSIDE_CASTLE)
            found_kside = true;
        if (move_from(moves[i]) == Square::E8 && move_to(moves[i]) == Square::C8 &&
            move_flags(moves[i]) == MOVE_QSIDE_CASTLE)
            found_qside = true;
    }
    CHECK(found_kside == true);
    CHECK(found_qside == true);
}

TEST_CASE("castling -- none", "[movegen]") {
    init_attack_tables();
    
    Position pos;
    pos.set("r3k2r/pppppppp/8/8/8/8/PPPPPPPP/R3K2R w - - 0 1");

    Move moves[256];
    int count = generate_moves(pos, moves);

    bool found_castle = false;
    for (int i = 0; i < count; i++) {
        if (move_flags(moves[i]) == MOVE_KSIDE_CASTLE || 
            move_flags(moves[i]) == MOVE_QSIDE_CASTLE)
            found_castle = true;
    }
    CHECK(found_castle == false);
}

TEST_CASE("castling -- white kingside blocked", "[movegen]") {
    init_attack_tables();

    Position pos;
    pos.set("r3k2r/pppppppp/8/8/5r2/8/PPPPP1PP/R3K2R w KQkq - 0 1");

    Move moves[256];
    int count = generate_moves(pos, moves);

    bool found_kside = false;
    bool found_qside = false;
    for (int i = 0; i < count; i++) {
        if (move_from(moves[i]) == Square::E1 && move_to(moves[i]) == Square::G1 &&
            move_flags(moves[i]) == MOVE_KSIDE_CASTLE)
            found_kside = true;
        if (move_from(moves[i]) == Square::E1 && move_to(moves[i]) == Square::C1 &&
            move_flags(moves[i]) == MOVE_QSIDE_CASTLE)
            found_qside = true;
    }
    CHECK(found_kside == false);
    CHECK(found_qside == true);
}

TEST_CASE("make/unmake", "[movegen]") {
    init_attack_tables();
    
    Position pos;
    pos.set("r3k2r/pppppppp/8/8/8/8/PPPPPPPP/R3K2R w KQkq - 0 1");

    std::string original_fen = pos.fen();

    Move moves[256];
    int count = generate_moves(pos, moves);

    for (int i = 0; i < count; i++) {
        UndoInfo undo;
        pos.make_move(moves[i], undo);
        pos.unmake_move(moves[i], undo);
        CHECK(pos.fen() == original_fen);
    }
}

TEST_CASE("make/unmake with promotion", "[movegen]") {
    init_attack_tables();
    
    Position pos;
    pos.set("4k3/PPP2ppp/8/8/8/8/ppp2PPP/4K3 w - - 0 1");

    std::string original_fen = pos.fen();

    Move moves[256];
    int count = generate_moves(pos, moves);

    for (int i = 0; i < count; i++) {
        UndoInfo undo;
        pos.make_move(moves[i], undo);
        pos.unmake_move(moves[i], undo);
        CHECK(pos.fen() == original_fen);
    }
}

TEST_CASE("make/unmake with en passant", "[movegen]") {
    init_attack_tables();
    
    Position pos;
    pos.set("rnbqkbnr/ppp1pppp/8/3pP3/8/8/PPPP1PPP/RNBQKBNR w KQkq d6 0 1");

    std::string original_fen = pos.fen();

    Move moves[256];
    int count = generate_moves(pos, moves);

    for (int i = 0; i < count; i++) {
        UndoInfo undo;
        pos.make_move(moves[i], undo);
        pos.unmake_move(moves[i], undo);
        CHECK(pos.fen() == original_fen);
    }
}

TEST_CASE("legal moves from starting position", "[movegen]") {
    init_attack_tables();
    
    Position pos;
    pos.set("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");

    Move moves[256];
    int count = generate_legal_moves(pos, moves);

    // starting position has exactly 20 legal moves
    CHECK(count == 20);
}

TEST_CASE("legal moves with king in check", "[movegen]") {
    init_attack_tables();
    
    Position pos;
    // white king on E1 checked by black rook on E8, no other pieces in the way
    pos.set("4r3/8/8/8/8/8/8/4K3 w - - 0 1");

    Move moves[256];
    int count = generate_legal_moves(pos, moves);

    // every legal move must get the king out of check
    // king can go to D1, D2, F1, F2 — but not E2 (still on the file)
    // verify no move leaves king on E-file exposed to rook
    for (int i = 0; i < count; i++) {
        UndoInfo undo;
        pos.make_move(moves[i], undo);

        Color them_now = pos.get_side_to_move();
        Color us_orig = (them_now == Color::White) ? Color::Black : Color::White;
        int king_sq = lsb(pos.get_pieces(us_orig, PieceType::King));
        CHECK(is_square_attacked(pos, king_sq, them_now) == false);

        pos.unmake_move(moves[i], undo);
    }
}

TEST_CASE("legal moves with pinned piece", "[movegen]") {
    init_attack_tables();
    
    Position pos;
    // white king E1, white knight D2, black rook A5 — wait, that's not a pin
    // white king E1, white bishop D2, black rook A5 — not a pin either
    // white king E1, white knight E2, black rook E8 — knight pinned on E-file
    pos.set("4r3/8/8/8/8/8/4N3/4K3 w - - 0 1");

    Move moves[256];
    int count = generate_legal_moves(pos, moves);

    // the knight on E2 is pinned — none of its moves should appear
    bool knight_moved = false;
    for (int i = 0; i < count; i++) {
        if (move_from(moves[i]) == Square::E2)
            knight_moved = true;
    }
    CHECK(knight_moved == false);
}

TEST_CASE("legal moves for stalemate", "[movegen]") {
    init_attack_tables();
    
    Position pos;
    // classic stalemate: black king A8, white queen B6, white king A6
    pos.set("k7/8/KQ6/8/8/8/8/8 b - - 0 1");

    Move moves[256];
    int count = generate_legal_moves(pos, moves);

    CHECK(count == 0);
}

TEST_CASE("legal moves for checkmate", "[movegen]") {
    init_attack_tables();
    
    Position pos;
    // back rank mate: black king G8, white rooks on A8 and A7... 
    // simpler: scholar's mate position
    pos.set("r1bqkb1r/pppp1Qpp/2n2n2/4p3/2B1P3/8/PPPP1PPP/RNB1K1NR b KQkq - 0 1");

    Move moves[256];
    int count = generate_legal_moves(pos, moves);

    CHECK(count == 0);
}
