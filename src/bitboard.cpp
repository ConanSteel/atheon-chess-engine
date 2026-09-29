#include <iostream>
#include "bitboard.h"
#include <intrin.h>

void print_bitboard(Bitboard bb) {
    for (int rank = 7; rank >= 0; rank--) {
        for (int file = 0; file < 8; file++) {
            int square = rank * 8 + file;
            if (bb & (1ULL << square)) {
                std::cout << "1";
            } else {
                std::cout << "0";
            }
        }
        std::cout << "\n";
    }
    std::cout << "\n";
}

void set_bit(Bitboard& bb, Square sq) {
    bb |= (1ULL << static_cast<int>(sq));
}

void clear_bit(Bitboard& bb, Square sq) {
    bb &= ~(1ULL << static_cast<int>(sq));
}

bool test_bit(Bitboard bb, Square sq) {
    return (bb & (1ULL << static_cast<int>(sq)));
}

int popcount(Bitboard bb) {
    return static_cast<int>(__popcnt64(bb));
}

int lsb(Bitboard bb) {
    unsigned long index;
    _BitScanForward64(&index, bb);
    return static_cast<int>(index);
}
