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
    //TODO: implement
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
            if (fen[i] >= 'a' && fen[i] <= 'z') { //check for lower case: black
                board->black |= (__uint64_t)1 << bit_pos;
            } else { //white instead
                board->white |= (__uint64_t)1 << bit_pos;
            }

            //set piece type
            switch (toupper(fen[i]))
            {
            case 'P':
                board->pieces[PAWN] |= (__uint64_t)1 << bit_pos;
                break;
            case 'B':
                board->pieces[BISHOP] |= (__uint64_t)1 << bit_pos;
                break;
            case 'N':
                board->pieces[KNIGHT] |= (__uint64_t)1 << bit_pos;
                break;
            case 'R':
                board->pieces[ROOK] |= (__uint64_t)1 << bit_pos;
                break;
            case 'Q':
                board->pieces[QUEEN] |= (__uint64_t)1 << bit_pos;
                break;
            case 'K':
                board->pieces[KING] |= (__uint64_t)1 << bit_pos;
                break;
            default:
                bit_pos++; //wrong character nothing to do
                break;
            }
            bit_pos--;
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