#pragma once
#include "types.h"
#include "bitboard.h"
#include "board.h"

/*Bitboard constants*/
constexpr Bitboard NOT_A_FILE  = ~0x0101010101010101ULL ;
constexpr Bitboard NOT_H_FILE  = ~0x8080808080808080ULL ;
constexpr Bitboard NOT_AB_FILE = ~0x0303030303030303ULL ;
constexpr Bitboard NOT_GH_FILE = ~0xC0C0C0C0C0C0C0C0ULL ;

/*Attack tables that will be defined in attacks.cpp*/
extern Bitboard KNIGHT_ATTACKS[64]  ;
extern Bitboard KING_ATTACKS[64]    ;
extern Bitboard PAWN_ATTACKS[2][64] ;

/*Masks are needed for the bishops and rooks*/
extern Bitboard BISHOP_MASKS[64] ;
extern Bitboard ROOK_MASKS[64]   ;

/*Attacks for bishops and rooks*/
extern Bitboard bishop_attacks_classical(int square, Bitboard blockers) ;
extern Bitboard rook_attacks_classical(int square, Bitboard blockers)   ;

/*Public lookup functions*/
extern Bitboard get_bishop_attacks(int square, Bitboard occupied) ;
extern Bitboard get_rook_attacks(int square, Bitboard occupied)   ;

/*Legal castling and king moves*/
bool is_square_attacked(const Position& pos, int square, Color attacker_color);

/*Call once from main() before using tables*/
void init_attack_tables();
