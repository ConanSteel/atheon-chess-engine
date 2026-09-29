#include "search.h"
#include "eval.h"
#include "movegen.h"
#include "attacks.h"
#include "bitboard.h"
#include "perft.h"
#include <chrono>
#include <cstring>
#include <iostream>


/*Force stops a depth search if too much time is elapsing*/
bool search_stopped = false;
int node_count = 0;
auto search_start = std::chrono::steady_clock::now();
int search_time_limit = 0;

TT tt;
/*If quiet move causes beta cutoff, it might also be useful*/
/*in other positions, therefore it is good to save two plys*/
Move killers[MAX_PLY][2] = {} ;
/*Also track how often quiet moves cause beta cutoffs across the game*/
int history[2][64][64]   = {} ;

/*Used for storing the engine's "main line" for debugging*/
Move pv[MAX_PLY][MAX_PLY] ;
int pv_length[MAX_PLY]    ;


void check_time() {
    if (search_time_limit <= 0) return;
    int elapsed = static_cast<int>(std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now() - search_start).count());
    if (elapsed >= search_time_limit) search_stopped = true;
}


int negamax(Position& pos, int depth, int ply, int alpha, int beta, bool do_null) {
    /*Base case: reached target depth, ask eval how the position looks*/

    pv_length[ply] = ply;

    /*Too much time failsafe*/
    node_count++;
    if (node_count % 2048 == 0) check_time();
    if (search_stopped) return 0;

    if (depth == 0) {
        return quiescence(pos, ply, alpha, beta);
    }

    /*Probe the position using tt*/
    int tt_score;
    Move tt_move = MOVE_NONE;
    if (tt.probe(pos.hash, depth, alpha, beta, tt_score, tt_move)) { return tt_score; }

    /*Null pruning*/
    int king_sq   = lsb(pos.get_pieces(pos.get_side_to_move(), PieceType::King));
    Color enemy   = (pos.get_side_to_move() == Color::White) ? Color::Black : Color::White;
    bool in_check = is_square_attacked(pos, king_sq, enemy);

    /*Add depth for forced check sequences*/
    if (in_check) depth++;

    if(do_null && (depth >= 3) && !in_check) {
        /*Check side to move has at least one non-pawn piece*/
        Color us = pos.get_side_to_move();
        Bitboard non_pawn = pos.get_pieces(us, PieceType::Knight )
                          | pos.get_pieces(us, PieceType::Bishop )
                          | pos.get_pieces(us, PieceType::Rook   )
                          | pos.get_pieces(us, PieceType::Queen  ) ;

        if (non_pawn != 0) {
            UndoInfo undo;
            
            pos.null_make_move(undo);
            int score = -negamax(pos, depth -3, ply + 1, -beta, -beta + 1, false);
            pos.null_unmake_move(undo);

            if (score >= beta) {
                return beta;
            }
        }
    }

    /*Save original alpha for flag determination later on*/
    int original_alpha = alpha;

    /*Track the best move*/
    Move best_move = MOVE_NONE;

    Move moves[256];   
    int move_count = generate_legal_moves(pos, moves);
    
    /*No legal moves means either checkmate or stalemate*/
    if (move_count == 0) {
        /*Find king square to check for checkmate*/
        int king_sq = lsb(pos.get_pieces(pos.get_side_to_move(), PieceType::King));
        Color enemy = (pos.get_side_to_move() == Color::White) ? Color::Black : Color::White;

        if (is_square_attacked(pos, king_sq, enemy)) {
            /*Prefer quicker checkmates*/
            return NEGATIVE_INFINITY + ply;
        } else {
            return 0;
        }
    }

    /*Prioritise TT moves*/
    if (tt_move != MOVE_NONE) {
        for (int i = 0; i < move_count; i++) {
            if (moves[i] == tt_move) {
                std::swap(moves[0], moves[i]);
                break;
            }
        }
    }

    /*MVV-LVA calculations*/
    int scores[256];

    for (int i = 0; i < move_count; i++) {
        Square from_sq = move_from(moves[i]) ;
        Square to_sq   =   move_to(moves[i]) ;

        /*Priority score for TT moves*/
        if (moves[i] == tt_move) { 
            
            scores[i] = 10000; 
        
        } else if (move_flags(moves[i]) & MOVE_CAPTURE) {
        
            int victim   = static_cast<int>(pos.get_mailbox(static_cast<int>( to_sq ))) ;
            int attacker = static_cast<int>(pos.get_mailbox(static_cast<int>(from_sq))) ;

            scores[i] = (PIECE_VALUES[victim] * 10) - PIECE_VALUES[attacker];
        
        } else if (moves[i] == killers[ply][0] || moves[i] == killers[ply][1]) {
        
            scores[i] = 900;
        
        } else {

            int side = static_cast<int>(pos.get_side_to_move()) ;
            int from = static_cast<int>(   move_from(moves[i])) ;
            int  to  = static_cast<int>(     move_to(moves[i])) ;

            scores[i] = history[side][from][to];

        }
    }

    for (int i = 0; i < move_count; i++) {
        /*Find highest ranking move from i onward*/
        int best_idx = i;
        for (int j = (i + 1); j < move_count; j++) {
            if (scores[j] > scores[best_idx]) {
                best_idx = j;
            }
        }

        std::swap(moves[i] , moves[best_idx]  ) ;
        std::swap(scores[i], scores[best_idx] ) ;
    
        Move move = moves[i];
        UndoInfo undo{};

        pos.make_move(move, undo);
        
        /*Check for possible depth reductions*/
        int score;
        bool need_full_search = true;
        
        if ((i >= 3) && (depth >= 3) && !(move_flags(move) & MOVE_CAPTURE) && !in_check) {
            score = -negamax(pos, depth - 2, ply + 1, -beta, -alpha, true);
            if (score <= alpha) {
                need_full_search = false;
            } 
        }
        if (need_full_search) {
            if (i == 0) {
                /*First move always has full window and depth*/
                score = -negamax(pos, depth - 1, ply + 1, -beta, -alpha, true);
            } else {
                /*Zero window search*/
                score = -negamax(pos, depth - 1, ply + 1, -alpha - 1, -alpha, true);
                if ((score > alpha) && (score < beta)) {
                    score = -negamax(pos, depth - 1, ply + 1, -beta, -alpha, true);
                }
            }
        }

        pos.unmake_move(move, undo);        
        
        if (score > alpha) {
            alpha = score;
            best_move = move;

            /*Store this move as the start of the PV at this ply*/
            pv[ply][ply] = move;

            /*Copy child's PV after it*/
            for (int j = (ply + 1); j < pv_length[ply + 1]; j++) {
                pv[ply][j] = pv[ply + 1][j];
            }

            pv_length[ply] = pv_length[ply + 1];
        }
        if (alpha >= beta) {
            /*Killer condition*/
            if (!(move_flags(move) & MOVE_CAPTURE)) {
                killers[ply][1] = killers[ply][0];
                killers[ply][0] = move;
            
                int side = static_cast<int>(pos.get_side_to_move()) ;
                int from = static_cast<int>(    move_from(move)   ) ;
                int  to  = static_cast<int>(      move_to(move)   ) ;

                history[side][from][to] += depth * depth;
            }
            break;
        }
    }

    /*TT store*/
    Flag flag;
    if (alpha <= original_alpha) {
        flag = Flag::Upperbound;
    } else if (alpha >= beta) {
        flag = Flag::Lowerbound;
    } else {
        flag = Flag::Exact;
    }

    tt.store(pos.hash, depth, alpha, flag, best_move);

    return alpha;
}


