#pragma once
#include "types.h"
#include "board.h"

int generate_moves(const Position& pos, Move* move_list);
int generate_legal_moves(Position& pos, Move* move_list);
int generate_captures(Position& pos, Move* move_list);
