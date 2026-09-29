#pragma once
#include <string>
#include "board.h"
#include "types.h"

/*Convert UCI move string into a Move by matching against legal moves*/
Move parse_move(Position& pos, const std::string& str);

/*Read stdin, send command, write response*/
void uci_loop();