int quiescence(Position& pos, int ply, int alpha, int beta) {
    
    pv_length[ply] = ply;

    if (search_stopped) return evaluate(pos);
    
    int stand_pat = evaluate(pos);
    Move moves[256];

    /*Safety check so the engine doesn't get stuck in capture chains early on*/
    if (ply > 64) return evaluate(pos);

    if (stand_pat >= beta) {
        return beta;
    }
    if (stand_pat > alpha) {
        alpha = stand_pat;
    }

    int capture_num = generate_captures(pos, moves);
    
    for (int i = 0; i < capture_num; i++) {
        Move move = moves[i];
        UndoInfo undo{};

        pos.make_move(move, undo);
        int score = -quiescence(pos, ply + 1, -beta, -alpha);
        pos.unmake_move(move, undo);
        
        if (score > alpha) {
            alpha = score;
        }
        if (alpha >= beta) {
            break;
        }
    }

    return alpha;
}


SearchResult search_root(Position& pos, int depth, int alpha, int beta) {
    SearchResult result;
    result.best_move = MOVE_NONE;
    
    Move moves[256];   
    int move_count = generate_legal_moves(pos, moves);

    /*TT probe here as well*/
    int tt_score;
    Move tt_move = MOVE_NONE;
    tt.probe(pos.hash, depth, alpha, beta, tt_score, tt_move);

    /*Same scoring loop and selection sort as the one in negamax*/
    /*starting again with the MVV-LVA calculations              */
    int scores[256];

    pv_length[0] = 0;

    for (int i = 0; i < move_count; i++) {
        Square from_sq = move_from(moves[i]) ;
        Square to_sq   =   move_to(moves[i]) ;

        /*Priority score for TT moves*/
        if (moves[i] == tt_move) { 
            
            scores[i] = 10000; 
        
        } else if (move_flags(moves[i]) & MOVE_CAPTURE) {
        
            int victim   = static_cast<int>(pos.get_mailbox(static_cast<int>( to_sq ))) ;
            int attacker = static_cast<int>(pos.get_mailbox(static_cast<int>(from_sq))) ;

            scores[i] = (PIECE_VALUES[victim] * 10) - PIECE_VALUES[attacker];
        
        } else if (moves[i] == killers[0][0] || moves[i] == killers[0][1]) {
        
            scores[i] = 900;
        
        } else {

            int side = static_cast<int>(pos.get_side_to_move()) ;
            int from = static_cast<int>(   move_from(moves[i])) ;
            int  to  = static_cast<int>(     move_to(moves[i])) ;

            scores[i] = history[side][from][to];

        }
    }

    for (int i = 0; i < move_count; i++) {
        /*Find highest ranking move from i onward*/
        int best_idx = i;
        for (int j = (i + 1); j < move_count; j++) {
            if (scores[j] > scores[best_idx]) {
                best_idx = j;
            }
        }

        std::swap(moves[i] , moves[best_idx]  ) ;
        std::swap(scores[i], scores[best_idx] ) ;

        Move move = moves[i];
        UndoInfo undo{};

        pos.make_move(move, undo);
        int score = -negamax(pos, depth - 1, 1, -beta, -alpha, true);
        pos.unmake_move(move, undo);        
        
        if (score > alpha) {
            alpha = score;
            result.best_move = move;

            /*Store this move as PV start at ply 0*/
            pv[0][0] = move;

            /*Copy child's PV*/
            for (int k = 1; k < pv_length[1]; k++) {
                pv[0][k] = pv[1][k];
            }

            pv_length[0] = pv_length[1];

            /*Copy into result so iterative_deepening can read it*/
            result.pv_length = pv_length[0];
            for (int l = 0; l < result.pv_length; l++) {
                result.pv[l] = pv[0][l];
            }
        }
        if (alpha >= beta) {
            /*Killer condition*/
            if (!(move_flags(move) & MOVE_CAPTURE)) {
                killers[0][1] = killers[0][0];
                killers[0][0] = move;
            
                int side = static_cast<int>(pos.get_side_to_move()) ;
                int from = static_cast<int>(    move_from(move)   ) ;
                int  to  = static_cast<int>(      move_to(move)   ) ;

                history[side][from][to] += depth * depth;
            }
            break;
        }
    }

    result.score = alpha;
    return result;   
}


