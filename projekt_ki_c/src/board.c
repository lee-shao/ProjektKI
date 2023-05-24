#include "board.h"
#include <inttypes.h>
#include <math.h>
#include <inttypes.h>
#include <math.h>

const char PIECE_CHARS[] = {'P', 'B', 'N', 'R', 'Q', 'K'};
const int PIECE_VALUES[] = {10, 30, 30, 50, 90, 20000};

const int LOG264_TAB[64] = {
    63,  0, 58,  1, 59, 47, 53,  2,
    60, 39, 48, 27, 54, 33, 42,  3,
    61, 51, 37, 40, 49, 18, 28, 20,
    55, 30, 34, 11, 43, 14, 22,  4,
    62, 57, 46, 52, 38, 26, 32, 41,
    50, 36, 17, 19, 29, 10, 13, 21,
    56, 45, 25, 31, 35, 16,  9, 12,
    44, 24, 15,  8, 23,  7,  6,  5
};

int log2_64 (__uint64_t value)
{
    value |= value >> 1;
    value |= value >> 2;
    value |= value >> 4;
    value |= value >> 8;
    value |= value >> 16;
    value |= value >> 32;
    return LOG264_TAB[((__uint64_t)((value - (value >> 1))*0x07EDD5E59A4E28C2)) >> 58];
}

void print_board(board_state* pos) {
    printf(" | A| B| C| D| E| F| G| H|\n8|");
    for (int i = 0; i < 64; i++) {
        //print curr field
        char player = ' ';
        char piece = ' ';
        int bit_pos = ((63 - i) / 8) * 8 + (i % 8);; //(i / 8) * 8 + 7 - (i % 8); // ahh what is this xD

        //extract piece
        for (int j = 0; j < 6; j++) {
            if ((pos->pieces[j] >> bit_pos) & 1) {
                piece = PIECE_CHARS[j];
            }
        }

        //extract player
        if ((pos->black >> bit_pos) & 1) {
            player = 'B';
        } else if ((pos->white >> bit_pos) & 1) {
            player = 'W';
        }

        //check for invalid combinations
        if (player != ' ' && piece == ' ') {
            piece = 'E'; //missing piece
        } else if (player == ' ' && piece != ' ') {
            player = 'E'; //missing player
        }

        printf("%c%c|", player, piece);

        //new line every 8 fields
        if ((i + 1) % 8 == 0 && i != 63) {
            printf("\n%d|", 7 - (i / 8)); //we print line 8 first
        }
    }
    printf("\n");
}

void print_board_binary(board_state* pos, __uint64_t bin) {
    printf(" | A| B| C| D| E| F| G| H|\n8|");
    for (int i = 0; i < 64; i++) {
        //print curr field
        char bin_val = ' ';
        char piece = ' ';
        int bit_pos = ((63 - i) / 8) * 8 + (i % 8);; //(i / 8) * 8 + 7 - (i % 8); // ahh what is this xD

        //extract piece
        if (pos != NULL) {
            for (int j = 0; j < 6; j++) {
                if ((pos->pieces[j] >> bit_pos) & 1) {
                    piece = PIECE_CHARS[j];
                }
            }

            //extract player
            if ((pos->black >> bit_pos) & 1) {
                piece = tolower(piece);
            }
        }


        if ((bin >> bit_pos) & 1) {
            bin_val = 'X';
        }

        printf("%c%c|", bin_val, piece);

        //new line every 8 fields
        if ((i + 1) % 8 == 0 && i != 63) {
            printf("\n%d|", 7 - (i / 8)); //we print line 8 first
        }
    }
    printf("\n");
}

