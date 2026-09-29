#pragma once
#include "types.h"

void print_bitboard(Bitboard bb);
void set_bit(Bitboard& bb, Square sq);
void clear_bit(Bitboard& bb, Square sq);
bool test_bit(Bitboard bb, Square sq);

int popcount(Bitboard bb);
int lsb(Bitboard bb);
