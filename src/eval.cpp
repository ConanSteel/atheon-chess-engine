#include "eval.h"
#include "attacks.h"

Bitboard PASSED_PAWN_MASKS[2][64];


int evaluate(const Position& pos) {
    int white_mg = 0 ; int white_eg = 0 ;
    int black_mg = 0 ; int black_eg = 0 ;
    
    int phase = 0;

    /*Values assigned to shift between middlegame and endgame*/
    /*Pawn, knight, bishop, rook, queen, king                */
    constexpr int PHASE_WEIGHTS[6] = { 0, 1, 1, 2, 4, 0 };

    /*Gather the full board for activity scores*/
    Bitboard all_white = pos.get_all_pieces(Color::White) ;
    Bitboard all_black = pos.get_all_pieces(Color::Black) ;
    Bitboard occupied  = all_white | all_black            ;

    for (int piece_type =  static_cast<int>(PieceType::Pawn) ; 
             piece_type <= static_cast<int>(PieceType::King) ; 
             piece_type++) {

        Bitboard white_bb = pos.get_pieces(Color::White, static_cast<PieceType>(piece_type)) ;
        Bitboard black_bb = pos.get_pieces(Color::Black, static_cast<PieceType>(piece_type)) ;
        
        /*Count and weight remaining pieces to see how close game state is to endgame*/
        phase += PHASE_WEIGHTS[piece_type] * (popcount(white_bb) + popcount(black_bb));

        /*Now populate each white piece*/
        while (white_bb != 0) {

            int sq = lsb(white_bb);
            /*Square index needs to be shifted to match PST grid*/
            white_mg += PIECE_VALUES[piece_type] + PST_MG[piece_type][sq ^ 56] ;
            white_eg += PIECE_VALUES[piece_type] + PST_EG[piece_type][sq ^ 56] ;

            white_bb &= white_bb - 1;

            /*For pieces (not pawns or kings), assign bonuses for activity based on number of possible moves*/
            Bitboard attacks = 0;

            if        (piece_type == static_cast<int>(PieceType::Knight)) {
                attacks = KNIGHT_ATTACKS[sq];
            } else if (piece_type == static_cast<int>(PieceType::Bishop)) {
                attacks = get_bishop_attacks(sq, occupied);
            } else if (piece_type == static_cast<int>(PieceType::Rook  )) {
                attacks = get_rook_attacks(sq, occupied);
            } else if (piece_type == static_cast<int>(PieceType::Queen )) {
                attacks = get_bishop_attacks(sq, occupied) | get_rook_attacks(sq, occupied);
            } else { continue; }

            int move_count = popcount(attacks & ~all_white);
                
            white_mg += ACTIVITY_MG[piece_type][move_count] ;
            white_eg += ACTIVITY_EG[piece_type][move_count] ;
        }

        /*Now populate each black piece*/
        while (black_bb != 0) {

            int sq = lsb(black_bb);
            
            black_mg += PIECE_VALUES[piece_type] + PST_MG[piece_type][sq] ;
            black_eg += PIECE_VALUES[piece_type] + PST_EG[piece_type][sq] ;
            
            black_bb &= black_bb - 1;

            Bitboard attacks = 0;
                
            if        (piece_type == static_cast<int>(PieceType::Knight)) {
                attacks = KNIGHT_ATTACKS[sq];
            } else if (piece_type == static_cast<int>(PieceType::Bishop)) {
                attacks = get_bishop_attacks(sq, occupied);
            } else if (piece_type == static_cast<int>(PieceType::Rook  )) {
                attacks = get_rook_attacks(sq, occupied);
            } else if (piece_type == static_cast<int>(PieceType::Queen )) {
                attacks = get_bishop_attacks(sq, occupied) | get_rook_attacks(sq, occupied);
            } else { continue; }

            int move_count = popcount(attacks & ~all_black);
                
            black_mg += ACTIVITY_MG[piece_type][move_count] ;
            black_eg += ACTIVITY_EG[piece_type][move_count] ;
        }
    }

    /*Pawn specific point adjustments*/
    Bitboard white_pawns = pos.get_pieces(Color::White, PieceType::Pawn) ;
    Bitboard black_pawns = pos.get_pieces(Color::Black, PieceType::Pawn) ;
    
    /*Doubled pawns                                    */
    /*For each file, check how many pawns for each side*/
    for (int file = 0; file < 8; file++) {
        /*wpof = white pawns on file, bpof = black pawns on file*/
        int wpof = popcount(white_pawns & FILE_MASKS[file]) ;
        int bpof = popcount(black_pawns & FILE_MASKS[file]) ;
    
        /*Penalise >1 pawns, i.e. doubled*/
        if (wpof > 1) {
            white_mg += DOUBLED_PAWN_PENALTY * (wpof - 1) ;
            white_eg += DOUBLED_PAWN_PENALTY * (wpof - 1) ;
        }
        if (bpof > 1) {
            black_mg += DOUBLED_PAWN_PENALTY * (bpof - 1) ;
            black_eg += DOUBLED_PAWN_PENALTY * (bpof - 1) ;
        }
    }

    /*Isolated pawns                */
    /*Check adjacent files for pawns*/
    for (int file = 0; file < 8; file++) {

        /*Penalise isolated pawns*/
        if ((white_pawns & FILE_MASKS[file]) != 0) {
            if ((white_pawns & ADJACENT_FILES[file]) == 0) {
                int count = popcount(white_pawns & FILE_MASKS[file]);

                white_mg += ISOLATED_PAWN_PENALTY * count ;
                white_eg += ISOLATED_PAWN_PENALTY * count ;
            }
        }
        if ((black_pawns & FILE_MASKS[file]) != 0) {
            if ((black_pawns & ADJACENT_FILES[file]) == 0) {
                int count = popcount(black_pawns & FILE_MASKS[file]);

                black_mg += ISOLATED_PAWN_PENALTY * count ;
                black_eg += ISOLATED_PAWN_PENALTY * count ;
            }
        } 
    }

    /*Passed pawns                                  */
    /*Loop over each pawn and check passed pawn mask*/
    Bitboard temp = white_pawns;
    while (temp != 0) {
        int sq = lsb(temp) ;
        int rank = sq >> 3 ;
        int file = sq  & 7 ;

        if ((black_pawns & PASSED_PAWN_MASKS[static_cast<int>(Color::White)][sq]) == 0) {
            /*No enemy pawns can block or capture, therefore passed*/
            white_mg += PASSED_PAWN_BONUS_MG[rank] ;
            white_eg += PASSED_PAWN_BONUS_EG[rank] ;

            int ahead_sq = sq + 8;

            /*Check if the passed pawn is blocked by an enemy piece*/
            if (ahead_sq <= 63 && test_bit(occupied, static_cast<Square>(ahead_sq))) {
                white_mg += BLOCKED_PASSER_PENALTY ;
                white_eg += BLOCKED_PASSER_PENALTY ;
            } else if (ahead_sq <= 63) {
                white_mg += FREE_PASSER_BONUS_MG ;
                white_eg += FREE_PASSER_BONUS_EG ;
            }

            /*Check if the passed pawn is supported by another pawn*/
            if ((PAWN_ATTACKS[static_cast<int>(Color::Black)][sq] & white_pawns) != 0) {
                white_mg += PAWN_SUPPORT_BONUS_MG ;
                white_eg += PAWN_SUPPORT_BONUS_EG ;
            } 

            /*Check for friendly rook supporting the pawn*/
            Bitboard friendly_rooks_on_file = pos.get_pieces(Color::White, PieceType::Rook) & FILE_MASKS[file];
            Bitboard behind_mask = (1ULL << (rank * 8)) - 1;

            if ((friendly_rooks_on_file & behind_mask) != 0) {
                white_mg += ROOK_SUPPORT_BONUS_MG ;
                white_eg += ROOK_SUPPORT_BONUS_EG ;
            }
        }

        temp &= temp -1;
    }
    
    temp = black_pawns;
    while (temp != 0) {
        int sq = lsb(temp) ;
        int rank = sq >> 3 ;
        int file = sq  & 7 ;

        if ((white_pawns & PASSED_PAWN_MASKS[static_cast<int>(Color::Black)][sq]) == 0) {
            black_mg += PASSED_PAWN_BONUS_MG[7 - rank] ;
            black_eg += PASSED_PAWN_BONUS_EG[7 - rank] ;

            int ahead_sq = sq - 8;

            if (ahead_sq >= 0 && test_bit(occupied, static_cast<Square>(ahead_sq))) {
                black_mg += BLOCKED_PASSER_PENALTY ;
                black_eg += BLOCKED_PASSER_PENALTY ;
            } else if (ahead_sq >= 0) {
                black_mg += FREE_PASSER_BONUS_MG ;
                black_eg += FREE_PASSER_BONUS_EG ;
            }

            if ((PAWN_ATTACKS[static_cast<int>(Color::White)][sq] & black_pawns) != 0) {
                black_mg += PAWN_SUPPORT_BONUS_MG ;
                black_eg += PAWN_SUPPORT_BONUS_EG ;
            } 

            Bitboard friendly_rooks_on_file = pos.get_pieces(Color::Black, PieceType::Rook) & FILE_MASKS[file];
            Bitboard behind_mask = ~((1ULL << ((rank + 1) * 8)) - 1);

            if ((friendly_rooks_on_file & behind_mask) != 0) {
                black_mg += ROOK_SUPPORT_BONUS_MG ;
                black_eg += ROOK_SUPPORT_BONUS_EG ;
            }
        }

        temp &= temp -1;
    }

    /*Knight outpost bonus*/
    Bitboard white_knights = pos.get_pieces(Color::White, PieceType::Knight) ;
    Bitboard black_knights = pos.get_pieces(Color::Black, PieceType::Knight) ;
    
    while (white_knights != 0) {
        int   sq = lsb(white_knights) ;
        int rank = sq >> 3            ;
        int file = sq  & 7            ;
        int outpost_condition = 0     ;

        /*Ranks 4-6 count as outposts*/
        if (!((rank < 3) || (rank > 5))) {
            outpost_condition++;
        } 

        /*Check that the knight is supported by a pawn*/
        if ((PAWN_ATTACKS[static_cast<int>(Color::Black)][sq] & white_pawns) != 0) {
            outpost_condition++;
        }

        /*Check for enemy pawns that could attack the outpost square*/
        if ((black_pawns & PASSED_PAWN_MASKS[static_cast<int>(Color::White)][sq] & ADJACENT_FILES[file]) == 0) {
            outpost_condition++;
        }

        /*If all three conditions are met, we have a knight outpost*/
        if (outpost_condition == 3) {
            white_mg += KNIGHT_OUTPOST_BONUS_MG ;
            white_eg += KNIGHT_OUTPOST_BONUS_EG ;
        }

        white_knights &= white_knights - 1;
    }

    while (black_knights != 0) {
        int   sq = lsb(black_knights) ;
        int rank = sq >> 3            ;
        int file = sq  & 7            ;
        int outpost_condition = 0     ;

        if (!((rank < 2) || (rank > 4))) {
            outpost_condition++;
        } 

        if ((PAWN_ATTACKS[static_cast<int>(Color::White)][sq] & black_pawns) != 0) {
            outpost_condition++;
        }

        if ((white_pawns & PASSED_PAWN_MASKS[static_cast<int>(Color::Black)][sq] & ADJACENT_FILES[file]) == 0) {
            outpost_condition++;
        }

        if (outpost_condition == 3) {
            black_mg += KNIGHT_OUTPOST_BONUS_MG ;
            black_eg += KNIGHT_OUTPOST_BONUS_EG ;
        }

        black_knights &= black_knights - 1;
    }

    /*Bishop pair bonus*/
    if (popcount(pos.get_pieces(Color::White, PieceType::Bishop)) >= 2) {
        white_mg += BISHOP_PAIR_BONUS_MG ;
        white_eg += BISHOP_PAIR_BONUS_EG ;
    }
    if (popcount(pos.get_pieces(Color::Black, PieceType::Bishop)) >= 2) {
        black_mg += BISHOP_PAIR_BONUS_MG ;
        black_eg += BISHOP_PAIR_BONUS_EG ;
    }

    /*Open and semi-open rook files*/
    Bitboard white_rooks = pos.get_pieces(Color::White, PieceType::Rook) ;
    Bitboard black_rooks = pos.get_pieces(Color::Black, PieceType::Rook) ;

    while (white_rooks != 0) {
        int sq = lsb(white_rooks);
        int file = sq & 7;

        if ((white_pawns & FILE_MASKS[file]) == 0 && (black_pawns & FILE_MASKS[file]) == 0) {
            white_mg += ROOK_OPEN_FILE_BONUS_MG ;
            white_eg += ROOK_OPEN_FILE_BONUS_EG ;
        } else if ((white_pawns & FILE_MASKS[file]) == 0) {
            white_mg += ROOK_SEMI_OPEN_FILE_BONUS_MG ;
            white_eg += ROOK_SEMI_OPEN_FILE_BONUS_EG ;
        }

        white_rooks &= white_rooks - 1;
    }

    while (black_rooks != 0) {
        int sq = lsb(black_rooks);
        int file = sq & 7;

        if ((black_pawns & FILE_MASKS[file]) == 0 && (white_pawns & FILE_MASKS[file]) == 0) {
            black_mg += ROOK_OPEN_FILE_BONUS_MG ;
            black_eg += ROOK_OPEN_FILE_BONUS_EG ;
        } else if ((black_pawns & FILE_MASKS[file]) == 0) {
            black_mg += ROOK_SEMI_OPEN_FILE_BONUS_MG ;
            black_eg += ROOK_SEMI_OPEN_FILE_BONUS_EG ;
        }

        black_rooks &= black_rooks - 1;
    }

    /*King safety*/
    /*White first*/
    int w_king_sq   = lsb(pos.get_pieces(Color::White, PieceType::King));
    int w_king_file = w_king_sq  & 7 ;
    int w_king_rank = w_king_sq >> 3 ;

    int w_shield_score = 0;

    for (int file = w_king_file - 1; file < w_king_file + 2; file++) {
        if (!(file < 0 || file > 7)) {
            
            int w_near_sq = (w_king_rank + 1) * 8 + file ;
            int  w_far_sq = (w_king_rank + 2) * 8 + file ;

            if        ((w_king_rank + 1 <= 7) && test_bit(white_pawns, static_cast<Square>(w_near_sq))) {
                /*Pawns on 3 squares directly in front of king*/
                w_shield_score += PAWN_SHIELD_NEAR;
            } else if ((w_king_rank + 2 <= 7) && test_bit(white_pawns, static_cast<Square>( w_far_sq))) {
                /*Pawns on 3 squares two in front of king, i.e. pushed*/
                w_shield_score += PAWN_SHIELD_FAR;
            } else {
                /*No pawns in 6 squares in front of king*/
                w_shield_score += PAWN_SHIELD_MISSING;
            }
        
        } else { continue; }

        /*King is weaker if on open or semi-open file*/
        if ((white_pawns & FILE_MASKS[file]) == 0 && (black_pawns & FILE_MASKS[file]) == 0) {
            w_shield_score += KING_OPEN_FILE_PENALTY ;
        } else if ((white_pawns & FILE_MASKS[file]) == 0) {
            w_shield_score += KING_SEMI_OPEN_FILE_PENALTY ;
        }
    }

    /*King is also weak if enemy pieces are nearby, especially if coordinated*/    
    Bitboard w_king_zone = KING_ATTACKS[w_king_sq] | (1ULL << w_king_sq);

    int w_attacker_count  = 0 ;
    int w_attacker_weight = 0 ;

    /*Check for each black piece near the white king, starting with Knights*/
    Bitboard b_knights = pos.get_pieces(Color::Black, PieceType::Knight);
    while (b_knights != 0) {
        int sq = lsb(b_knights);

        if ((KNIGHT_ATTACKS[sq] & w_king_zone) != 0) {
            w_attacker_count++;
            w_attacker_weight += KNIGHT_ATTACK_WEIGHT;
        }
        b_knights &= b_knights - 1;
    }
    /*Then bishops*/
    Bitboard b_bishops = pos.get_pieces(Color::Black, PieceType::Bishop);
    while (b_bishops != 0) {
        int sq = lsb(b_bishops);

        if ((get_bishop_attacks(sq, occupied) & w_king_zone) != 0) {
            w_attacker_count++;
            w_attacker_weight += BISHOP_ATTACK_WEIGHT;
        }
        b_bishops &= b_bishops - 1;
    }
    /*Then rooks*/
    Bitboard b_rooks = pos.get_pieces(Color::Black, PieceType::Rook);
    while (b_rooks != 0) {
        int sq = lsb(b_rooks);

        if ((get_rook_attacks(sq, occupied) & w_king_zone) != 0) {
            w_attacker_count++;
            w_attacker_weight += ROOK_ATTACK_WEIGHT;
        }
        b_rooks &= b_rooks - 1;
    }
    /*Then queens*/
    Bitboard b_queens = pos.get_pieces(Color::Black, PieceType::Queen);
    while (b_queens != 0) {
        int sq = lsb(b_queens);
        if (((get_bishop_attacks(sq, occupied) | get_rook_attacks(sq, occupied)) & w_king_zone) != 0) {
            w_attacker_count++;
            w_attacker_weight += QUEEN_ATTACK_WEIGHT;
        }
        b_queens &= b_queens - 1;
    }
    /*Add nonlinear multiplier for multiple attacking pieces*/
    if (w_attacker_count >= 2) {
        int idx = w_attacker_count < 9 ? w_attacker_count : 8;
        w_shield_score -= (w_attacker_weight * SAFETY_TABLE[idx]) / 100;
    }

    white_mg += w_shield_score;

    /*Black next*/
    int b_king_sq   = lsb(pos.get_pieces(Color::Black, PieceType::King));
    int b_king_file = b_king_sq  & 7 ;
    int b_king_rank = b_king_sq >> 3 ;

    int b_shield_score = 0;

    for (int file = b_king_file - 1; file < b_king_file + 2; file++) {
        if (!(file < 0 || file > 7)) {
            
            int b_near_sq = (b_king_rank - 1) * 8 + file ;
            int  b_far_sq = (b_king_rank - 2) * 8 + file ;

            if        ((b_king_rank - 1 >= 0) && test_bit(black_pawns, static_cast<Square>(b_near_sq))) {
                b_shield_score += PAWN_SHIELD_NEAR;
            } else if ((b_king_rank - 2 >= 0) && test_bit(black_pawns, static_cast<Square>( b_far_sq))) {
                b_shield_score += PAWN_SHIELD_FAR;
            } else {
                b_shield_score += PAWN_SHIELD_MISSING;
            }

        } else { continue; }

        if ((black_pawns & FILE_MASKS[file]) == 0 && (white_pawns & FILE_MASKS[file]) == 0) {
            b_shield_score += KING_OPEN_FILE_PENALTY ;
        } else if ((black_pawns & FILE_MASKS[file]) == 0) {
            b_shield_score += KING_SEMI_OPEN_FILE_PENALTY ;
        }

    }

    Bitboard b_king_zone = KING_ATTACKS[b_king_sq] | (1ULL << b_king_sq);

    int b_attacker_count  = 0 ;
    int b_attacker_weight = 0 ;

    Bitboard w_knights = pos.get_pieces(Color::White, PieceType::Knight);
    while (w_knights != 0) {
        int sq = lsb(w_knights);

        if ((KNIGHT_ATTACKS[sq] & b_king_zone) != 0) {
            b_attacker_count++;
            b_attacker_weight += KNIGHT_ATTACK_WEIGHT;
        }
        w_knights &= w_knights - 1;
    }
    Bitboard w_bishops = pos.get_pieces(Color::White, PieceType::Bishop);
    while (w_bishops != 0) {
        int sq = lsb(w_bishops);

        if ((get_bishop_attacks(sq, occupied) & b_king_zone) != 0) {
            b_attacker_count++;
            b_attacker_weight += BISHOP_ATTACK_WEIGHT;
        }
        w_bishops &= w_bishops - 1;
    }
    Bitboard w_rooks = pos.get_pieces(Color::White, PieceType::Rook);
    while (w_rooks != 0) {
        int sq = lsb(w_rooks);

        if ((get_rook_attacks(sq, occupied) & b_king_zone) != 0) {
            b_attacker_count++;
            b_attacker_weight += ROOK_ATTACK_WEIGHT;
        }
        w_rooks &= w_rooks - 1;
    }
    Bitboard w_queens = pos.get_pieces(Color::White, PieceType::Queen);
    while (w_queens != 0) {
        int sq = lsb(w_queens);
        if (((get_bishop_attacks(sq, occupied) | get_rook_attacks(sq, occupied)) & b_king_zone) != 0) {
            b_attacker_count++;
            b_attacker_weight += QUEEN_ATTACK_WEIGHT;
        }
        w_queens &= w_queens - 1;
    }
    if (b_attacker_count >= 2) {
        int idx = b_attacker_count < 9 ? b_attacker_count : 8;
        b_shield_score -= (b_attacker_weight * SAFETY_TABLE[idx]) / 100;
    }
    
    black_mg += b_shield_score;


    /*Blend MG and EG scores*/
    int mg_score = white_mg - black_mg ;
    int eg_score = white_eg - black_eg ;

    /*Tempo bonus for the side that gets to move*/
    if (pos.get_side_to_move() == Color::White) {
        mg_score += TEMPO_BONUS ;
        eg_score += TEMPO_BONUS ;
    } else {
        mg_score -= TEMPO_BONUS ;
        eg_score -= TEMPO_BONUS ;
    }

    /*Limit phase to 24 in case of promotions*/
    if (phase > 24) phase = 24;

    int score = ((mg_score * phase) + (eg_score * (24 - phase))) / 24;

    /*Ensure score swaps for each side for negamax*/
    if (pos.get_side_to_move() == Color::White) {
        return  score ;
    } else {
        return -score ;
    }

}


void init_eval() {
    for (int sq = 0; sq < 64; sq++) {

        int file = sq  & 7 ;
        int rank = sq >> 3 ;

        Bitboard files = FILE_MASKS[file] | ADJACENT_FILES[file];
        Bitboard ranks_ahead_white = 0 ;
        Bitboard ranks_ahead_black = 0 ;

        /*For white check ranks ahead of the pawn*/
        if (rank < 7) {
            ranks_ahead_white = 0xFFFFFFFFFFFFFFFFULL << (8 * (rank + 1));
        } else {
            ranks_ahead_white = 0;
        }

        PASSED_PAWN_MASKS[static_cast<int>(Color::White)][sq] = files & ranks_ahead_white;

        /*For black check ranks behind the pawn*/
        if (rank > 0) {
            ranks_ahead_black = 0xFFFFFFFFFFFFFFFFULL >> (8 * (8 - rank));
        } else {
            ranks_ahead_black = 0;
        }

        PASSED_PAWN_MASKS[static_cast<int>(Color::Black)][sq] = files & ranks_ahead_black;
        
    }
}