board_state* fen_to_board(char* fen) {
    //copy string to make sure it is editable
    char* fen_copy = calloc(strlen(fen) + 1, sizeof(char));
    strcpy(fen_copy, fen);

    //alloc board
    board_state *board = malloc(sizeof(board_state));
    memset(board, 0, sizeof(board_state));

    //split fen string
    char* positions = NULL;
    char* player = NULL;
    char* castling = NULL;
    char* en_passant = NULL;
    char* half_moves = NULL;
    char* full_moves = NULL;

    char delim[] = " ";
    char* curr_split_state = fen_copy;
    positions =  strtok_r(curr_split_state, delim, &curr_split_state);
    player =     strtok_r(curr_split_state, delim, &curr_split_state);
    castling =   strtok_r(curr_split_state, delim, &curr_split_state);
    en_passant = strtok_r(curr_split_state, delim, &curr_split_state);
    half_moves = strtok_r(curr_split_state, delim, &curr_split_state);
    full_moves = strtok_r(curr_split_state, delim, &curr_split_state);


    //iterate over positions string
    int pos_len = strlen(positions);
    int bit_pos = 0;
    int field_count = 0;
    for (int i = 0; i < pos_len && bit_pos >= 0; i++) {
        bit_pos = ((63 - field_count) / 8) * 8 + (field_count % 8);
        if (positions[i] >= '0' && positions[i] <= '9') { //if is number
            field_count += positions[i] - '0'; //skip x amount of fields
        } else if (positions[i] != '/') {
            if (get_player_from_fen(positions[i]) == -1) { //check for lower case: black
                board->black |= (__uint64_t)1 << bit_pos;
            } else { //white instead
                board->white |= (__uint64_t)1 << bit_pos;
            }

            //set piece type
            int type = 0;
            if ((type = get_piece_from_fen(positions[i])) != -1) {
                board->pieces[type] |= (__uint64_t)1 << bit_pos;
                field_count++;
            }
        } else {
            //TODO: use '/' to try to fix wrongly formated fen strings
        }
    }

    //get curr player
    if (player != NULL) {
        if (player[0] == 'w')
            board->player = 1; //white
        else
            board->player = -1; //black
    }

    //extract castling possibilities
    if (castling != NULL) {
        int castle_len = strlen(castling);
        for (int i = 0; i < castle_len; i++) {
            switch (castling[i])
            {
            case 'K':
                board->castling |= 0x0000000000000004;
                break;
            case 'Q':
                board->castling |= 0x0000000000000040;
                break;
            case 'k':
                board->castling |= 0x0400000000000000;
                break;
            case 'q':
                board->castling |= 0x4000000000000000;
                break;
            default:
                break;
            }
        }
    }

    //extract en passant targets
    if (en_passant != NULL) {
        int en_passant_len = strlen(en_passant);
        if (en_passant_len == 2) {
            board->en_passant |= (__uint64_t)1 << ((toupper(en_passant[0]) - 'A') + (en_passant[1] - '1') * 8);
        }
    }

    //get halfmove clock
    if (half_moves != NULL) {
        board->half_moves = atoi(half_moves);
    }

    //get fullmove clock
    if (full_moves != NULL) {
        board->full_moves = atoi(full_moves);
    }

    //clean up
    free(fen_copy);

    return board;
}

char* board_to_fen(board_state* state) {
    //TODO: implement
    return NULL;
}

board_state* clone_board_state(board_state* state) {
    board_state *cloned_state = malloc(sizeof(board_state));
    memcpy(cloned_state, state, sizeof(board_state));
    return cloned_state;
}

int get_piece_from_fen(char fen) {
    switch (toupper(fen))
    {
    case 'P':
        return PAWN;
    case 'B':
        return BISHOP;
    case 'N':
        return KNIGHT;
    case 'R':
        return ROOK;
    case 'Q':
        return QUEEN;
    case 'K':
        return KING;
    default:
        return -1; //invalid char
    }
}

__int8_t get_player_from_fen(char fen) {
    if (fen >= 'a' && fen <= 'z') { //check for lower case: black
        return -1;
    }
    return 1; //white instead
}

