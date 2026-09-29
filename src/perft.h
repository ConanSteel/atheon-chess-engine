#pragma once
#include "types.h"
#include <cstdint>
#include <string>

class Position;

uint64_t perft(Position& pos, int depth);
void divide(Position& pos, int depth);
std::string move_to_string(Move m);
