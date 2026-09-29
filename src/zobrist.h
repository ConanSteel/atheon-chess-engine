#pragma once
#include <cstdint>

class Position;

namespace Zobrist {

    /*12 piece-colour combos * 64 squares*/
    extern uint64_t piece_keys[12][64];

    /*XOR on every move to flip side-to-move*/
    extern uint64_t side_key;

    /*Castling rights are 4-bit so need 16 entries*/
    extern uint64_t castling_keys[16];

    /*8 en passant entries, one per file*/
    extern uint64_t ep_keys[8];

    /*Call at startup*/
    void init();

    /*Compute hash by scanning full position used by FEN parser*/
    uint64_t compute_hash(const Position& pos);

}