board_move* fen_to_move(char *fen, board_state *state) {
    int len = strlen(fen);

    if (len >= 1 && fen[len -1] == '\n') { //filter new line char
        len--;
    }

    if (len < 2) { //check if string is to short
        return NULL;
    }

    int piece_type = PAWN; //piece is pawn if nothing is given
    __int64_t to = 0; //target pos
    int from_column = -1; //column given if move is not unique
    int from_row = -1; //row given if move is not unique

    //extract data from fen
    switch (len) {
        case 2: //pawn move
            to = (__uint64_t)1 << ((toupper(fen[0]) - 'A') + (fen[1] - '1') * 8);
            break;
        case 3: //piece type and target coords given
            piece_type = get_piece_from_fen(fen[0]); //get piece type
            to = (__uint64_t)1 << ((toupper(fen[1]) - 'A') + (fen[2] - '1') * 8);
            break;
        case 4: //piece type row or column and coords given
            piece_type = get_piece_from_fen(fen[0]); //get piece type
            if (toupper(fen[1]) >= 'A' && toupper(fen[1]) <= 'Z') {
                from_column = (toupper(fen[1]) - 'A');
            } else {
                from_row = (fen[1] - '1');
            }
            to = (__uint64_t)1 << ((toupper(fen[2]) - 'A') + (fen[3] - '1') * 8);
            break;
        default:
            return NULL; //invalid move
    }

    if (piece_type == -1) { //check for invalid piece type
        return NULL;
    }

    //input format seems legit - generate move
    board_move* move = malloc(sizeof(board_move));
    memset(move, 0, sizeof(board_move));

    //get possible moves and iterate through them
    __uint64_t player_board = state->black;
    if (state->player == 1)
        player_board = state->white;
    __uint64_t moves = 0;
    for (int bit_pos = 0; bit_pos < 64; bit_pos++) {
        if (state->pieces[piece_type] & player_board & (__uint64_t)1 << bit_pos) {
            //TODO: get piece that can perform the move
            //CHANGE ME this is a dummy implementation
            if ((from_column == -1 || bit_pos % 8 == from_column) && (from_row == -1 || bit_pos / 8 == from_row)) { //only search for moves in from row or column if given
                moves = get_all_possible_moves(state, piece_type, (__uint64_t)1 << bit_pos);
                if (moves & to) {
                    move->from = (__uint64_t)1 << bit_pos;
                    break;
                }
            }
        }
    }

    move->to = to;
    move->piece = piece_type;

    return move;
}

__uint64_t fen_pos_to_uint(char* pos, int start_index) {
    return (__uint64_t)1 << ((toupper(pos[start_index]) - 'A') + (pos[start_index + 1] - '1') * 8);
}

char* uint_pos_to_fen(__uint64_t pos) {
    int bit_pos = log2(pos);
    char* ret = malloc(3 * sizeof(char)); //return value is 2 chars
    ret[0] = bit_pos % 8 + 'a'; //x
    ret[1] = bit_pos / 8 + '1'; //y
    ret[2] = 0;                 //end of string
    return ret;
}

int perform_move(board_state* state, board_move* move) {
    int from_player = 0;
    int to_player = 0;

    //check from pos
    if (state->black & move->from)
        from_player = -1; //black
    else if (state->white & move->from)
        from_player = 1; //white

    if (from_player != state->player)
        return 1; //invalid start pos

    //check to pos
    if (state->black & move->to)
        to_player = -1; //black
    else if (state->white & move->to)
        to_player = 1; //white
    
    if (from_player == to_player)
        return 2; //invalid to pos (can't take own piece)

    //get piece type to move
    int piece_type = move->piece;
    if (piece_type == -1) {
        //search piece type if not given
        for (piece_type = 0; piece_type < 6; piece_type++) {
            if (state->pieces[piece_type] & move->from) {
                break; //piece type found
            }
        }
    }

    if ((state->pieces[piece_type] & move->from) == 0)
        return 3; //invalid piece type

    //__uint64_t test = get_all_possible_moves(state, piece_type, move->from);
    //print_board_binary(state, test);

    //perform move
    update_castling_state(state);
    detect_and_perforn_castling(state, move);
    state->pieces[piece_type] &= ~move->from;

    if (to_player != 0) {
        for (int i = 0; i < 6; i++) {
            state->pieces[i] &= ~move->to; //take piece at to pos
        }
    }
    state->pieces[piece_type] |= move->to;

    //update player boards
    state->white &= ~move->from;
    state->black &= ~move->from;
    if (from_player == 1) {
        state->white |= move->to;
        state->black &= ~move->to;
    } else {
        state->black |= move->to;
        state->white &= ~move->to;
        //update full moves
        state->full_moves++;
    }
    
    //update half moves
    if (piece_type == PAWN || to_player != 0) {
        //reset on pawn move or if a piece is taken
        state->half_moves = 0;
    } else {
        state->half_moves++;
    }

    //update player
    state->player *= -1;

    return 0; //move successful
}

