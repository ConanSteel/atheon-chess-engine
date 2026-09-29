#include "tt.h"
#include "board.h"


void TT::resize(int mb) {
    /*Convert megabytes to bytes*/
    uint64_t b = static_cast<uint64_t>(mb) * 1024 * 1024;

    num_entries = b / sizeof(TTEntry);
    table.resize(num_entries);

    clear();
}


void TT::clear() {
    std::fill(table.begin(), table.end(), TTEntry{});
}


void TT::store(uint64_t hash, int depth, int score, Flag flag, Move move) {
    uint64_t idx = hash % num_entries;

    table[idx].hash  = hash  ;
    table[idx].depth = depth ;
    table[idx].score = score ;
    table[idx].flag  = flag  ;
    table[idx].move  = move  ;
}


bool TT::probe(uint64_t hash, int depth, int alpha, int beta, int& out_score, Move& out_move) {
    uint64_t idx = hash % num_entries;

    if (table[idx].hash != hash) {
        return false;
    }

    out_move = table[idx].move;
    
    if (table[idx].depth >= depth) {

        if (table[idx].flag == Flag::Exact) {
            
            out_score = table[idx].score;
            return true;

        } else if (table[idx].flag == Flag::Lowerbound) {

            if (table[idx].score >= beta) {
                out_score = table[idx].score;
                return true;
            } else {
                return false;
            }

        } else if (table[idx].flag == Flag::Upperbound) {

            if (table[idx].score <= alpha) {
                out_score = table[idx].score;
                return true;
            } else {
                return false;
            }
        }

    } else {
        
        return false;
    }
}

