#include "attacks.h"
#include <random>
#include <vector>

/* -------------------- */
/* VARIABLES TO DECLARE */
/* -------------------- */

/*1D array for knights and kings because they attack the same on each side*/
Bitboard KNIGHT_ATTACKS[64]    ;
Bitboard KING_ATTACKS[64]      ;
/*2D because white and black pawns move in opposite directions*/
Bitboard PAWN_ATTACKS[2][64]   ;
/*Masks for sliding pieces to remove edge tiles*/
Bitboard BISHOP_MASKS[64]      ;
Bitboard ROOK_MASKS[64]        ;
/*Attack lookup tables*/
Bitboard BISHOP_TABLE[64][512] ;
Bitboard ROOK_TABLE[64][4096]  ;

/*Key formulae:
    index   = (blockers & mask) * magic >> shift
    attacks = table[index]
*/
struct MagicEntry {
    Bitboard mask  ;
    Bitboard magic ;
    int shift;
};
MagicEntry BISHOP_MAGICS[64]   ;
MagicEntry ROOK_MAGICS[64]     ;

/* ----------------------- */
/* KNIGHTS, KINGS, & PAWNS */
/* ----------------------- */

/*Takes in a square and returns which squares the knight attacks*/
Bitboard compute_knight_attacks(Square sq) {
    Bitboard bb = 1ULL << static_cast<int>(sq);
    Bitboard attacks = 0ULL;

    attacks |= (bb << 17) & NOT_A_FILE  ;    /*Up   2, Right 1*/
    attacks |= (bb << 15) & NOT_H_FILE  ;    /*Up   2, Left  1*/
    attacks |= (bb << 10) & NOT_AB_FILE ;    /*Up   1, Right 2*/
    attacks |= (bb << 6 ) & NOT_GH_FILE ;    /*Up   1, Left  2*/
    attacks |= (bb >> 17) & NOT_H_FILE  ;    /*Down 2, Right 1*/
    attacks |= (bb >> 15) & NOT_A_FILE  ;    /*Down 2, Left  1*/
    attacks |= (bb >> 10) & NOT_GH_FILE ;    /*Down 1, Right 2*/
    attacks |= (bb >> 6 ) & NOT_AB_FILE ;    /*Down 1, Left  2*/
    
    return attacks;
}

/*Takes in a square and returns which squares the king attacks*/
Bitboard compute_king_attacks(Square sq) {
    Bitboard bb = 1ULL << static_cast<int>(sq);
    Bitboard attacks = 0ULL;

    attacks |= (bb << 8)              ;      /*Up        */
    attacks |= (bb >> 8)              ;      /*Down      */
    attacks |= (bb << 1) & NOT_A_FILE ;      /*Right     */
    attacks |= (bb >> 1) & NOT_H_FILE ;      /*Left      */
    attacks |= (bb << 9) & NOT_A_FILE ;      /*Up-Right  */
    attacks |= (bb << 7) & NOT_H_FILE ;      /*Up-Left   */
    attacks |= (bb >> 7) & NOT_A_FILE ;      /*Down-Right*/
    attacks |= (bb >> 9) & NOT_H_FILE ;      /*Down-Left */

    return attacks;
}

/*Takes in a square and returns which squares the pawn attacks*/
Bitboard compute_pawn_attacks(Square sq, Color color) {
    Bitboard bb = 1ULL << static_cast<int>(sq);
    Bitboard attacks = 0ULL;

    if (color == Color::White) {
        attacks |= (bb << 9) & NOT_A_FILE ;      /*Up-Right  */
        attacks |= (bb << 7) & NOT_H_FILE ;      /*Up-Left   */ 
    } else if (color == Color::Black) {
        attacks |= (bb >> 7) & NOT_A_FILE ;      /*Down-Right*/
        attacks |= (bb >> 9) & NOT_H_FILE ;      /*Down-Left */
    }

    return attacks;
}

/* --------------- */
/* BISHOPS & ROOKS */
/* --------------- */

/*Uses namespace so nothing outside the file can call this function*/
namespace {
    
    /*Fixed seed for repeated debugging runs*/
    std::mt19937_64 rng(12345);

    Bitboard sparse_random() {
        return rng() & rng() & rng();
    }

