#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "board.h"
#include "game.h"

typedef struct _test_position_state {
    struct _board_state* state;
    __uint64_t moves[6]; //all possible moves
    int move_counts[6];
} test_position_state;

typedef struct _test_alpha_beta_state {
    struct _board_state* state;
    __uint64_t from;
    __uint64_t to;
    __uint8_t depth;
} test_alpha_beta_state;

test_position_state* new_test_pos_state(char* fen, char* pawn_moves, char* bishop_moves, char* knight_moves, char* rook_moves, char* queen_moves, char* king_moves);

test_alpha_beta_state* new_alpha_beta_state(char* fen, char* from, char* to, __uint8_t depth);

/*
 * Run position test on given index
 */
int test_position(int index);

/*
 * Run alpha beta test on given index
 */
int test_alpha_beta(int index);

/*
 * Run benchmark on given index
 */
void benchmark_position(int index, int depth);

void print_benchmark_result(char* name, unsigned long time);