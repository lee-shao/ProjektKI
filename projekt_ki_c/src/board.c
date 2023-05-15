#include "board.h"
#include <inttypes.h>
#include <math.h>

const char PIECE_CHARS[] = {'P', 'B', 'N', 'R', 'Q', 'K'};
const int PIECE_VALUES[] = {10, 30, 30, 50, 90, 20000};

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
                board->castling |= (__uint64_t)1 << (('C' - 'A'));
                break;
            case 'Q':
                board->castling |= (__uint64_t)1 << (('G' - 'A'));
                break;
            case 'k':
                board->castling |= (__uint64_t)1 << (('C' - 'A') + 7 * 8);
                break;
            case 'q':
                board->castling |= (__uint64_t)1 << (('G' - 'A') + 7 * 8);
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

    //TODO: get possible moves and iterate through them
    for (int bit_pos = 0; bit_pos < 64; bit_pos++) {
        __uint64_t player_board = state->black;
        if (state->player == 1)
            player_board = state->white;
        if (state->pieces[piece_type] & player_board & (__uint64_t)1 << bit_pos) {
            //TODO: get piece that can perform the move
            //CHANGE ME this is a dummy implementation
            if ((from_column == -1 || bit_pos % 8 == from_column) && (from_row == -1 || bit_pos / 8 == from_row)) { //only search for moves in from row or column if given
                move->from = (__uint64_t)1 << bit_pos;
                break;
            }
        }
    }

    move->to = to;
    move->piece = piece_type;

    return move;
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

    //perform move
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

    /*
    //Nicht löschen, brauche noch!
    int pt = KING;
    __uint64_t test = get_all_possible_moves(pt, move->from);
    printf("%" PRIu64 "\n", move->from);
    print_binary(test);
    printf("\n");
    */

    return 0; //move successful
}

/**
 * Diese Funktion berechnet für eine beliebige Figur das Bewegungsmuster und gibt diese in einem Bitboard aus
 * @param piecetype Gibt den Figurentyp an, für den das Bewegungsmuster berechnet werden soll
 * @param move enthält die Position, auf die sich die Figur befindet
 * @return Gibt das Bewegungsmusters einer Figur als ein 64-Bit-Integer aus
 * @author Shao
*/
__uint64_t get_all_possible_moves(int piecetype, __uint64_t f) {
    __uint64_t possible_moves = 0;
    __uint64_t a_col = 0x7F7F7F7F7F7F7F7F;
    __uint64_t h_col = 0xFEFEFEFEFEFEFEFE;

    switch (piecetype)
    {
    /**
    * Either pawn is at left/right border or between
    * TODO: extra rules (attacking, double step, promotion, en-passant)
    */
    case 0:
        if (f & ~a_col) {
            possible_moves |= f<<8 | f<<7;
        } else if (f & ~h_col) {
            possible_moves |= f<<9 | f<<8;
        } else {
            possible_moves |= f<<9 | f<<8 | f<<7;
        }
        return possible_moves;
    case BISHOP:
        possible_moves |= diagonal_movement(f, 0ULL);
        return possible_moves;
    case KNIGHT:
        //possible_moves = knight_movement(f, 0ULL);
        return possible_moves;
    case ROOK:
        possible_moves |= straight_movement(f, 0ULL);
        return possible_moves;
    case QUEEN:
        possible_moves |= diagonal_movement(f, 0ULL) | straight_movement(f, 0ULL);
        return possible_moves;
    case KING:
        // If the king is at the left/right border
        if (f & ~a_col) {
            possible_moves |= f<<7 | f<<8 | f>>1 | f>>8 | f>> 9;
        } else if (f & ~h_col) {
            possible_moves |= f<<9 | f<<8 | f<<1 | f>>8 | f>> 7;
        } else {
            possible_moves |= f<<9 | f<<8 | f<<7 | f<<1 | f>>1 | f>>7 | f>>8 | f>>9;
        }
        return possible_moves;
    default:
        return -1; //invalid piecetype
    }
}

/**
 * Diese Funktion berechnet ausgehend von einer Position alle diagonal erreichbaren Felder bis zum Spielbrettrand. 
 * Dafür werden 2 Diagonale an die Position der Figur verschoben, um die Felder abzudecken, die die Figur in 4 Richtungen laufen kann. 
 * TODO: In occupied kann ein 64-Bitboard gespeichert werden, wo bereits Figuren stehen.
 * Das Invertierte Bitboard von occupied kann dann verUNDed werden um besetzte Felder auszuschließen. 
 * @author Shao
*/
__uint64_t diagonal_movement(__uint64_t position, __uint64_t occupied) {
    __uint64_t possible_moves = 0;
    int row = get_row(position); // Zeile
    int col = get_col(position); // Spalte
    int i = 0;
    
    //links oben
    for (i = 1; row - i >= 0 && col - i >= 0; i++) {
        possible_moves |= 1ULL << ((row - i) * 8 + (col - i));
    }
    //rechts oben
    for (i = 1; row - i >= 0 && col + i < 8; i++) {
        possible_moves |= 1ULL << ((row - i) * 8 + (col + i));
    }
    //links unten
    for (i = 1; row + i < 8 && col - i >= 0; i++) {
        possible_moves |= 1ULL << ((row + i) * 8 + (col - i));
    }
    //rechts unten
    for (i = 1; row + i < 8 && col + i < 8; i++) {
        possible_moves |= 1ULL << ((row + i) * 8 + (col + i));
    }

    return possible_moves;
}

/**
 * Diese Funktion berechnet ausgehend von einer Position alle gerade erreichbaren Felder bis zum Spielbrettrand. 
 * Dafür werden Geraden an die Position der Figur verschoben, um die Felder abzudecken, die die Figur in 4 Richtungen laufen kann. 
 * TODO: In occupied kann ein 64-Bitboard gespeichert werden, wo bereits Figuren stehen.
 * Das Invertierte Bitboard von occupied kann dann verUNDed werden um besetzte Felder auszuschließen. 
 * @author Shao
*/
__uint64_t straight_movement(__uint64_t position, __uint64_t occupied) {
    __uint64_t possible_moves = 0;
    __uint64_t top = 0xFF00000000000000ULL;
    __uint64_t bottom = 0x00000000000000FFULL;
    __uint64_t left = 0x0101010101010101ULL;
    __uint64_t right = 0x8080808080808080ULL;
    __uint64_t mask = 1ULL << position;
    int row = get_row(position); // Zeile
    int col = get_col(position); // Spalte

    //top
    mask = (top >> (8 * (7 - row))); //& ~occupied;
    possible_moves |= mask; // << (8 * (7 - row));
    //bottom
    mask = (bottom << (8 * row)); //& ~occupied;
    possible_moves |= mask; // << (8 * row);
    //left
    mask = (left << col); //& ~occupied;
    possible_moves |= mask; // << col;
    //right
    mask = (right >> (7 - col)); //~occupied;
    possible_moves |= mask;// << col;

    possible_moves &= ~position;

    return possible_moves;
}


/**
 * Bitmaske für den Springer
 * 
*/
/*
__uint64_t knight_movement(__uint64_t position, __uint64_t occupied) {
    __uint64_t possible_moves = 0;
    __uint64_t mask;
    __uint64_t tmp = position;
    int row = get_row(position);
    int col = get_col(position);

    return 0; //possible_moves;
}
*/

int get_row(__uint64_t position) {
    int row = log2(position) /8;
    printf("Zeile: %i\n", row);
    return row;
}

int get_col(__uint64_t position) {
    int col = (int) log2(position) % 8;
    printf("Spalte: %i\n", col);
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
    return value;
}