    Bitboard slide_ray(int square, int rank_delta, int file_delta) {
        Bitboard ray = 0;
        /*Temporary rank and file coordinates for the sliding ray*/
        int r = square / 8;
        int f = square % 8;

        r += rank_delta;
        f += file_delta;

        /*Stood on (r, f), "Am I still on the board if I take one more step?", yes = safe, no = edge*/
        while (r + rank_delta >= 0 && r + rank_delta <= 7 &&
               f + file_delta >= 0 && f + file_delta <= 7) {
            
            ray |= (1ULL << (r * 8 + f));
            r += rank_delta;
            f += file_delta;

        }

        return ray;
    }

    /*Different from the ray because it includes blocked squares as captures to have colours determined later*/
    Bitboard slide_attacks(int square, int rank_delta, int file_delta, Bitboard blockers) {
        Bitboard attacks = 0;
        int r = square / 8;
        int f = square % 8;

        r += rank_delta;
        f += file_delta;

        while (r >= 0 && r <= 7 && f >= 0 && f <= 7) {
            Bitboard sq_bb = 1ULL << (r * 8 + f);
            attacks |= sq_bb;
            /*Loop finishes when a piece is hit, allow for capture but no further*/
            if (sq_bb & blockers) break;
            r += rank_delta;
            f += file_delta;
        }   

        return attacks;
    }

    /*Calculates the 128 magic numbers on startup to power every sliding piece each game*/
    Bitboard find_magic(int square, bool is_bishop) {
        /*Grab masks and compute the shift*/
        Bitboard mask = is_bishop ? BISHOP_MASKS[square] : ROOK_MASKS[square];
        /*Number of relevant bits*/
        int n = popcount(mask)  ;
        int shift = 64 - n      ;
        /*2^n possible blocker configurations*/
        int table_size = 1 << n ;

        /*Enumerate all blocker subsets and compute the correct attacks for each one*/
        std::vector<Bitboard> blockers(table_size) ;
        std::vector<Bitboard>  attacks(table_size) ;

        Bitboard subset = 0;
        for (int i = 0; i < table_size; ++i) {
            /*Two arrays needed to calculate correct attacks*/
            blockers[i] = subset;
            attacks[i]  = is_bishop ? bishop_attacks_classical(square, subset) :
                                        rook_attacks_classical(square, subset) ;
            /*Carry-Rippler*/
            subset = (subset - mask) & mask;
        }

        /*Loop through random candidates until correct attack is found*/
        while (true) {
            Bitboard candidate = sparse_random();
            /*Immediately reject cases where the magic does not map the mask's bits into the top bits*/
            if (popcount((mask * candidate) & 0xFF00000000000000ULL) < 6) continue;
            /*Try the candidate against blocker config (0ULL safe to use as sentinel as 0 can never result from an actual attack)*/
            std::vector<Bitboard> table(table_size, 0ULL);
            bool failed = false;

            /*If the index is already filled with a different attack, the candidate fails*/
            for (int i = 0; i < table_size; ++i) {
                int index = static_cast<int>((blockers[i] * candidate) >> shift);
                if (table[index] == 0ULL) {
                    table[index] = attacks[i];
                } else if (table[index] != attacks[i]) {
                    failed = true;
                    break;
                }
            }

            if (!failed) return candidate;
        }
    }

}

/*Computes bishop and rook attacks for initialisation to fill magic lookup tables*/
Bitboard bishop_attacks_classical(int square, Bitboard blockers) {
    return slide_attacks(square,  1,  1, blockers) | 
           slide_attacks(square,  1, -1, blockers) | 
           slide_attacks(square, -1,  1, blockers) | 
           slide_attacks(square, -1, -1, blockers) ;
}
Bitboard rook_attacks_classical(int square, Bitboard blockers) {
    return slide_attacks(square,  1,  0, blockers) | 
           slide_attacks(square,  0,  1, blockers) | 
           slide_attacks(square, -1,  0, blockers) | 
           slide_attacks(square,  0, -1, blockers) ;
}

/*Public lookup functions*/
Bitboard get_bishop_attacks(int square, Bitboard occupied) {
    int index = static_cast<int>((occupied & BISHOP_MAGICS[square].mask) * BISHOP_MAGICS[square].magic >> BISHOP_MAGICS[square].shift);
    return BISHOP_TABLE[square][index];
}
Bitboard get_rook_attacks(int square, Bitboard occupied) {
    int index = static_cast<int>((occupied & ROOK_MAGICS[square].mask) * ROOK_MAGICS[square].magic >> ROOK_MAGICS[square].shift);
    return ROOK_TABLE[square][index];
}

/*------------------*/
/* ATTACKED SQUARES */
/*------------------*/

