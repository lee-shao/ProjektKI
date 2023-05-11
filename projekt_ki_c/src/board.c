#include "board.h"

const char PIECE_CHARS[] = {'P', 'B', 'N', 'R', 'Q', 'K'};

void print_board(board_state* pos) {
    printf(" | A| B| C| D| E| F| G| H|\n8|");
    for (int i = 0; i < 64; i++) {
        //print curr field
        char player = ' ';
        char piece = ' ';
        int bit_pos = 63 - i; //(i / 8) * 8 + 7 - (i % 8); // ahh what is this xD

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
    //TODO: support full fen string with player info etc.

    //alloc board
    board_state *board = malloc(sizeof(board_state));
    memset(board, 0, sizeof(board_state));

    //iterate over fen string
    int fen_len = strlen(fen);
    int bit_pos = 63;
    for (int i = 0; i < fen_len && bit_pos >= 0; i++) {
        if (fen[i] >= '0' && fen[i] <= '9') { //if is number
            bit_pos -= fen[i] - '0'; //skip x amount of fields
        } else if (fen[i] != '/') {
            if (get_player_from_fen(fen[i]) == -1) { //check for lower case: black
                board->black |= (__uint64_t)1 << bit_pos;
            } else { //white instead
                board->white |= (__uint64_t)1 << bit_pos;
            }

            //set piece type
            int type = 0;
            if ((type = get_piece_from_fen(fen[i])) != -1) {
                board->pieces[type] |= (__uint64_t)1 << bit_pos;
                bit_pos--;
            }
        } else {
            //TODO: use '/' to try to fix wrongly formated fen strings
        }
    }

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
            to = (__uint64_t)1 << (7 - (toupper(fen[0]) - 'A') + (fen[1] - '1') * 8);
            break;
        case 3: //piece type and target coords given
            piece_type = get_piece_from_fen(fen[0]); //get piece type
            to = (__uint64_t)1 << (7 - (toupper(fen[1]) - 'A') + (fen[2] - '1') * 8);
            break;
        case 4: //piece type row or column and coords given
            piece_type = get_piece_from_fen(fen[0]); //get piece type
            if (toupper(fen[1]) >= 'A' && toupper(fen[1]) <= 'Z') {
                from_column = 7 - (toupper(fen[1]) - 'A');
            } else {
                from_row = (fen[1] - '1');
            }
            to = (__uint64_t)1 << (7 - (toupper(fen[2]) - 'A') + (fen[3] - '1') * 8);
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
    //note: maybe give player as argument?
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
    }

    //update player
    state->player *= -1;

    return 0; //move successful
}

board_state* get_all_possible_moves(int piecetype, board_move* move) {
    __uint64_t possible_moves;
    __uint64_t f = move->from;
    __uint64_t a_col = 0x7F7F7F7F7F7F7F7F;
    __uint64_t h_col = 0xFEFEFEFEFEFEFEFE;
    __uint64_t top_row = 0xFF00000000000000;
    __uint64_t bot_row = 0x00000000000000FF;

    switch (piecetype)
    {
    case PAWN:
        // Either pawn is at left/right border or between
        // TODO: extra rules (attacking, double step, promotion, en-passant)
        if (f & a_col) {
            possible_moves = f<<8 || f<<7;
        } else if (f & h_col) {
            possible_moves = f<<9 || f<<8;
        } else {
            possible_moves = f<<9 || f<<8 || f<<7;
        }
        return possible_moves;
    case BISHOP:
        if (f & a_col) {
            // 1 = diagonally top right, 2 = diagonally bottom right
            possible_moves = linear_movement(1, move) || linear_movement(2, move);
        } else if (f & h_col) {
            // 3 = diagonally top left, 4 = diagonally bottom left
            possible_moves |= linear_movement(3, move) || linear_movement(4, move);
        } else {
            // all diagonal directions
            possible_moves = linear_movement(1, move) || linear_movement(2, move) || linear_movement(3, move) || linear_movement(4, move);
        }
        return possible_moves;
    case KNIGHT:
        return possible_moves;
    case ROOK:
        // 5 = left, 6 = right, 7 = top, 8 = bottom
        if (f & a_col & top_row) {
            possible_moves = linear_movement(6, move) || linear_movement(8, move);
        } else if (f & h_col & top_row) {
            possible_moves = linear_movement(5, move) || linear_movement(8, move);
        } else if (f & top_row) {
            possible_moves = linear_movement(5, move) || linear_movement(6, move) || linear_movement(8, move);
        } else if (f & a_col & bot_row) {
            possible_moves = linear_movement(6, move) || linear_movement(7, move);
        } else if (f & h_col & bot_row) {
            possible_moves = linear_movement(5, move) || linear_movement(7, move);
        } else if (f & bot_row) {
            possible_moves = linear_movement(5, move) || linear_movement(6, move) || linear_movement(7, move);
        } else {
            possible_moves = linear_movement(5, move) || linear_movement(6, move) || linear_movement(7, move) || linear_movement(8, move);
        }
        return possible_moves;
    case QUEEN:
        return possible_moves;
    case KING:
        // If the king is at the left/right border
        if (f & a_col) {
            possible_moves = f<<7 || f<<8 || f>>1 || f>>8 || f>> 9;
        } else if (f & h_col) {
            possible_moves = f<<9 || f<<8 || f<<1 || f>>8 || f>> 7;
        } else {
            possible_moves = f<<9 || f<<8 || f<<7 || f<<1 || f>>1 || f>>7 || f>>8 || f>>9;
        }
        return possible_moves;
    default:
        return -1; //invalid piecetype
    }
}


