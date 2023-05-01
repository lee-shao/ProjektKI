#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

enum pieces {PAWN = 0, BISHOP, KNIGHT, ROOK, QUEEN, KING};
extern const char PIECE_CHARS[];

//stores the pieces positions on the board
typedef struct _board_state {
    //pieces
    __uint64_t pieces[6];
    //player
    __uint64_t black;
    __uint64_t white;
} board_state;

void print_board(board_state* pos);

board_state* fen_to_board(char* fen);
char* board_to_fen(board_state* state);

/*
 * performs specified move
 * from: position of piece to move
 * to: destination of the move
 * note: please only set one bit each. results in unspecified behaviour otherwise.
 */
int perform_move(board_state* state, __uint64_t from, __uint64_t to);