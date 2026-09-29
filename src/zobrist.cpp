#include "zobrist.h"
#include "board.h"
#include "bitboard.h"


uint64_t Zobrist::piece_keys[12][64] ;
uint64_t Zobrist::side_key           ;
uint64_t Zobrist::castling_keys[16]  ;
uint64_t Zobrist::ep_keys[8]         ;


/*Any non-zero seed*/
static uint64_t prng_state = 0x123456789ABCDEF;

/*Static helper function to maintain internal state*/
static uint64_t random_u64() {
    prng_state ^= prng_state << 13 ;
    prng_state ^= prng_state >>  7 ;
    prng_state ^= prng_state << 17 ;
    
    return prng_state; 
}

/*Use init to fill every table with random numbers*/
void Zobrist::init() {
    for (int pidx = 0; pidx < 12; pidx++) {
        for (int sq = 0; sq < 64; sq++) {
            piece_keys[pidx][sq] = random_u64();
        }
    }

    side_key = random_u64();

    for (int i = 0; i < 16; i++) {
        castling_keys[i] = random_u64();
    }

    for (int file = 0; file < 8; file++) {
        ep_keys[file] = random_u64();
    }
}

/*Build each hash from scratch by scanning the position*/
uint64_t Zobrist::compute_hash(const Position& pos) {
    uint64_t hash = 0;

    /*Loop through each of the two colours*/
    for (int c = 0; c < 2; c++) {
        /*Then through each piecetype*/
        for (int p = 0; p < 6; p++) {

            Bitboard bb = pos.get_pieces(static_cast<Color>(c), static_cast<PieceType>(p));

            while (bb != 0) {
                int sq = lsb(bb) ;      /*Pop the least significant bit*/
                bb &= bb - 1     ;      /*Clear that bit               */

                int pidx = (c * 6) + p;
                hash ^= piece_keys[pidx][sq];
            }
        }
    }

    /*Side to move only if XOR is black*/
    if (pos.get_side_to_move() == Color::Black) {
        hash ^= side_key;
    }

    /*Castling rights*/
    hash ^= castling_keys[pos.get_castling_rights()];

    /*En passant (if active)*/
    Square ep_square = pos.get_ep_square();
    if (ep_square != Square::NUM_SQUARES) {
        int file = static_cast<int>(ep_square) % 8;
        hash ^= ep_keys[file];
    }

    return hash;
}