__uint64_t diagonal_movement(board_move* move, __uint64_t occupied) {
    __uint64_t possible_moves;
    __uint64_t position = move->from;
    __uint64_t top_left_to_bottom_right = 0x8040201008040201;
    __uint64_t bottom_left_to_top_right = 0x0102040810204080;
    __uint64_t mask;
    int row = get_row(position); // Zeile
    int col = get_col(position); // Spalte

    /*diagonally top right*/
    mask = (top_left_to_bottom_right >> ((7 - col) + 8 * (7 - row))); //& ~occupied;
    possible_moves |= mask << ((7 - col) + 8 * (7 - row));

    /*diagonally bottom right*/
    mask = (bottom_left_to_top_right >> ((7 - col) + 8 * row)); //& ~occupied;
    possible_moves |= mask << ((7 - col) + 8 * row);

    /*diagonally top left*/
    mask = (bottom_left_to_top_right >> (col + 8 * (7 - row))); //& ~occupied;
    possible_moves |= mask << (col + 8 * (7 - row));

    /*diagonally bottom left*/
    mask = (top_left_to_bottom_right >> (col + 8 * row)); //& ~occupied;
    possible_moves |= mask << (col + 8 * row);
}

__uint64_t straight_movement(board_move* move, __uint64_t occupied) {
    __uint64_t possible_moves;
    __uint64_t position = move->from;
    __uint64_t top = 0xFF00000000000000;
    __uint64_t bottom = 0x00000000000000FF;
    __uint64_t left = 0x0101010101010101;
    __uint64_t right = 0x8080808080808080;
    __uint64_t mask;
    int row = get_row(position); // Zeile
    int col = get_col(position); // Spalte

    /*top*/
    mask = (top >> (8 * (7 - row))); //& ~occupied;
    possible_moves |= mask << (8 * (7 - row));

    /*bottom*/
    mask = (bottom << (8 * row)); //& ~occupied;
    possible_moves |= mask << (8 * row);

    /*left*/
    mask = (left >> col); //& ~occupied;
    possible_moves |= mask << col;

    /*right*/
    mask = (right << (7 - col)); //& ~occupied;
    possible_moves |= mask << col;

    return possible_moves;
}

/**
__uint64_t linear_movement(int direction, board_move* move) {
    __uint64_t line;
    __uint64_t f = move->from;
    __uint64_t tmp = f;

    switch (direction)
    {
    //diagonally top right
    case 1:
        for (int i = 0; i<7; i++) {
            tmp = tmp<<7;
            line |= tmp;
        }
        break;
    //diagonally bottom right
    case 2:
        for (int i = 0; i<7; i++) {
            tmp = tmp>>9;
            line |= tmp;
        }
        break;
    //diagonally top left
    case 3:
        for (int i = 0; i<7; i++) {
            tmp = tmp<<9;
            line |= tmp;
        }
        break;
    //diagonally bottom left
    case 4:
        for (int i = 0; i<7; i++) {
            tmp = tmp>>7;
            line |= tmp;
        }
        break;
    //straight left
    case 5:
        for (int i = 0; i<7; i++) {
            tmp = tmp<<1;
            line |= tmp;
        }
        break;
    //straight right
    case 6:
        for (int i = 0; i<7; i++) {
            tmp = tmp>>1;
            line |= tmp;
        }
        break;
    //straight top
    case 7:
        for (int i = 0; i<7; i++) {
            tmp = tmp<<8;
            line |= tmp;
        }
        break;
    //straight bottom
    case 8:
        for (int i = 0; i<7; i++) {
            tmp = tmp>>8;
            line |= tmp;
        }
        break;
    return line;
    }
}
**/

