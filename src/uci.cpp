#include "uci.h"
#include "movegen.h"
#include "perft.h"
#include "search.h"
#include <iostream>
#include <sstream>
#include <cstdlib>

/*UCI move strings*/
Move parse_move(Position& pos, const std::string& str) {
    Move moves[256];
    int count = generate_legal_moves(pos, moves);
    
    /*Loop through every legal move and convert to UCI*/
    for (int i = 0; i < count; ++i) {
        if (move_to_string(moves[i]) == str) {
            return moves[i];
        }
    }

    /*Illegal move or malformed string so return the sentinel*/
    return MOVE_NONE;
}

/*Engine main event loop*/
void uci_loop() {
    std::string line    ;
    std::string command ;
    Position pos        ;

    while (std::getline(std::cin, line)) {
        std::istringstream iss(line);
        iss >> command;

        if (command == "uci")               {
        
            std::cout << "id name Atheon"  << std::endl ;
            std::cout << "id author Conan" << std::endl ;
            std::cout << "uciok"           << std::endl ; 
        
        } else if (command == "isready")    {
        
            std::cout << "readyok"         << std::endl ;
        
        } else if (command == "ucinewgame") {
        
            pos.set("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
            tt.clear();
        
        } else if (command == "position")   {
        
            std::string type;
            iss >> type;
            
            if (type == "startpos") {
                pos.set("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
            } else if (type == "fen") {
                std::string fen  ;
                std::string part ;
                for (int i = 0; i < 6; ++i) {
                    iss >> part;
                    if (i > 0) fen += " ";
                    fen += part;
                }

                pos.set(fen);
            }

            /*Check for "moves"*/
            std::string token;
            UndoInfo undo;
            while (iss >> token) {
                if (token == "moves") continue;
                Move m = parse_move(pos, token);
                if (m != MOVE_NONE) {
                    pos.make_move(m, undo);
                }
            }

        } else if (command == "go")         {
        
            std::string word;
            iss >> word;

            if (word == "depth") {
                
                int depth;
                iss >> depth;
                SearchResult result = iterative_deepening(pos, depth, 0);
                std::cout << "bestmove " << move_to_string(result.best_move) << std::endl;

            } else if (word == "movetime") {

                int movetime;
                iss >> movetime;
                SearchResult result = iterative_deepening(pos, 100, movetime);
                std::cout << "bestmove " << move_to_string(result.best_move) << std::endl;

            } else if (word == "wtime") {

                int wtime = 0, btime = 0, 
                    winc  = 0, binc  = 0,
                    movestogo = 0;
                
                iss >> wtime;

                std::string token;
                while (iss >> token) {
                    if (token == "btime"    ) iss >> btime     ;
                    if (token == "winc"     ) iss >> winc      ;
                    if (token == "binc"     ) iss >> binc      ;
                    if (token == "movestogo") iss >> movestogo ;
                }

                int our_time  = (pos.get_side_to_move() == Color::White) ? wtime : btime ;
                int our_inc   = (pos.get_side_to_move() == Color::White) ? winc  : binc  ;
                int base_time ;

                /*Calculate base time per move*/
                if (movestogo > 0) {
                    /*Tournament time control (e.g. 40 moves in 5 mins)*/
                    base_time = our_time / movestogo;
                } else {
                    /*Sudden death or increment (estimate 25 moves left)*/
                    base_time = our_time / 25;
                }

                /*Add most of the increment (leave some as buffer)*/
                int time_for_move = base_time + (our_inc * 3 / 4);

                /*Safety margin*/
                int max_time = our_time - 50;
                if (max_time < 1) max_time = 1;

                if (time_for_move > max_time) time_for_move = max_time ;
                if (time_for_move < 1       ) time_for_move = 1        ;

                /*Search with this allocation*/
                SearchResult result = iterative_deepening(pos, 100, time_for_move)        ;
                std::cout << "bestmove " << move_to_string(result.best_move) << std::endl ;

            } else {

                SearchResult result = iterative_deepening(pos, 4, 0)                      ;
                std::cout << "bestmove " << move_to_string(result.best_move) << std::endl ;

            }
        
        } else if (command == "quit")       {
        
            break;   
        
        }
    }
}
