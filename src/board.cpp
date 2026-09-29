#include "board.h"
#include "zobrist.h"
#include <sstream>
#include <string>
#include <iostream>

/*------------------*/
/* THE SET FUNCTION */
/*------------------*/

/*This is the set() function
    It takes in a FEN string so you can set up the position on the board
    and know which colour is next to move*/
void Position::set(const std::string& fen) {
    /*Clearning any FENs leftover from previous call*/
    for (int c = 0; c < 2; c++)
        for (int p = 0; p < 6; p++)
            piece_bbs[c][p] = 0;
    for (int i = 0; i < 64; i++)
        mailbox[i] = PieceType::NUM_PIECE_TYPES;
    for (int i = 0; i < 4 ; i++)
        castling_rights[i] = false;
        
    en_passant_sq = Square::NUM_SQUARES;
    halfmove_clock = 0;
    fullmove_number = 1;
    side_to_move = Color::White;

    std::istringstream ss(fen);
    /*Cutting the FEN string into its constituent parts*/
    std::string piece_placement, side, castling, en_passant;
    int half, full;

    ss >> piece_placement >> side >> castling >> en_passant >> half >> full;

    /*Starting at A8*/
    int square = 56;

    for (char c : piece_placement) {
        if (c == '/') {
            /*End of rank reached so you need to deduct 16*/
            square -= 16;
        } else if (c >= '1' && c <= '8') {
            /*Converts the digit character to the actual numeric value*/
            square += (c - '0');
        } else {
            /*A piece is present*/
            /*Assigning the pieces using a switch statement*/
            int color, piece;

            switch (c) {
                case 'P': color = 0; piece = 0; break ;
                case 'N': color = 0; piece = 1; break ;
                case 'B': color = 0; piece = 2; break ;
                case 'R': color = 0; piece = 3; break ;
                case 'Q': color = 0; piece = 4; break ;
                case 'K': color = 0; piece = 5; break ;
                case 'p': color = 1; piece = 0; break ;
                case 'n': color = 1; piece = 1; break ;
                case 'b': color = 1; piece = 2; break ;
                case 'r': color = 1; piece = 3; break ;
                case 'q': color = 1; piece = 4; break ;
                case 'k': color = 1; piece = 5; break ;
                defualt :                       break ;
            }
            
            piece_bbs[color][piece] |= (1ULL << square);
            mailbox[square] = static_cast<PieceType>(piece);
            square++;
        }
    }

    /*Ternary operator for counting which move it is*/
    side_to_move = (side == "w") ? Color::White : Color::Black;

    /*Tracking the 4 castling options*/
    castling_rights[0] = castling.find('K') != std::string::npos;
    castling_rights[1] = castling.find('Q') != std::string::npos;
    castling_rights[2] = castling.find('k') != std::string::npos;
    castling_rights[3] = castling.find('q') != std::string::npos;
    
    halfmove_clock = half;
    fullmove_number = full;

    /*En passant square converter*/
    if (en_passant == "-") {
        en_passant_sq = Square::NUM_SQUARES;
    } else {
        int file = en_passant[0] - 'a';
        int rank = en_passant[1] - '1';
        en_passant_sq = static_cast<Square>(rank * 8 + file);
    }

    /*Zobrist*/
    hash = Zobrist::compute_hash(*this);
}

/*------------------*/
/* THE FEN FUNCTION */
/*------------------*/

/*This is the fen() function
    It does the opposite of set() by looking at the current position
    and returning a FEN string that describes it*/