/**
 * Diese Funktion berechnet für eine beliebige Figur das Bewegungsmuster und gibt diese in einem Bitboard aus
 * @param piecetype Gibt den Figurentyp an, für den das Bewegungsmuster berechnet werden soll
 * @param move enthält die Position, auf die sich die Figur befindet
 * @return Gibt das Bewegungsmusters einer Figur als ein 64-Bit-Integer aus
 * @author Shao
*/
__uint64_t get_all_possible_moves(board_state* state, int piecetype, __uint64_t f) {
    __uint64_t possible_moves = 0;
    __uint64_t a_col = 0x7F7F7F7F7F7F7F7F;
    __uint64_t h_col = 0xFEFEFEFEFEFEFEFE;
    __uint64_t w_baseline = 0x000000000000FF00;
    __uint64_t b_baseline = 0x00FF000000000000;
    __uint64_t occupied = state->white | state->black;

    switch (piecetype)
    {
    /**
    * Either pawn is at left/right border or between
    * TODO: extra rules (attacking, double step, promotion, en-passant)
    */
    case 0:
        // doppelzug
        if (f & w_baseline && state->player == 1) {
            if ((f << 8) & ~occupied && (f << 16) & ~occupied) {
                possible_moves |= f << 16;
            }
        } else if (f & b_baseline && state->player == -1) {
            if ((f >> 8) & ~occupied && (f >> 16) & ~occupied) {
                possible_moves |= f >> 16;
            }
        }
        // Bauer auf H-Reihe
        if (f & ~a_col) {
            if (state->player == 1) {
                if (f << 8 & ~occupied)
                    possible_moves |= f << 8;
                if (f << 7 & state->black)
                    possible_moves |= f << 7;
            } else if (state->player == -1) {
                if (f >> 8 & ~occupied)
                    possible_moves |= f >> 8;
                if (f >> 9 & state->white)
                    possible_moves |= f >> 9;
            }
        // Bauer auf A-Reihe
        } else if (f & ~h_col) {
            if (state->player == 1) {
                if (f << 8 & ~occupied)
                    possible_moves |= f << 8;
                if (f << 9 & state->black)
                    possible_moves |= f << 9;
            } else if (state->player == -1) {
                if (f >> 8 & ~occupied)
                    possible_moves |= f >> 8;
                if (f >> 7 & state->white)
                    possible_moves |= f >> 7;
            }
        // Bauer mittig
        } else {
            if (state->player == 1) {
                if (f << 8 & ~occupied) {
                    possible_moves |= f << 8;
                }
                if (f << 9 & state->black) {
                    possible_moves |= f << 9;
                }
                if (f << 7 & state->black) {
                    possible_moves |= f << 7;
                }
            } else if (state->player == -1) {
                if (f >> 8 & ~occupied) {
                    possible_moves |= f >> 8;
                }
                if (f >> 9 & state->white) {
                    possible_moves |= f >> 9;
                }
                if (f >> 7 & state->white) {
                    possible_moves |= f >> 7;
                }
            }
        }
        possible_moves = filter_occupied_moves(state, possible_moves);
        return possible_moves;
    case BISHOP:
        if (state->player == 1) {
            possible_moves |= diagonal_movement(f, state);
        } else {
            possible_moves |= diagonal_movement(f, state);
        }
        return possible_moves;
    case KNIGHT:
        if (state->player == 1) {
            possible_moves = knight_movement(f);
        } else {
            possible_moves = knight_movement(f);
        }
        possible_moves = filter_occupied_moves(state, possible_moves);
        return possible_moves;
    case ROOK:
        if (state->player == 1) {
            possible_moves = straight_movement(f, state);
        } else {
            possible_moves |= straight_movement(f, state);
        }
        return possible_moves;
    case QUEEN:
        if (state->player == 1) {
            possible_moves |= diagonal_movement(f, state) | straight_movement(f, state);
        } else {
            possible_moves |= diagonal_movement(f, state) | straight_movement(f, state);
        }
        return possible_moves;
    case KING:
        // If the king is at the left/right border
        if (f & ~a_col) {
            possible_moves |= f << 7 | f << 8 | f >> 1 | f >> 8 | f >> 9;
        } else if (f & ~h_col) {
            possible_moves |= f << 9 | f << 8 | f << 1 | f >> 8 | f >> 7;
        } else {
            possible_moves |= f << 9 | f << 8 | f << 7 | f << 1 | f >> 1 | f >> 7 | f >> 8 | f >> 9;
        }
        possible_moves = filter_occupied_moves(state, possible_moves);
        possible_moves |= check_castling(state);
        return possible_moves;
    default:
        return -1; //invalid piecetype
    }
}

