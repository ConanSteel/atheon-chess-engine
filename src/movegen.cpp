#include "movegen.h"
#include "attacks.h"
#include "bitboard.h"
#include "board.h"

/*-----------------------*/
/* KNIGHT MOVE GENERATOR */
/*-----------------------*/

int generate_knight_moves(const Position& pos, Move* move_list, int move_count) {
    /*Figure out who's moving and get the required bitboards*/
    Color us   = pos.get_side_to_move();
    Color them = (us == Color::White) ? Color::Black : Color::White;
    Bitboard friendly = pos.get_all_pieces(us)  ;
    Bitboard enemy    = pos.get_all_pieces(them);
    Bitboard knights  = pos.get_pieces(us, PieceType::Knight);

    /*Loop through each knight*/
    while (knights) {
        Square from_sq = static_cast<Square>(lsb(knights));
        Bitboard attacks = KNIGHT_ATTACKS[static_cast<int>(from_sq)] & ~friendly;

        /*Loop through each target square to create move*/
        while (attacks) {
            Square to_sq = static_cast<Square>(lsb(attacks));

            if (enemy & (1ULL << static_cast<int>(to_sq))) {
                move_list[move_count++] = create_move(from_sq, to_sq, MOVE_CAPTURE);
            } else {
                move_list[move_count++] = create_move(from_sq, to_sq, MOVE_QUIET);
            }

            attacks &= attacks - 1;
        }

        knights &= knights - 1;
    }

    return move_count;
}

/*---------------------*/
/* KING MOVE GENERATOR */
/*---------------------*/

int generate_king_moves(const Position& pos, Move* move_list, int move_count) {
    /*Figure out who is moving and get the required bitboards*/
    Color us   = pos.get_side_to_move();
    Color them = (us == Color::White) ? Color::Black : Color::White;
    Bitboard friendly   = pos.get_all_pieces(us)              ;
    Bitboard enemy      = pos.get_all_pieces(them)            ;
    Bitboard king       = pos.get_pieces(us, PieceType::King) ;
    Bitboard all_pieces = pos.get_all_pieces(Color::White)    | 
                          pos.get_all_pieces(Color::Black)    ;

    /*There is only one king so only a single loop is needed*/
    Square from_sq = static_cast<Square>(lsb(king));
    Bitboard attacks = KING_ATTACKS[static_cast<int>(from_sq)] & ~friendly;

    /*Single loop for each target square*/
    while (attacks) {
        Square to_sq = static_cast<Square>(lsb(attacks));

        if (enemy & (1ULL << static_cast<int>(to_sq))) {
            move_list[move_count++] = create_move(from_sq, to_sq, MOVE_CAPTURE);
        } else {
            move_list[move_count++] = create_move(from_sq, to_sq, MOVE_QUIET);
        }

        attacks &= attacks - 1;
    }

    /*---------------------*/
    /* FOUR CASTLING CASES */
    /*---------------------*/

    /*White*/
    if (us == Color::White) {
        /*White kingside*/
        if (pos.get_castling_rights(0)) {
            /*Check for empty squares between king and rook*/
            Bitboard between = (1ULL << static_cast<int>(Square::F1)) |
                               (1ULL << static_cast<int>(Square::G1)) ;

            if (!(all_pieces & between)) {
                /*Check for king either currently in, moving through, or landing in check*/
                if (!is_square_attacked(pos, static_cast<int>(Square::E1), Color::Black) &&
                    !is_square_attacked(pos, static_cast<int>(Square::F1), Color::Black) &&
                    !is_square_attacked(pos, static_cast<int>(Square::G1), Color::Black)) {

                    move_list[move_count++] = create_move(Square::E1, Square::G1, MOVE_KSIDE_CASTLE);
                }
            }
        }

        /*White queenside*/
        if (pos.get_castling_rights(1)) {
            /*Check for empty squares between king and rook*/
            Bitboard between = (1ULL << static_cast<int>(Square::B1)) |
                               (1ULL << static_cast<int>(Square::C1)) |
                               (1ULL << static_cast<int>(Square::D1)) ;
                           
            if (!(all_pieces & between)) {
                /*Check for king either currently in, moving through, or landing in check*/
                if (!is_square_attacked(pos, static_cast<int>(Square::E1), Color::Black) &&
                    !is_square_attacked(pos, static_cast<int>(Square::D1), Color::Black) &&
                    !is_square_attacked(pos, static_cast<int>(Square::C1), Color::Black)) {

                    move_list[move_count++] = create_move(Square::E1, Square::C1, MOVE_QSIDE_CASTLE);
                }
            }
        }
    /*Black*/
    } else {
        /*Black kingside*/
        if (pos.get_castling_rights(2)) {
            /*Check for empty squares between king and rook*/
            Bitboard between = (1ULL << static_cast<int>(Square::F8)) |
                               (1ULL << static_cast<int>(Square::G8)) ;

            if (!(all_pieces & between)) {
                /*Check for king either currently in, moving through, or landing in check*/
                if (!is_square_attacked(pos, static_cast<int>(Square::E8), Color::White) &&
                    !is_square_attacked(pos, static_cast<int>(Square::F8), Color::White) &&
                    !is_square_attacked(pos, static_cast<int>(Square::G8), Color::White)) {

                    move_list[move_count++] = create_move(Square::E8, Square::G8, MOVE_KSIDE_CASTLE);
                }
            }
        }

        /*Black queenside*/
        if (pos.get_castling_rights(3)) {
            /*Check for empty squares between king and rook*/
            Bitboard between = (1ULL << static_cast<int>(Square::B8)) |
                               (1ULL << static_cast<int>(Square::C8)) |
                               (1ULL << static_cast<int>(Square::D8)) ;
                           
            if (!(all_pieces & between)) {
                /*Check for king either currently in, moving through, or landing in check*/
                if (!is_square_attacked(pos, static_cast<int>(Square::E8), Color::White) &&
                    !is_square_attacked(pos, static_cast<int>(Square::D8), Color::White) &&
                    !is_square_attacked(pos, static_cast<int>(Square::C8), Color::White)) {

                    move_list[move_count++] = create_move(Square::E8, Square::C8, MOVE_QSIDE_CASTLE);
                }
            }
        }
    }
    
    return move_count;
}