/*Need to check attacked square for legal castling and king moves later*/
bool is_square_attacked(const Position& pos, int square, Color attacker_color) {
    Color defender_color = (attacker_color == Color::White) ? Color::Black : Color::White;

    /*Check knights*/
    if (KNIGHT_ATTACKS[square] & pos.get_pieces(attacker_color, PieceType::Knight))
        return true;
    /*Check kings*/
    if (KING_ATTACKS[square] & pos.get_pieces(attacker_color, PieceType::King))
        return true;
    /*Check pawns*/
    if (PAWN_ATTACKS[static_cast<int>(defender_color)][square] & pos.get_pieces(attacker_color, PieceType::Pawn))
        return true;

    /*Sliding pieces*/
    Bitboard occupied     = pos.get_all_pieces(Color::White) | 
                            pos.get_all_pieces(Color::Black) ;
    Bitboard bishop_queen = pos.get_pieces(attacker_color, PieceType::Bishop) |
                            pos.get_pieces(attacker_color, PieceType::Queen ) ;
    Bitboard rook_queen   = pos.get_pieces(attacker_color, PieceType::Rook  ) |
                            pos.get_pieces(attacker_color, PieceType::Queen ) ;

    /*Check bishops & queen diagonals*/
    if (get_bishop_attacks(square, occupied) & bishop_queen)
        return true;
    /*Check rooks & queen orthogonals*/
    if (get_rook_attacks(square, occupied) & rook_queen)
        return true;

    return false;
}

/* -------------- */
/* INITIALISATION */
/* -------------- */

void init_slider_masks() {
    for (int sq = 0; sq < 64; ++sq) {
        BISHOP_MASKS[sq] = slide_ray(sq,  1, 1) | slide_ray(sq,  1, -1) |
                           slide_ray(sq, -1, 1) | slide_ray(sq, -1, -1) ;
        
        ROOK_MASKS[sq]   = slide_ray(sq,  1, 0) | slide_ray(sq, 0,  1) |
                           slide_ray(sq, -1, 0) | slide_ray(sq, 0, -1) ;
    }
}

/*Fills in the MagicEntry and then the actual attack lookup table*/
void init_magic_tables() {
    for (int sq = 0; sq < 64; ++sq) {
        /*Bishop*/
        BISHOP_MAGICS[sq].mask  = BISHOP_MASKS[sq]                ;
        BISHOP_MAGICS[sq].shift = 64 - popcount(BISHOP_MASKS[sq]) ; 
        BISHOP_MAGICS[sq].magic = find_magic(sq, true)            ;

        /*Now the BISHOP_TABLE[sq] is filled using found magic*/
        Bitboard mask   = BISHOP_MAGICS[sq].mask;
        Bitboard subset = 0;
        do {
            int index = static_cast<int>((subset * BISHOP_MAGICS[sq].magic) >> BISHOP_MAGICS[sq].shift);
            BISHOP_TABLE[sq][index] = bishop_attacks_classical(sq, subset);
            subset = (subset - mask) & mask;
        } while (subset != 0);

        /*Rook*/
        ROOK_MAGICS[sq].mask  = ROOK_MASKS[sq]                ;
        ROOK_MAGICS[sq].shift = 64 - popcount(ROOK_MASKS[sq]) ; 
        ROOK_MAGICS[sq].magic = find_magic(sq, false)         ;

        /*Now the ROOK_TABLE[sq] is filled using found magic (not redeclaring the same variable)*/
        mask   = ROOK_MAGICS[sq].mask;
        subset = 0;
        do {
            int index = static_cast<int>((subset * ROOK_MAGICS[sq].magic) >> ROOK_MAGICS[sq].shift);
            ROOK_TABLE[sq][index] = rook_attacks_classical(sq, subset);
            subset = (subset - mask) & mask;
        } while (subset != 0);

    }
}

/*Initialising the attack tables*/
void init_attack_tables() {
    /*Loop through every square on the board*/
    for (int sq = 0; sq < 64; ++sq) {
        /*Need to convert from int to Square enum class*/
        KNIGHT_ATTACKS[sq] = compute_knight_attacks(static_cast<Square>(sq)) ;
        KING_ATTACKS[sq]   = compute_king_attacks(static_cast<Square>(sq))   ;
        
        PAWN_ATTACKS[static_cast<int>(Color::White)][sq] = compute_pawn_attacks(static_cast<Square>(sq), Color::White);
        PAWN_ATTACKS[static_cast<int>(Color::Black)][sq] = compute_pawn_attacks(static_cast<Square>(sq), Color::Black);
    }

    init_slider_masks();
    init_magic_tables();
}