/**
 * Diese Funktion berechnet ausgehend von einer Position alle diagonal erreichbaren Felder bis zum Spielbrettrand. 
 * In occupied wird ein 64-Bitboard gespeichert, wo bereits die eigenen Figuren stehen.
 * Das Invertierte Bitboard von occupied kann dann verUNDed werden um besetzte Felder auszuschließen. 
 * @author Shao
*/
__uint64_t diagonal_movement(__uint64_t position, board_state* state) {
    __uint64_t possible_moves = 0;
    __uint64_t new_field = 0;
    __uint64_t occupied = state->white | state->black;
    int row = get_row(position); // Zeile
    int col = get_col(position); // Spalte
    int i = 0;

    //links oben
    for (i = 1; row + i < 8 && col - i >= 0; i++) {
        new_field = (__uint64_t) 1 << ((row + i) * 8 + (col - i));
        if (new_field & occupied) {
            if ((state->player == 1 && new_field & state->black) || (state->player == -1 && new_field & state->white)) {
                possible_moves |= new_field;
            }
            break;
        } else {
            possible_moves |= new_field;
        }
    }
    //rechts oben
    for (i = 1; row + i < 8 && col + i < 8; i++) {
        new_field = (__uint64_t) 1 << ((row + i) * 8 + (col + i));
        if (new_field & occupied) {
            if ((state->player == 1 && new_field & state->black) || (state->player == -1 && new_field & state->white)) {
                possible_moves |= new_field;
            }
            break;
        } else {
            possible_moves |= new_field;
        } 
    }
    //links unten
    for (i = 1; row - i >= 0 && col - i >= 0; i++) {
        new_field = (__uint64_t) 1 << ((row - i) * 8 + (col - i));
        if (new_field & occupied) {
            if ((state->player == 1 && new_field & state->black) || (state->player == -1 && new_field & state->white)) {
                possible_moves |= new_field;
            }
            break;
        } else {
            possible_moves |= new_field;
        }
    }
    //rechts unten
    for (i = 1; row - i >= 0 && col + i < 8; i++) {
        new_field = (__uint64_t) 1 << ((row - i) * 8 + (col + i));
        if (new_field & occupied) {
            if ((state->player == 1 && new_field & state->black) || (state->player == -1 && new_field & state->white)) {
                possible_moves |= new_field;
            }
            break;
        } else {
            possible_moves |= new_field;
        }
    }

    return possible_moves;
}

