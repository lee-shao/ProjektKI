#include "board.h"
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>

//TODO all game related stuff here (winning etc.)

enum game_phases {PRE_GAME = 0, OPENING, MIDGAME, WIN = 20, LOOSE = 21};
extern int game_phase;
extern board_move* alpha_move;

__uint64_t get_micros();

int alpha_beta_recursive(board_state* state, int alpha, int beta, __uint8_t depth, __uint8_t max_depth);

board_move* get_best_known_move(board_state* state, int timeout);