/*---------------------*/
/* PAWN MOVE GENERATOR */
/*---------------------*/

int generate_pawn_moves(const Position& pos, Move* move_list, int move_count) {
    
    /*-------------*/
    /* DEFINITIONS */
    /*-------------*/

    /*Figure out who is moving and get the required bitboards*/
    Color us   = pos.get_side_to_move();
    Color them = (us == Color::White) ? Color::Black : Color::White;

    /*Compute all pawn moves simultaneously rather than looping through each one*/
    Bitboard all_pieces = pos.get_all_pieces(us) | pos.get_all_pieces(them);
    Bitboard pawns  = pos.get_pieces(us, PieceType::Pawn);
    
    /*Still need the enemy variable for attacks*/
    Bitboard enemy = pos.get_all_pieces(them);

    /*Single push variable needed for loop*/
    Bitboard single_pushes;
    constexpr Bitboard RANK_8 = 0xFF00000000000000ULL;
    constexpr Bitboard RANK_1 = 0x00000000000000FFULL;
    /*Define the promotion rank*/
    Bitboard promo_rank = (us == Color::White) ? RANK_8 : RANK_1;
    if (us == Color::White) {
        single_pushes = (pawns << 8) & ~all_pieces;
    } else {
        single_pushes = (pawns >> 8) & ~all_pieces;
    }

    /*Double push variable needed for loop*/
    Bitboard double_pushes;
    constexpr Bitboard RANK_4 = 0x00000000FF000000ULL;
    constexpr Bitboard RANK_5 = 0x000000FF00000000ULL;
    if (us == Color::White) {
        double_pushes = (single_pushes << 8) & ~all_pieces & RANK_4;
    } else {
        double_pushes = (single_pushes >> 8) & ~all_pieces & RANK_5;
    }

    /*Pawn dummy variable needed to loop through each pawn for attacks*/
    Bitboard pawn_iter = pawns;

    /*-----------------*/
    /* MOVE GENERATORS */
    /*-----------------*/

    /*Single loop for each of the resulting bitboard*/
    while (single_pushes) {
        Square to_sq = static_cast<Square>(lsb(single_pushes));
        
        /*Calculate original square*/
        int from_idx = (us == Color::White) ? static_cast<int>(to_sq) - 8
                                            : static_cast<int>(to_sq) + 8;
        Square from_sq = static_cast<Square>(from_idx);

        /*Check for promotion*/
        if ((1ULL << static_cast<int>(to_sq)) & promo_rank) {
            move_list[move_count++] = create_move(from_sq, to_sq, MOVE_KNIGHT_PROMO);
            move_list[move_count++] = create_move(from_sq, to_sq, MOVE_BISHOP_PROMO);
            move_list[move_count++] = create_move(from_sq, to_sq, MOVE_ROOK_PROMO);
            move_list[move_count++] = create_move(from_sq, to_sq, MOVE_QUEEN_PROMO);
        } else {
            move_list[move_count++] = create_move(from_sq, to_sq, MOVE_QUIET);
        }

        single_pushes &= single_pushes - 1;
    }

    /*Push single_pushes once more to test for double pushes*/
    while (double_pushes) {
        Square to_sq = static_cast<Square>(lsb(double_pushes));
        
        /*Calculate original square*/
        int from_idx = (us == Color::White) ? static_cast<int>(to_sq) - 16
                                            : static_cast<int>(to_sq) + 16;
        Square from_sq = static_cast<Square>(from_idx);
        move_list[move_count++] = create_move(from_sq, to_sq, MOVE_PAWN_DOUBLE);

        /*No promotion check needed for double push since it can never move to final rank*/
        double_pushes &= double_pushes - 1;
    }

    /*Normal attacks*/
    while (pawn_iter) {
        Square from_sq = static_cast<Square>(lsb(pawn_iter));
        Bitboard attacks = PAWN_ATTACKS[static_cast<int>(us)][static_cast<int>(from_sq)] & enemy;
        
        while (attacks) {
            Square to_sq = static_cast<Square>(lsb(attacks));

            /*Check for promotion*/
            if ((1ULL << static_cast<int>(to_sq)) & promo_rank) {
                move_list[move_count++] = create_move(from_sq, to_sq, MOVE_KNIGHT_PROMO_CAP);
                move_list[move_count++] = create_move(from_sq, to_sq, MOVE_BISHOP_PROMO_CAP);
                move_list[move_count++] = create_move(from_sq, to_sq, MOVE_ROOK_PROMO_CAP);
                move_list[move_count++] = create_move(from_sq, to_sq, MOVE_QUEEN_PROMO_CAP);
            } else {
                move_list[move_count++] = create_move(from_sq, to_sq, MOVE_CAPTURE);
            }

            attacks &= attacks - 1;
        }

        pawn_iter &= pawn_iter - 1;
    }

    /*En passant attacks*/
    Square ep_square = pos.ep_square();
    if (ep_square != Square::NUM_SQUARES) {
        Bitboard ep_captures = PAWN_ATTACKS[static_cast<int>(them)][static_cast<int>(ep_square)] & pawns;

        while (ep_captures) {
            Square from_sq = static_cast<Square>(lsb(ep_captures));
            move_list[move_count++] = create_move(from_sq, ep_square, MOVE_EN_PASSANT_CAP);
            ep_captures &= ep_captures - 1;
        }
    }

    return move_count;
}