/**
 * Diese Funktion berechnet ausgehend von einer Position alle gerade erreichbaren Felder bis zum Spielbrettrand. 
 * Dafür werden Geraden an die Position der Figur verschoben, um die Felder abzudecken, die die Figur in 4 Richtungen laufen kann. 
 * In occupied wird ein 64-Bitboard gespeichert, wo bereits die eigenen Figuren stehen.
 * Das Invertierte Bitboard von occupied kann dann verUNDed werden um besetzte Felder auszuschließen. 
 * @author Shao
*/
__uint64_t straight_movement(__uint64_t position, board_state* state) {
    __uint64_t possible_moves = 0;
    __uint64_t new_field = 0;
    __uint64_t occupied = state->white | state->black;
    int row = get_row(position); // Zeile
    int col = get_col(position); // Spalte
    int i = 0;

    //top
    for (i = 1; row + i < 8; i++) {
        new_field = (__uint64_t) 1 << ((row + i) * 8 + col);
        if (new_field & occupied) {
            if (((state->player == 1) && (new_field & state->black)) || (state->player == -1 && new_field & state->white)) {
                possible_moves |= new_field;
            }
            break;
        } else {
            possible_moves |= new_field;
        }
    }
    //bottom
    for (i = 1; row - i >= 0 ; i++) {
        new_field = (__uint64_t) 1 << ((row - i) * 8 + col);
        if (new_field & occupied) {
            if ((state->player == 1 && new_field & state->black) || (state->player == -1 && new_field & state->white)) {
                possible_moves |= new_field;
            
            }
            break;  
        } else {
            possible_moves |= new_field;
        }
    }
    //left
    for (i = 1; col - i >= 0 ; i++) {
        new_field = (__uint64_t) 1 << (row * 8 + (col - i));
        if (new_field & occupied) {
            if ((state->player == 1 && new_field & state->black) || (state->player == -1 && new_field & state->white)) {
                possible_moves |= new_field;
            }
            break;
        } else {
            possible_moves |= new_field;
        }
    }
    //right
    for (i = 1; col + i < 8 ; i++) {
        new_field = (__uint64_t) 1 << (row * 8 + (col + i));
        if (new_field & occupied) {
            if ((state->player == 1 && new_field & state->black) || (state->player == -1 && new_field & state->white)) {
                possible_moves |= new_field;
            }
            break;
        } else {
            possible_moves |= new_field;
        }
    }

    return possible_moves;
}

/**
 * Bewegungsmuster für den Springer
*/
__uint64_t knight_movement(__uint64_t position) {
    __uint64_t possible_moves = 0;

    // 2 Felder erreichbar

    // h8
    if (position & 0x8000000000000000)
        possible_moves |= position >> 10 | position >> 17;
    // a8
    if (position & 0x0100000000000000)
        possible_moves |= position >> 6 | position << 15;
    // h1
    if (position & 0x0000000000000080)
        possible_moves |= position << 15 | position << 6;
    // a1
    if (position & 0x0000000000000001)
        possible_moves |= position << 10 | position << 17;

    // 3 Felder erreichbar

    // b8 und g8
    if (position & 0x4000000000000000 || position & 0x0200000000000000) {
        possible_moves |= position >> 17 | position >> 15;
        if (position & 0x4000000000000000) {
            possible_moves |= position >> 10;
        }
        if (position & 0x0200000000000000) {
            possible_moves |= position >> 6;
        }
    }
    // h7 und h2
    if (position & 0x0080000000000000 || position & 0x0000000000008000) {
        possible_moves |= position << 6 | position >> 10;
        if (position & 0x0080000000000000)
            possible_moves |= position >> 17;
        if (position & 0x0000000000008000)
            possible_moves |= position << 15;
    }
    // a7 und a2
    if (position & 0x0001000000000000 || position & 0x0000000000000100) {
        possible_moves |= position << 10 | position >> 6;
        if (position & 0x0001000000000000)
            possible_moves |= position >> 15;
        if (position & 0x0000000000000100)
            possible_moves |= position << 17;
    }
    // g1 und b1
    if ((position & 0x0000000000000040) || (position & 0x0000000000000002)) {
        possible_moves |= position << 15 | position << 17;
        if (position & 0x0000000000000040)
            possible_moves |= position << 6;
        if (position & 0x0000000000000002)
            possible_moves |= position << 10; 
    }

    // 4 Felder erreichbar

    // c8, d8, e8, f8
    if (position & 0x3C00000000000000)
        possible_moves |= position >> 6 | position >> 10 | position >> 15 | position >> 17;
    // h6, h5, h4, h3
    if (position & 0x0000808080800000)
        possible_moves |= position << 15 | position << 6 | position >> 10 | position >> 17;
    // a6, a5, a4, a3
    if (position & 0x0000010101010000)
        possible_moves |= position << 17 | position << 10 | position >> 6 | position >> 15;
    // c1, d1, e1, f1
    if (position & 0x000000000000003C)
        possible_moves |= position >> 6 | position >> 10 | position >> 15 | position >> 17;
    // g7
    if (position & 0x0040000000000000)
        possible_moves |= position << 6 | position >> 10 | position >> 17 | position >> 15;
    // b7
    if (position & 0x0002000000000000)
        possible_moves |= position << 10 | position >> 6 | position >> 15 | position >> 17;
    // g2
    if (position & 0x0000000000004000)
        possible_moves |= position << 17 | position << 15 | position << 6 | position >> 10;
    // b2
    if (position & 0x0000000000000200)
        possible_moves |= position << 15 | position << 17 | position << 10 | position >> 6;

    // 6 Felder erreichbar

    // c7, d7, e7, f7
    if (position & 0x003C000000000000)
        possible_moves |= position << 6 | position << 10 | position >> 10 | position >> 6 | position >> 17 | position >> 15;
    // g6, g5, g4, g3
    if (position & 0x0000404040400000)
        possible_moves |= position << 17 | position << 15 | position << 6 | position >> 10 | position >> 17 | position >> 15;
    // b6, b5, b4, b3
    if (position & 0x0000020202020000)
        possible_moves |= position << 17 | position << 15 | position << 10 | position >> 6 | position >> 15 | position >> 17;
    // c2, d2, e2, f2
    if (position & 0x003C000000000000)
        possible_moves |= position << 17 | position << 15 | position << 10 | position << 6 | position >> 10 | position >> 6;

    // 8 Felder erreichbar

    if (position & 0x00003C3C3C3C0000)
        possible_moves |= position << 17 | position << 15 | position << 10 | position << 6 | position >> 6 | position >> 10| position >> 15 | position >> 17;

    return possible_moves;
}