std::string Position::fen() const {
    std::string result = "";
    int empty = 0;

    const char piece_chars[2][6] = {
        {'P', 'N', 'B', 'R', 'Q', 'K'},
        {'p', 'n', 'b', 'r', 'q', 'k'}
    };

    for (int rank = 7; rank >= 0; rank--) {
        for (int file = 0; file < 8; file++) {
            int square = rank * 8 + file;
            /*Found tracks whether any piece was on the square*/
            bool found = false;

            /*Two nested loops to check each color/piece combination*/
            for (int color = 0; color < 2; color++) {
                for (int piece = 0; piece < 6; piece++) {
                    if (piece_bbs[color][piece] & (1ULL << square)) {
                        if (empty > 0) {
                            result += std::to_string(empty);
                            empty = 0;
                        }
                        result += piece_chars[color][piece];
                        found = true;
                        break;
                    }
                }
                if (found) break;
            }
            
            /*If no piece is found, increment the empty counter*/
            if (!found) {
                empty++;
            }
        }

        /*When at the end of the rank: flush any remaining empty count*/
        if (empty > 0) {
            result += std::to_string(empty);
            empty = 0;
        }

        /*Add slash between ranks, but not after the last one*/
        if (rank > 0) {
            result += '/';
        }
    }

    /*Side to move*/
    result += ' ';
    result += (side_to_move == Color::White) ? 'w' : 'b';

    /*Castling rights*/
    result += ' ';
    std::string castling_str = "";
    if (castling_rights[0]) castling_str += 'K';
    if (castling_rights[1]) castling_str += 'Q';
    if (castling_rights[2]) castling_str += 'k';
    if (castling_rights[3]) castling_str += 'q';
    if (castling_str.empty()) castling_str += '-';
    result += castling_str;

    /*En passant*/
    result += ' ';
    if (en_passant_sq == Square::NUM_SQUARES) {
        result += '-';
    } else {
        int sq = static_cast<int>(en_passant_sq);
        /*Gives file:*/
        result += char('a' + sq % 8);
        /*Gives rank:*/
        result += char('1' + sq / 8);
    }

    /*Two remaining counters*/
    result += ' ' + std::to_string(halfmove_clock);
    result += ' ' + std::to_string(fullmove_number);

    return result;
}

/*--------------------*/
/* THE PRINT FUNCTION */
/*--------------------*/

/*This is the print() function
    It produces an ASCII board to the console so that a human
    can easily debug the board*/
void Position::print() const {
    const char piece_chars[2][6] = {
        {'P', 'N', 'B', 'R', 'Q', 'K'},
        {'p', 'n', 'b', 'r', 'q', 'k'}
    };

    for (int rank = 7; rank >= 0; rank--) {
        for (int file = 0; file < 8; file++) {
            int square = rank * 8 + file;
            bool found = false;

            for (int color = 0; color < 2; color++) {
                for (int piece = 0; piece < 6; piece++) {
                    if (piece_bbs[color][piece] & (1ULL << square)) {
                        std::cout << piece_chars[color][piece] << ' ';
                        found = true;
                        break;
                    }
                }
                if (found) break;
            }

            if (!found) {
                std::cout << ". ";
            }
        }
        std::cout << '\n';
    }
    std::cout << '\n';
}

/*------------------------*/
/* THE MAKE MOVE FUNCTION */
/*------------------------*/

/*This is the make_move() function
    It saves the current game state and then handles any
    peice movement, captures, special cases before updating
    key board information like castling rights and en passant squares
    and then flips the side to move*/