/*-----------------------*/
/* BISHOP MOVE GENERATOR */
/*-----------------------*/

int generate_bishop_moves(const Position& pos, Move* move_list, int move_count) {
    /*Figure out who's moving and get the required bitboards*/
    Color us   = pos.get_side_to_move();
    Color them = (us == Color::White) ? Color::Black : Color::White;
    Bitboard friendly = pos.get_all_pieces(us)  ;
    Bitboard enemy    = pos.get_all_pieces(them);
    Bitboard bishops  = pos.get_pieces(us, PieceType::Bishop);
    Bitboard occupied = pos.get_all_pieces(us) | pos.get_all_pieces(them);

    /*Loop through each bishop*/
    while (bishops) {
        Square from_sq = static_cast<Square>(lsb(bishops));
        Bitboard attacks = get_bishop_attacks(static_cast<int>(from_sq), occupied) & ~friendly;

        /*Loop through each target square to create move*/
        while (attacks) {
            Square to_sq = static_cast<Square>(lsb(attacks));

            if (enemy & (1ULL << static_cast<int>(to_sq))) {
                move_list[move_count++] = create_move(from_sq, to_sq, MOVE_CAPTURE);
            } else {
                move_list[move_count++] = create_move(from_sq, to_sq, MOVE_QUIET);
            }

            attacks &= attacks - 1;
        }

        bishops &= bishops - 1;
    }

    return move_count;
}

/*---------------------*/
/* ROOK MOVE GENERATOR */
/*---------------------*/

