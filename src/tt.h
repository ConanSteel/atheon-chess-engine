#pragma once
#include "types.h"
#include <cstdint>
#include <vector>


enum class Flag : uint8_t {
    None,
    Exact,
    Lowerbound,
    Upperbound
};

struct TTEntry {
    uint64_t hash  = 0          ;
    int      depth = 0          ;
    int      score = 0          ;
    Flag     flag  = Flag::None ;
    Move     move  = 0          ;
};

class TT {
    private:
        std::vector<TTEntry> table ;
        uint64_t num_entries = 0   ;

    public:
        /*Resize the table for a given size in megabytes*/
        void resize(int mb);

        /*Clear all entries*/
        void clear();

        /*Store hash, depth, score, flag, and best move*/
        void store(uint64_t hash, int depth, int score, Flag flag, Move move);

        /*Probe the table for true if usable entry found*/
        bool probe(uint64_t hash, int depth, int alpha, int beta, int& out_score, Move& out_move);
};
