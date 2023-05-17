#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

enum pieces {PAWN = 0, BISHOP, KNIGHT, ROOK, QUEEN, KING};
extern const char PIECE_CHARS[];
extern const int PIECE_VALUES[];

/*
 * stores the pieces positions on the board
 * pieces: bitboards to store the pieces. Use enum pieces as index
 * black: bitboard to represent black pieces
 * white bitboard to represent white pieces
 * player: -1: black, 1: white
 * en_passant: possible en_passant targets
 * castling: castling possibilities
 * half_moves: moves (of each player) since last pawn move or piece capture
 * full_moves: increments on every black move
 */
typedef struct _board_state {
    //pieces
    __uint64_t  pieces[6];
    //player
    __uint64_t  black;
    __uint64_t  white;
    __int8_t    player;
    //special moves
    __uint64_t  en_passant;
    __uint64_t  castling;
    //clocks
    __uint16_t  half_moves;
    __uint16_t  full_moves;
} board_state;

/*
 * stores one or multipe moves
 * from: position of piece to move
 * to: destination of the move
 * note: please only set one bit each. results in unspecified behaviour otherwise.
 * piece: piece type to move. -1 when unknown
 * next: pointer to next move
 */
typedef struct _board_move {
    __uint64_t  from;
    __uint64_t  to;
    int         piece;
    struct _board_move *next; //make it a linked list
} board_move;

void print_board(board_state* pos);

board_state* fen_to_board(char* fen);
char* board_to_fen(board_state* state);

/*
 * Converts a piece char in fen notation to an array index for the pieces array
 */
int get_piece_from_fen(char fen);

/*
 * Extracts player from fen char (1: white, -1: black)
 */
__int8_t get_player_from_fen(char fen);

/*
 * Converts a fen move string to a board_move
 */
board_move* fen_to_move(char *fen, board_state *state);

/*
 * performs specified move
 */
int perform_move(board_state* state, board_move* move);

/**
 * 
*/
__uint64_t get_all_possible_moves(int piecetype, __uint64_t f);

/**
 * 
*/
__uint64_t diagonal_movement(__uint64_t position, __uint64_t occupied);

/**
 * 
*/
__uint64_t straight_movement(__uint64_t position, __uint64_t occupied);

/**
 * 
*/
__uint64_t knight_movement(__uint64_t position, __uint64_t occupied);

/**
 * 
*/
int insideBoardBounds(int x, int y);

/**
 *
*/
int get_row(__uint64_t position);

/**
 * 
*/
int get_col(__uint64_t position);

/**
 * 
*/
void print_binary(__uint64_t value);

/*
 * Evaluates given board state
 * if positive white is in advantage. if negative black is in advantage
 */
int evaluate_board_state(board_state* state);