__uint64_t filter_occupied_moves(board_state* state, __uint64_t moves) {
    if (state->player == 1) {
        moves &= ~state->white;
    } else {
        moves &= ~state->black;
    }
    return moves;
}

int get_row(__uint64_t position) {
    int row = log2(position) /8;
    //printf("Zeile: %i\n", row);
    return row;
}

int get_col(__uint64_t position) {
    int col = (int) log2(position) % 8;
    //printf("Spalte: %i\n", col);
    return col;
}

void print_binary(__uint64_t value) {
    int row, col = 0;

    for (row = 7; row >= 0; row--) {
        printf("%i | ", row +1);
        for (col = 0; col < 8; col++) {
            __uint64_t bit = (value >> (row * 8 + col)) & 1;
            printf("%lu ", bit);
        }
        printf("\n");
    }
    printf("  | A B C D E F G H \n");
}

int evaluate_board_state(board_state* state) {
    __uint64_t hill = 0x0000001818000000; //four middle fields

    int value = 0;
    for (int i = 0; i < 6; i++) {
        //get pieces of both players
        __uint64_t white = state->pieces[i] & state->white;
        __uint64_t black = state->pieces[i] & state->black;
        int count = 0;

        //count white pieces
        while (white)
        {
            white &= white - 1;
            count++;
        }

        //simply substract the piece count of black
        while (black)
        {
            black &= black - 1;
            count--;
        }


        value += count * PIECE_VALUES[i];
    }

    //rate hill
    if (state->pieces[KING] & state->white & hill) {
        value += 10000;
    } else if (state->pieces[KING] & state->black & hill) {
        value -= 10000;
    }

    return value;
}

