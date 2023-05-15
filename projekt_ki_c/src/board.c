#include "board.h"

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

    return 0; //move successful
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