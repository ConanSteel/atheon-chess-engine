#pragma once
#include "board.h"
#include "tt.h"

constexpr int NEGATIVE_INFINITY = -1000000;
constexpr int MAX_PLY = 64;

struct SearchResult {
    int score         ;
    Move best_move    ;
    Move pv[MAX_PLY]  ;
    int pv_length = 0 ;
};

int negamax(Position& pos, int depth, int ply, int alpha, int beta, bool do_null);
int quiescence(Position& pos, int ply, int alpha, int beta);
SearchResult search_root(Position& pos, int depth, int alpha, int beta);
SearchResult iterative_deepening(Position& pos, int depth, int time_limit_ms);

extern bool search_stopped;
extern TT tt;
extern Move pv[MAX_PLY][MAX_PLY] ;
extern int pv_length[MAX_PLY]    ;