SearchResult iterative_deepening(Position& pos, int depth, int time_limit_ms) {
    /*Start by clearing killer and history arrays*/
    memset(killers, 0, sizeof(killers)) ;
    memset(history, 0, sizeof(history)) ;

    /*Reset the node count*/
    node_count = 0;

    auto start = std::chrono::steady_clock::now();
    SearchResult result               ;
    SearchResult best_result          ;
    best_result.best_move = MOVE_NONE ;
    
    search_start = std::chrono::steady_clock::now();
    search_time_limit = time_limit_ms ;
    search_stopped = false            ;

    int alpha ;
    int beta  ;
    int window     = 50 ; 
    int prev_score = 0  ;

    for (int d = 1; d <= depth; d++) {
        search_stopped = false;

        if (d == 1) {
            alpha =  NEGATIVE_INFINITY ;
            beta  = -NEGATIVE_INFINITY ;
        } else {
            alpha = prev_score - window ;
            beta  = prev_score + window ;
        }

        result = search_root(pos, d, alpha, beta);

        if (search_stopped) break;

        if ((result.score <= alpha) || (result.score >= beta)) {
            result = search_root(pos, d, NEGATIVE_INFINITY, -NEGATIVE_INFINITY);
            if (search_stopped) break;
        } 

        best_result = result       ;
        prev_score  = result.score ;

        auto now = std::chrono::steady_clock::now();
        int elapsed = static_cast<int>(std::chrono::duration_cast<std::chrono::milliseconds>(
            now - search_start).count());
        int nps = (elapsed > 0) ? (node_count * 1000 / elapsed) : 0;

        /*Build PV string*/
        std::cout << "info depth " << d 
                  << " score cp "  << result.score
                  << " nodes "     << node_count 
                  << " nps "       << nps
                  << " time "      << elapsed
                  << " pv"         ;
        
        for (int i = 0; i < result.pv_length; i++) {
            std::cout << " " << move_to_string(result.pv[i]);
        }
        std::cout << std::endl;

        /*Calculate elapsed time*/
        if (time_limit_ms > 0 && elapsed >= time_limit_ms) break ;
    }

    search_stopped = false ;
    return best_result     ;
}