int generate_rook_moves(const Position& pos, Move* move_list, int move_count) {
    /*Figure out who's moving and get the required bitboards*/
    Color us   = pos.get_side_to_move();
    Color them = (us == Color::White) ? Color::Black : Color::White;
    Bitboard friendly = pos.get_all_pieces(us)  ;
    Bitboard enemy    = pos.get_all_pieces(them);
    Bitboard rooks    = pos.get_pieces(us, PieceType::Rook);
    Bitboard occupied = pos.get_all_pieces(us) | pos.get_all_pieces(them);

    /*Loop through each rook*/
    while (rooks) {
        Square from_sq = static_cast<Square>(lsb(rooks));
        Bitboard attacks = get_rook_attacks(static_cast<int>(from_sq), occupied) & ~friendly;

        /*Loop through each target square to create move*/
        while (attacks) {
            Square to_sq = static_cast<Square>(lsb(attacks));

            if (enemy & (1ULL << static_cast<int>(to_sq))) {
                move_list[move_count++] = create_move(from_sq, to_sq, MOVE_CAPTURE);
            } else {
                move_list[move_count++] = create_move(from_sq, to_sq, MOVE_QUIET);
            }

            attacks &= attacks - 1;
        }

        rooks &= rooks - 1;
    }

    return move_count;
}

/*----------------------*/
/* QUEEN MOVE GENERATOR */
/*----------------------*/

int generate_queen_moves(const Position& pos, Move* move_list, int move_count) {
    /*Figure out who's moving and get the required bitboards*/
    Color us   = pos.get_side_to_move();
    Color them = (us == Color::White) ? Color::Black : Color::White;
    Bitboard friendly = pos.get_all_pieces(us)  ;
    Bitboard enemy    = pos.get_all_pieces(them);
    Bitboard queens   = pos.get_pieces(us, PieceType::Queen);
    Bitboard occupied = pos.get_all_pieces(us) | pos.get_all_pieces(them);

    /*Loop through each queen (promotion allows for multiple queens)*/
    while (queens) {
        Square from_sq = static_cast<Square>(lsb(queens));
        Bitboard attacks = (get_bishop_attacks(static_cast<int>(from_sq), occupied)  | 
                              get_rook_attacks(static_cast<int>(from_sq), occupied)) & 
                                                                           ~friendly ;

        /*Loop through each target square to create move*/
        while (attacks) {
            Square to_sq = static_cast<Square>(lsb(attacks));

            if (enemy & (1ULL << static_cast<int>(to_sq))) {
                move_list[move_count++] = create_move(from_sq, to_sq, MOVE_CAPTURE);
            } else {
                move_list[move_count++] = create_move(from_sq, to_sq, MOVE_QUIET);
            }

            attacks &= attacks - 1;
        }

        queens &= queens - 1;
    }

    return move_count;
}

/*-------------------------*/
/* COMPLIED MOVE GENERATOR */
/*-------------------------*/

int generate_moves(const Position& pos, Move* move_list) {
    int move_count = 0;

    move_count =   generate_pawn_moves(pos, move_list, move_count) ;
    move_count = generate_knight_moves(pos, move_list, move_count) ;
    move_count = generate_bishop_moves(pos, move_list, move_count) ;
    move_count =   generate_rook_moves(pos, move_list, move_count) ;
    move_count =  generate_queen_moves(pos, move_list, move_count) ;
    move_count =   generate_king_moves(pos, move_list, move_count) ;

    return move_count;
}

/*----------------------*/
/* CHECKS MOVE LEGALITY */
/*----------------------*/

int generate_legal_moves(Position& pos, Move* move_list) {
    Move pseudo_legal[256];
    int num_moves = generate_moves(pos, pseudo_legal);

    int legal_count = 0;
    UndoInfo undo;

    for (int i = 0; i < num_moves; i++) {
        pos.make_move(pseudo_legal[i], undo);

        /*Side to move has switched so we need to find opponent king*/
        Color them_now = pos.get_side_to_move();
        Color us_orig  = (them_now == Color::White) ? Color::Black : Color::White;

        /*Find the square that has the king*/
        int king_sq = lsb(pos.get_pieces(us_orig, PieceType::King));

        /*Check if that square is under attack*/
        bool in_check = is_square_attacked(pos, king_sq, them_now);
        pos.unmake_move(pseudo_legal[i], undo);

        if (!in_check) {
            move_list[legal_count++] = pseudo_legal[i];
        }
    }

    return legal_count;
}

/*---------------------------*/
/* COUNTS AVAILABLE CAPTURES */
/*---------------------------*/

int generate_captures(Position& pos, Move* move_list) {
    Move all_moves[256];
    int num_moves = generate_legal_moves(pos, all_moves);

    int capture_num = 0;

    for (int i = 0; i < num_moves; i++) {
        Move flags = move_flags(all_moves[i]);

        if (flags & MOVE_CAPTURE) {
            move_list[capture_num] = all_moves[i];
            capture_num++;
        }
    }
    
    return capture_num;
}