void Position::make_move(Move move, UndoInfo& undo) {
    /*Start by saving the irreversible state*/
    undo.castling_rights[0] = castling_rights[0] ;
    undo.castling_rights[1] = castling_rights[1] ;
    undo.castling_rights[2] = castling_rights[2] ;
    undo.castling_rights[3] = castling_rights[3] ;
    undo.en_passant_sq      = en_passant_sq      ;
    undo.halfmove_clock     = halfmove_clock     ;
    undo.hash               = hash               ;

    /*Then extract the move info*/
    Square from_sq = move_from(move)  ;
    Square  to_sq  = move_to(move)    ;
    uint16_t flags = move_flags(move) ;

    /*Then identify pieces*/
    PieceType moving_piece = mailbox[static_cast<int>(from_sq)] ;
    PieceType captured     = mailbox[static_cast<int>( to_sq )] ;
    undo.captured_piece = captured;

    /*Then handle captures by removing any captured pieces if they exist*/
    Color  us  = side_to_move;
    Color them = (us == Color::White) ? Color::Black : Color::White;
    int  us_i  = static_cast<int>( us ) ;
    int them_i = static_cast<int>(them) ;
    if (captured != PieceType::NUM_PIECE_TYPES) {
        piece_bbs[them_i][static_cast<int>(captured)] &= ~(1ULL << static_cast<int>(to_sq))              ;
        hash ^= Zobrist::piece_keys[(them_i * 6) +  static_cast<int>(captured)][static_cast<int>(to_sq)] ;
    }

    /*Then move the piece on the bitboard and mailbox*/
    piece_bbs[us_i][static_cast<int>(moving_piece)] &= ~(1ULL << static_cast<int>(from_sq)) ;
    piece_bbs[us_i][static_cast<int>(moving_piece)] |=  (1ULL << static_cast<int>( to_sq )) ;
    mailbox[static_cast<int>(from_sq)] = PieceType::NUM_PIECE_TYPES ;
    mailbox[static_cast<int>( to_sq )] = moving_piece               ;
    hash ^= Zobrist::piece_keys[(us_i * 6) + static_cast<int>(moving_piece)][static_cast<int>(from_sq)] ;
    hash ^= Zobrist::piece_keys[(us_i * 6) + static_cast<int>(moving_piece)][static_cast<int>( to_sq )] ;

    /*---------------*/
    /* SPECIAL CASES */
    /*---------------*/

    /*En passant captures*/
    if (flags == MOVE_EN_PASSANT_CAP) {
        /*The captured pawn is one rank behind destination*/
        int captured_pawn_sq = (us == Color::White) ? static_cast<int>(to_sq) - 8 :
                                                      static_cast<int>(to_sq) + 8 ;
        piece_bbs[them_i][static_cast<int>(PieceType::Pawn)] &= ~(1ULL << captured_pawn_sq);
        mailbox[captured_pawn_sq] = PieceType::NUM_PIECE_TYPES;
        hash ^= Zobrist::piece_keys[(them_i * 6) + static_cast<int>(PieceType::Pawn)][static_cast<int>(captured_pawn_sq)];
    }

    /*En passant square logging*/
    if (undo.en_passant_sq != Square::NUM_SQUARES) {
        int old_file = static_cast<int>(undo.en_passant_sq) % 8;
        hash ^= Zobrist::ep_keys[old_file];
    }
    if (flags == MOVE_PAWN_DOUBLE) {
        en_passant_sq = static_cast<Square>(
            (us == Color::White) ? static_cast<int>(to_sq) - 8 :
                                   static_cast<int>(to_sq) + 8 ) ;
        
        int new_file = static_cast<int>(en_passant_sq) % 8;
        hash ^= Zobrist::ep_keys[new_file];
    } else { 
        en_passant_sq = Square::NUM_SQUARES; 
    }

    /*Castling*/
    if (flags == MOVE_KSIDE_CASTLE) {
        
        /*Start with king side rook movement*/
        int old_rook_sq = (us == Color::White) ? static_cast<int>(Square::H1) :
                                                 static_cast<int>(Square::H8) ;
        int new_rook_sq = (us == Color::White) ? static_cast<int>(Square::F1) :
                                                 static_cast<int>(Square::F8) ;
        
        piece_bbs[us_i][static_cast<int>(PieceType::Rook)] &= ~(1ULL << old_rook_sq) ;
        piece_bbs[us_i][static_cast<int>(PieceType::Rook)] |=  (1ULL << new_rook_sq) ;
        mailbox[old_rook_sq] = PieceType::NUM_PIECE_TYPES ;
        mailbox[new_rook_sq] = PieceType::Rook            ;
        hash ^= Zobrist::piece_keys[(us_i * 6) + static_cast<int>(PieceType::Rook)][static_cast<int>(old_rook_sq)] ;
        hash ^= Zobrist::piece_keys[(us_i * 6) + static_cast<int>(PieceType::Rook)][static_cast<int>(new_rook_sq)] ;

    } else if (flags == MOVE_QSIDE_CASTLE) {

        /*Queen side next*/
        int old_rook_sq = (us == Color::White) ? static_cast<int>(Square::A1) :
                                                 static_cast<int>(Square::A8) ;
        int new_rook_sq = (us == Color::White) ? static_cast<int>(Square::D1) :
                                                 static_cast<int>(Square::D8) ;

        piece_bbs[us_i][static_cast<int>(PieceType::Rook)] &= ~(1ULL << old_rook_sq) ;
        piece_bbs[us_i][static_cast<int>(PieceType::Rook)] |=  (1ULL << new_rook_sq) ;
        mailbox[old_rook_sq] = PieceType::NUM_PIECE_TYPES ;
        mailbox[new_rook_sq] = PieceType::Rook            ;
        hash ^= Zobrist::piece_keys[(us_i * 6) + static_cast<int>(PieceType::Rook)][static_cast<int>(old_rook_sq)] ;
        hash ^= Zobrist::piece_keys[(us_i * 6) + static_cast<int>(PieceType::Rook)][static_cast<int>(new_rook_sq)] ;

    } 

    /*Promotion*/
    if (flags >= (8 << 12)) {
        /*Promotions can be dealt with together due to flag bits*/
        int promo_bits = (flags >> 12) & 0x3;
        /*+1 to skip pawns*/
        PieceType promo_piece = static_cast<PieceType>(promo_bits + 1);

        /*Remove the pawn and add the promoted piece*/
        piece_bbs[us_i][static_cast<int>(PieceType::Pawn)] &= ~(1ULL << static_cast<int>(to_sq)) ;
        piece_bbs[us_i][static_cast<int>(  promo_piece  )] |=  (1ULL << static_cast<int>(to_sq)) ;
        /*Update the mailbox*/
        mailbox[static_cast<int>(to_sq)] = promo_piece;

        hash ^= Zobrist::piece_keys[(us_i * 6) + static_cast<int>(PieceType::Pawn)][static_cast<int>(to_sq)] ;
        hash ^= Zobrist::piece_keys[(us_i * 6) + static_cast<int>(  promo_piece  )][static_cast<int>(to_sq)] ;
    }

    /*-----------------------*/
    /* CASTLING RIGHTS CHECK */
    /*-----------------------*/

    int from_i = static_cast<int>(from_sq) ;
    int  to_i  = static_cast<int>( to_sq ) ;

    int old_castle = (static_cast<int>(castling_rights[0]) << 3)
                   | (static_cast<int>(castling_rights[1]) << 2)
                   | (static_cast<int>(castling_rights[2]) << 1)
                   | (static_cast<int>(castling_rights[3])     ) ;
    
    /*Check for moved kings or rooks and captured rooks*/
    if (from_i == static_cast<int>(Square::E1) || to_i == static_cast<int>(Square::E1))
        castling_rights[0] = castling_rights[1] = false;
    if (from_i == static_cast<int>(Square::E8) || to_i == static_cast<int>(Square::E8))
        castling_rights[2] = castling_rights[3] = false;
    if (from_i == static_cast<int>(Square::H1) || to_i == static_cast<int>(Square::H1))
        castling_rights[0] = false;
    if (from_i == static_cast<int>(Square::A1) || to_i == static_cast<int>(Square::A1))
        castling_rights[1] = false;
    if (from_i == static_cast<int>(Square::H8) || to_i == static_cast<int>(Square::H8))
        castling_rights[2] = false;    
    if (from_i == static_cast<int>(Square::A8) || to_i == static_cast<int>(Square::A8))
        castling_rights[3] = false;

    int new_castle = (static_cast<int>(castling_rights[0]) << 3)
                   | (static_cast<int>(castling_rights[1]) << 2)
                   | (static_cast<int>(castling_rights[2]) << 1)
                   | (static_cast<int>(castling_rights[3])     ) ;

    hash ^= Zobrist::castling_keys[old_castle] ;
    hash ^= Zobrist::castling_keys[new_castle] ;

    /*------------------*/
    /* CLOCK MANAGEMENT */
    /*------------------*/

    /*Halfmove clock for draw by 50-move rule*/
    if (moving_piece == PieceType::Pawn || captured != PieceType::NUM_PIECE_TYPES) {
        halfmove_clock = 0;
    } else { halfmove_clock++; }

    /*Flip side_to_move*/
    if (us == Color::Black) {
        fullmove_number++;
        side_to_move = Color::White;
    } else { side_to_move = Color::Black; }

    hash ^= Zobrist::side_key;

}

