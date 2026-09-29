#include "perft.h"
#include "board.h"
#include "movegen.h"
#include "types.h"
#include <iostream>

/*Need to convert our current square notation to UCI long algebraic notation*/
std::string square_to_string(Square sq) {
    char file = 'a' + (static_cast<int>(sq) % 8);
    char rank = '1' + (static_cast<int>(sq) / 8);
    return std::string{file, rank};
}

/*Need to combine from and to squares and append any promotions*/
std::string move_to_string(Move m) {
    std::string s = square_to_string(move_from(m))
                  + square_to_string( move_to(m) ) ;

    Move flags = move_flags(m);
    if (flags & (8 << 12)) {
        const char promo_chars[] = { 'n', 'b', 'r', 'q' };
        s += promo_chars[(flags >> 12) & 0x3];
    }

    return s;
}

/*Calculate how many moves exist after each side makes a move*/
uint64_t perft(Position& pos, int depth) {
    if (depth == 0)
        return 1;

    Move moves[256];
    int count = generate_legal_moves(pos, moves);
    UndoInfo undo;
    
    uint64_t nodes = 0;
    for (int i = 0; i < count; ++i) {
        pos.make_move(moves[i], undo)   ;
        nodes += perft(pos, depth - 1)  ;
        pos.unmake_move(moves[i], undo) ;
    }

    return nodes;
}

/*Use divide to debug node total against known-correct engine output*/
void divide(Position& pos, int depth) {
    Move moves[256];
    int count = generate_legal_moves(pos, moves);
    UndoInfo undo;

    uint64_t total = 0;
    for (int i = 0; i < count; ++i) {
        pos.make_move(moves[i], undo)          ;
        uint64_t nodes = perft(pos, depth - 1) ;
        pos.unmake_move(moves[i], undo)        ;

        std::cout << move_to_string(moves[i])
                  << ": " << nodes << "\n";
        total += nodes;
    }

    std::cout << "Total: " << total << "\n";
}