void print_moves_of_piece(board_state* state, int type, __uint64_t pos) {
    __uint64_t combined_board;
    if (state->player == 1) {
        combined_board = state->pieces[type] & state->white;
    } else {
        combined_board = state->pieces[type] & state->black;
    }
    if (pos == 0) {
        //no position given. print all
        //loop through bits
        __uint64_t moves = 0;
        for (int bit = 0; bit < 64; bit++) {
            if ((combined_board >> bit) & 1) {
                //get and loop trough all valid moves
                moves = get_all_possible_moves(state, type, (__uint64_t)1 << bit);
                //board_move* move = malloc(sizeof(board_move));
                for (int m_bit = 0; m_bit < 64; m_bit++) {
                    if ((moves >> m_bit) & 1) {
                        printf("%s %s,", uint_pos_to_fen((__uint64_t)1 << bit), uint_pos_to_fen((__uint64_t)1 << m_bit));
                    }
                }
            }
        }
    } else {
        //print moves of given position
        //get and loop trough all valid moves
        __uint64_t moves = get_all_possible_moves(state, type, pos);
        //board_move* move = malloc(sizeof(board_move));
        for (int m_bit = 0; m_bit < 64; m_bit++) {
            if ((moves >> m_bit) & 1) {
                printf("%s,", uint_pos_to_fen((__uint64_t)1 << m_bit));
            }
        }
    }
    printf("\n");
}

__uint64_t check_castling(board_state* state) {
    //get saved castling informations
    __uint64_t castling = 0;
    if (state->player == 1) {
        //check left
        if (!((state->white | state->black) & 0x000000000000000E) && state->castling & 0x0000000000000004) {
            castling |= 0x0000000000000004; //c1
        }
        //check right
        if (!((state->white | state->black) & 0x0000000000000060) && state->castling & 0x0000000000000040) {
            castling |= 0x0000000000000040; //g1
        }
    } else {
        //check left
        if (!((state->white | state->black) & 0x0E00000000000000) && state->castling & 0x0400000000000000) {
            castling |= 0x0400000000000000; //c8
        }
        //check right
        if (!((state->white | state->black) & 0x6000000000000000) && state->castling & 0x4000000000000000) {
            castling |= 0x4000000000000000; //g8
        }
    }
    return castling;
}

void detect_and_perforn_castling(board_state* state, board_move* move) {
    if (move->piece != KING)
        return; //not a king. nothing to do here
    
    if (move->to == move->from >> 2) { //castling left
        //clear old pos
        state->pieces[ROOK] &= ~(move->to >> 2);
        state->black &= ~(move->to >> 2);
        state->white &= ~(move->to >> 2);

        //set new pos
        state->pieces[ROOK] |= move->from >> 1;
        if (state->player == 1) {
            state->white |= move->from >> 1;
        } else {
            state->black |= move->from >> 1;
        }
        //update castling
        state->castling &= ~move->to;
    } else if (move->to == move->from << 2) { //castling right
        //clear old pos
        state->pieces[ROOK] &= ~(move->to << 1);
        state->black &= ~(move->to << 1);
        state->white &= ~(move->to << 1);

        //set new pos
        state->pieces[ROOK] |= move->from << 1;
        if (state->player == 1) {
            state->white |= move->from << 1;
        } else {
            state->black |= move->from << 1;
        }
        //update castling
        state->castling &= ~move->to;
    }
}

void update_castling_state(board_state* state) {
    if (state->player == 1) {
        if (!(state->pieces[KING] & 0x0000000000000010)) { //e1
            //king
            state->castling &= ~0x0000000000000044;
        } else {
            //left
            if (!(state->pieces[ROOK] & 0x0000000000000001) || !(state->white & 0x0000000000000001)) { //a1
                state->castling &= ~0x0000000000000004; //c1
            }
            //right
            if (!(state->pieces[ROOK] & 0x0000000000000080) || !(state->white & 0x0000000000000080)) { //h1
                state->castling &= ~0x0000000000000040; //g1
            }            
        }
    } else {
        if (!(state->pieces[KING] & 0x1000000000000000)) { //e8
            //king
            state->castling &= ~0x4400000000000000;
        } else {
            //left
            if (!(state->pieces[ROOK] & 0x0100000000000000) || !(state->black & 0x0100000000000000)) { //a8
                state->castling &= ~0x0400000000000000; //c8
            }
            //right
            if (!(state->pieces[ROOK] & 0x8000000000000000) || !(state->black & 0x8000000000000000)) { //h8
                state->castling &= ~0x4000000000000000; //g8
            }
        }
    }
}