/*--------------------------*/
/* THE UNMAKE MOVE FUNCTION */
/*--------------------------*/

/*This is the unmake_move() function
    It does the exact opposite of the make_move() function*/
void Position::unmake_move(Move move, const UndoInfo& undo) {
    /*Start by flipping the side to move back*/
    side_to_move = (side_to_move == Color::White) ? Color::Black : Color::White;
    
    /*Then set the correct us/them color*/
    Color  us  = side_to_move;
    Color them = (us == Color::White) ? Color::Black : Color::White;
    int  us_i  = static_cast<int>( us ) ;
    int them_i = static_cast<int>(them) ;

    /*Then extract the move info*/
    Square from_sq = move_from(move)  ;
    Square  to_sq  = move_to(move)    ;
    uint16_t flags = move_flags(move) ;

    /*Then identify pieces (reinstating captured pieces comes later)*/
    PieceType moving_piece = mailbox[static_cast<int>( to_sq )];
    PieceType captured     = undo.captured_piece;

    /*Restore irreversible state*/
    castling_rights[0] = undo.castling_rights[0] ;
    castling_rights[1] = undo.castling_rights[1] ;
    castling_rights[2] = undo.castling_rights[2] ;
    castling_rights[3] = undo.castling_rights[3] ;
    en_passant_sq      = undo.en_passant_sq      ;
    halfmove_clock     = undo.halfmove_clock     ;

    /*---------------*/
    /* SPECIAL CASES */
    /*---------------*/

    /*En passant captures*/
    if (flags == MOVE_EN_PASSANT_CAP) {
        int captured_pawn_sq = (us == Color::White) ? static_cast<int>(to_sq) - 8 :
                                                      static_cast<int>(to_sq) + 8 ;
        /*Replace taken pawn*/
        piece_bbs[them_i][static_cast<int>(PieceType::Pawn)] |=  (1ULL << captured_pawn_sq);
        mailbox[captured_pawn_sq] = PieceType::Pawn;
    }

    /*Castling*/
    if (flags == MOVE_KSIDE_CASTLE) {
        /*Start with king side rook movement*/
        int old_rook_sq = (us == Color::White) ? static_cast<int>(Square::H1) :
                                                 static_cast<int>(Square::H8) ;
        int new_rook_sq = (us == Color::White) ? static_cast<int>(Square::F1) :
                                                 static_cast<int>(Square::F8) ;
        piece_bbs[us_i][static_cast<int>(PieceType::Rook)] |=  (1ULL << old_rook_sq) ;
        piece_bbs[us_i][static_cast<int>(PieceType::Rook)] &= ~(1ULL << new_rook_sq) ;
        mailbox[old_rook_sq] = PieceType::Rook            ;
        mailbox[new_rook_sq] = PieceType::NUM_PIECE_TYPES ;
    } else if (flags == MOVE_QSIDE_CASTLE) {
        /*Queen side next*/
        int old_rook_sq = (us == Color::White) ? static_cast<int>(Square::A1) :
                                                 static_cast<int>(Square::A8) ;
        int new_rook_sq = (us == Color::White) ? static_cast<int>(Square::D1) :
                                                 static_cast<int>(Square::D8) ;
        piece_bbs[us_i][static_cast<int>(PieceType::Rook)] |=  (1ULL << old_rook_sq) ;
        piece_bbs[us_i][static_cast<int>(PieceType::Rook)] &= ~(1ULL << new_rook_sq) ;
        mailbox[old_rook_sq] = PieceType::Rook            ;
        mailbox[new_rook_sq] = PieceType::NUM_PIECE_TYPES ;
    } 

    /*Promotion*/
    if (flags >= (8 << 12)) {
        int promo_bits = (flags >> 12) & 0x3;
        PieceType promo_piece = static_cast<PieceType>(promo_bits + 1);

        /*Remove the promoted piece and add back the pawn*/
        piece_bbs[us_i][static_cast<int>(PieceType::Pawn)] |=  (1ULL << static_cast<int>(to_sq)) ;
        piece_bbs[us_i][static_cast<int>(promo_piece)]     &= ~(1ULL << static_cast<int>(to_sq)) ;
        mailbox[static_cast<int>(to_sq)] = PieceType::Pawn;

        /*Move back the pawn instead of the promoted piece*/
        moving_piece = PieceType::Pawn;
    }

    /*Special cases undone*/

    /*Unmove the piece on the bitboard and mailbox*/
    piece_bbs[us_i][static_cast<int>(moving_piece)] |=  (1ULL << static_cast<int>(from_sq)) ;
    piece_bbs[us_i][static_cast<int>(moving_piece)] &= ~(1ULL << static_cast<int>( to_sq )) ;
    mailbox[static_cast<int>(from_sq)] = moving_piece               ;
    mailbox[static_cast<int>( to_sq )] = PieceType::NUM_PIECE_TYPES ;               ;
    
    /*Undo any captures if they took place*/
    if (undo.captured_piece != PieceType::NUM_PIECE_TYPES) {
        piece_bbs[them_i][static_cast<int>(undo.captured_piece)] |=  (1ULL << static_cast<int>(to_sq));
        mailbox[static_cast<int>(to_sq)] = undo.captured_piece;
    }

    if (us == Color::Black) {
        fullmove_number--;
    }

    hash = undo.hash;
}

/*--------------------*/
/* THE NULL FUNCTIONS */
/*--------------------*/

void Position::null_make_move(UndoInfo& undo) {
    /*Far less to save because a move doesn't actually take place*/
    undo.en_passant_sq      = en_passant_sq      ;
    undo.halfmove_clock     = halfmove_clock     ;
    undo.hash               = hash               ;

    /*En passant square logging*/
    if (undo.en_passant_sq != Square::NUM_SQUARES) {
        int old_file = static_cast<int>(en_passant_sq) % 8;
        hash ^= Zobrist::ep_keys[old_file];
    } 
    en_passant_sq = Square::NUM_SQUARES; 

    /*Flip side_to_move*/
    side_to_move = (side_to_move == Color::White) ? Color::Black : Color::White;
    hash ^= Zobrist::side_key;
}

void Position::null_unmake_move(const UndoInfo& undo) {
    en_passant_sq  = undo.en_passant_sq  ;
    halfmove_clock = undo.halfmove_clock ;
    hash           = undo.hash           ;

    side_to_move = (side_to_move == Color::White) ? Color::Black : Color::White;
}
