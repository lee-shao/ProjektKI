#include "game.h"

int game_phase = PRE_GAME;
board_move* alpha_move = NULL;

int alpha_beta_recursive(board_state* state, int alpha, int beta, __uint8_t depth, __uint8_t max_depth) {
    if (depth >= max_depth) {
        return evaluate_board_state(state);

    }

    int score = 0;
    
    //loop through figures
    for (int piece = 0; piece < 6; piece++) {
        //combine with curr player
        __uint64_t combined_board;
        if (state->player == 1) {
            combined_board = state->pieces[piece] & state->white;
        } else {
            combined_board = state->pieces[piece] & state->black;
        }
        //loop through bits
        __uint64_t moves = 0;
        for (int bit = 0; bit < 64; bit++) {
            if ((combined_board >> bit) & 1) {
                //get and loop trough all valid moves
                moves = get_all_possible_moves(state, piece, (__uint64_t)1 << bit);
                //board_move* move = malloc(sizeof(board_move));
                for (int m_bit = 0; m_bit < 64; m_bit++) {
                    if ((moves >> m_bit) & 1) {
                        board_state* clone = clone_board_state(state);
                        alpha_move->from = (__uint64_t)1 << bit;
                        alpha_move->to = (__uint64_t)1 << m_bit;
                        alpha_move->piece = piece;
                        perform_move(clone, alpha_move);
                        score = alpha_beta_recursive(clone, alpha, beta, depth + 1, max_depth);
                        free(clone); //we don't need the clone any more

                        if (state->player == 1) {
                            if (score > alpha) {
                                alpha = score;
                                if (alpha >= beta) {
                                    //free(move);
                                    return score;
                                }
                            }
                        } else {
                            if (score < beta) {
                                beta = score;
                                if (beta <= alpha) {
                                    //free(move);
                                    return score;
                                }
                            }
                        }
                    }
                }
                //free(move);

            }
        }
    }

    //free(move);
    if (state->player == 1) {
        return alpha;
    }

    return beta;
}

board_move* get_best_known_move(board_state* state, int timeout) {
    alpha_move = malloc(sizeof(board_move));
    board_move* move = malloc(sizeof(board_move));
    int highest_score = 0;
    int score = 0;
    for (int piece = 0; piece < 6; piece++) {
        //combine with curr player
        __uint64_t combined_board;
        if (state->player == 1) {
            combined_board = state->pieces[piece] & state->white;
        } else {
            combined_board = state->pieces[piece] & state->black;
        }
        //loop through bits
        __uint64_t moves = 0;
        for (int bit = 0; bit < 64; bit++) {
            if ((combined_board >> bit) & 1) {
                //get and loop trough all valid moves
                moves = get_all_possible_moves(state, piece, (__uint64_t)1 << bit);
                //board_move* move = malloc(sizeof(board_move));
                for (int m_bit = 0; m_bit < 64; m_bit++) {
                    if ((moves >> m_bit) & 1) {
                        board_state* clone = clone_board_state(state);
                        board_move* new_move = malloc(sizeof(board_move));
                        new_move->from = (__uint64_t)1 << bit;
                        new_move->to = (__uint64_t)1 << m_bit;
                        new_move->piece = piece;
                        perform_move(clone, new_move);
                        score = alpha_beta_recursive(clone, -999999, 999999, 0, timeout); //CHANGE ME should not be timeout
                        if (score > highest_score) {
                            move->from = new_move->from;
                            move->to = new_move->to;
                            move->piece = new_move->piece;
                            move->score = score;
                        }
                        free(clone); //we don't need the clone any more
                        free(new_move);
                    }
                }
            }
        }
    }
    free(alpha_move);
    return move;
}

__uint64_t get_micros() {
    struct timeval time;
    gettimeofday(&time,NULL);
    return time.tv_sec*(__uint64_t)1000000+time.tv_usec;
}