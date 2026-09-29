#pragma once
#include <cstdint>

using Bitboard = uint64_t ;
using Move     = uint16_t ;

/*Defining the two colours*/
enum class Color {
    White,
    Black,
    NUM_COLORS
};

/*Defining general piece type*/
enum class PieceType{
    Pawn,
    Knight,
    Bishop,
    Rook,
    Queen,
    King,
    NUM_PIECE_TYPES
};

/*Defining the squares on the board*/
enum class Square{
    A1,
    B1,
    C1,
    D1,
    E1,
    F1,
    G1,
    H1,
    A2,
    B2,
    C2,
    D2,
    E2,
    F2,
    G2,
    H2,
    A3,
    B3,
    C3,
    D3,
    E3,
    F3,
    G3,
    H3,
    A4,
    B4,
    C4,
    D4,
    E4,
    F4,
    G4,
    H4,
    A5,
    B5,
    C5,
    D5,
    E5,
    F5,
    G5,
    H5,
    A6,
    B6,
    C6,
    D6,
    E6,
    F6,
    G6,
    H6,
    A7,
    B7,
    C7,
    D7,
    E7,
    F7,
    G7,
    H7,
    A8,
    B8,
    C8,
    D8,
    E8,
    F8,
    G8,
    H8,
    NUM_SQUARES
};

/*Defining the pieces by colour & empty squares*/
enum class Piece{
    WhitePawn,
    WhiteKnight,
    WhiteBishop,
    WhiteRook,
    WhiteQueen,
    WhiteKing,
    BlackPawn,
    BlackKnight,
    BlackBishop,
    BlackRook,
    BlackQueen,
    BlackKing,
    NONE,
    NUM_PIECES
};

/*Defining all move types:
    Bits: 15 14 13 12 | 11 10 9 8 7 6 | 5 4 3 2 1 0
          [  flags  ] | [ to square ] | [from square]*/
constexpr Move MOVE_NONE             = 0        ;
constexpr Move MOVE_QUIET            = 0  << 12 ;
constexpr Move MOVE_PAWN_DOUBLE      = 1  << 12 ;
constexpr Move MOVE_KSIDE_CASTLE     = 2  << 12 ;
constexpr Move MOVE_QSIDE_CASTLE     = 3  << 12 ;
constexpr Move MOVE_CAPTURE          = 4  << 12 ;
constexpr Move MOVE_EN_PASSANT_CAP   = 5  << 12 ;
constexpr Move MOVE_KNIGHT_PROMO     = 8  << 12 ;
constexpr Move MOVE_BISHOP_PROMO     = 9  << 12 ;
constexpr Move MOVE_ROOK_PROMO       = 10 << 12 ;
constexpr Move MOVE_QUEEN_PROMO      = 11 << 12 ;
constexpr Move MOVE_KNIGHT_PROMO_CAP = 12 << 12 ;
constexpr Move MOVE_BISHOP_PROMO_CAP = 13 << 12 ;
constexpr Move MOVE_ROOK_PROMO_CAP   = 14 << 12 ;
constexpr Move MOVE_QUEEN_PROMO_CAP  = 15 << 12 ;

/*Function to construct the move*/
constexpr Move create_move(Square from, Square to, Move flags) {
    return static_cast<Move>(static_cast<int>(from))    |
           static_cast<Move>(static_cast<int>(to) << 6) |
           flags;
}

/*Function for extracting the move*/
constexpr Square move_from(Move m) {
    return static_cast<Square>(m & 0x3F);
}
constexpr Square move_to(Move m) {
    return static_cast<Square>((m >> 6) & 0x3F);
}
constexpr Move move_flags(Move m) {
    return static_cast<Move>(m & 0xF000);
}
