#pragma once
#include "types.h"
#include <string>

/*Needed to create unmake_move() for the engine to search through lines without destroying the position*/
struct UndoInfo {
    PieceType captured_piece ;
    bool castling_rights[4]  ;
    Square en_passant_sq     ;
    int halfmove_clock       ;
    uint64_t hash            ;
};

class Position {
    public:
        void set(const std::string& fen);
        std::string fen() const;
        void print() const;
        
        /*Read only getters for movegen*/
        Color get_side_to_move() const { return side_to_move; }
        Bitboard get_pieces(Color c, PieceType pt) const { return piece_bbs[static_cast<int>(c)][static_cast<int>(pt)]; }
        Bitboard get_all_pieces(Color c) const {
            int ci = static_cast<int>(c);
            return piece_bbs[ci][0] | piece_bbs[ci][1] | piece_bbs[ci][2] |
                   piece_bbs[ci][3] | piece_bbs[ci][4] | piece_bbs[ci][5] ;
        }
        
        Square ep_square() const { return en_passant_sq; }
        bool get_castling_rights(int index) const { return castling_rights[index]; }
        
        void make_move(  Move move,       UndoInfo& undo) ;
        void unmake_move(Move move, const UndoInfo& undo) ;
        /*Null versions of make and unmake move for null pruning*/
        void null_make_move(        UndoInfo& undo) ;
        void null_unmake_move(const UndoInfo& undo) ;

        /*Needed for Zobrist*/
        int get_castling_rights() const {
        return (static_cast<int>(castling_rights[0]) << 3)
             | (static_cast<int>(castling_rights[1]) << 2)
             | (static_cast<int>(castling_rights[2]) << 1)
             | (static_cast<int>(castling_rights[3])     ) ;
        }
        
        Square get_ep_square() const { return en_passant_sq; }
        uint64_t hash;
        PieceType get_mailbox(int sq) const { return mailbox[sq]; }

    private:
        Bitboard piece_bbs[2][6];
        Color side_to_move;
        bool castling_rights[4];
        Square en_passant_sq;
        int halfmove_clock;
        int fullmove_number;
        PieceType mailbox[64];
};
