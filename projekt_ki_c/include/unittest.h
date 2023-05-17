#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "board.h"
#include "game.h"

typedef struct _test_position_state {
    struct _board_state* state;
    __uint64_t moves[6][32]; //all possible moves
    int move_counts[6];
} test_position_state;

test_position_state* new_test_pos_state(char* fen, char* pawn_moves, char* bishop_moves, char* knight_moves, char* rook_moves, char* queen_moves, char* king_moves);

/*
 * Run position test on given index
 */
int test_position(int index);

/*
 * Run benchmark on given index
 */
void benchmark_position(int index);

void print_benchmark_result(char* name, unsigned long time);