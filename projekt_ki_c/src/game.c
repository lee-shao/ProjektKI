#include <time.h>
#include <sys/time.h>
#include <unistd.h>
#include <math.h>

#include "game.h"

int game_phase = PRE_GAME;
//board_move* alpha_move = NULL;
int state_count = 0;
int disable_cutoff = 0;
pthread_mutex_t thread_state_lock;

int time_limit = 300;
float time_spent_black = 0;
float time_spent_white = 0;

int alpha_beta_recursive(board_state* state, int alpha, int beta, __uint8_t depth, __uint8_t max_depth) {
    state_count++; //NOTE: NOT thread safe
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
                        board_move* alpha_move = malloc(sizeof(board_move));
                        alpha_move->from = (__uint64_t)1 << bit;
                        alpha_move->to = (__uint64_t)1 << m_bit;
                        alpha_move->piece = piece;
                        perform_move(clone, alpha_move);
                        free(alpha_move);
                        score = alpha_beta_recursive(clone, alpha, beta, depth + 1, max_depth);
                        free(clone); //we don't need the clone any more

                        if (disable_cutoff) //NOTE: NOT thread safe
                            continue;
                            
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

board_move* get_best_known_move_in_depth(board_state* state, int depth) {
    board_move* move = calloc(1, sizeof(board_move));
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
                //moves = filter_check(state, (__uint64_t)1 << bit, piece, moves);
                //board_move* move = malloc(sizeof(board_move));
                for (int m_bit = 0; m_bit < 64; m_bit++) {
                    if ((moves >> m_bit) & 1) {
                        board_state* clone = clone_board_state(state);
                        board_move* new_move = malloc(sizeof(board_move));
                        new_move->from = (__uint64_t)1 << bit;
                        new_move->to = (__uint64_t)1 << m_bit;
                        new_move->piece = piece;
                        perform_move(clone, new_move);
                        score = alpha_beta_recursive(clone, -999999, 999999, 0, depth);
                        //invert score for black
                        if (state->player == -1) {
                            score = -score;
                        }
                        if (score > highest_score || move->to == 0 || (score >= highest_score && rand() % 100 > 95)) {
                            move->from = new_move->from;
                            move->to = new_move->to;
                            move->piece = new_move->piece;
                            move->score = score;
                            highest_score = score;
                        }
                        free(clone); //we don't need the clone any more
                        free(new_move);
                    }
                }
            }
        }
    }
    return move;
}

void* best_move_thread(void* arg) {
    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
    pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);

    thread_state* state = (thread_state*) arg;
    board_move* generated_move = get_best_known_move_in_depth(state->state, state->depth);
    pthread_mutex_lock(&thread_state_lock);
    state->generated_move = generated_move;
    //if (state->should_free) {
    //    //Hacky I know. But couldn't think of another way to do it at the moment 
    //    free(state->state);
    //    free(state);
    //}
    pthread_mutex_unlock(&thread_state_lock);

    //printf("thread finished\n");

    return NULL;
}

board_move* get_best_known_move(board_state* state, int timeout) {
    printf("generating move in %dms\n", timeout);

    __uint64_t start_time = get_micros();
    int curr_depth = -1;
    board_move* best_move = NULL;

    thread_state* t_state = calloc(1, sizeof(thread_state));
    t_state->generated_move = get_best_known_move_in_depth(state, 0); //to make sure we always return a move
    t_state->state = clone_board_state(state);
    pthread_t* search_thread = NULL;
    while ((get_micros() - start_time) / 1000 < timeout) {
        //check if last iteration is finished
        pthread_mutex_lock(&thread_state_lock);
        if (t_state->generated_move != NULL) {
            //update best move
            if (best_move != NULL) {
                free(best_move);
                best_move = NULL;
            }
            best_move = t_state->generated_move;
            if (best_move != NULL)
                printf("curr move %s %s\n", uint_pos_to_fen(best_move->from), uint_pos_to_fen(best_move->to));

            curr_depth += 2;

            //execute search in seperate thread
            t_state->depth = curr_depth;
            
            t_state->generated_move = NULL;
            pthread_mutex_unlock(&thread_state_lock);
            search_thread = malloc(sizeof(pthread_t));
            pthread_create(search_thread, NULL, best_move_thread, t_state);
            //pthread_detach(*search_thread);
        } else {
            pthread_mutex_unlock(&thread_state_lock);
        }

        usleep(10000); //10ms

        //printf("%d\n", (int)(get_micros() - start_time) / 1000);

    }

    pthread_mutex_lock(&thread_state_lock);
    if (t_state->generated_move == NULL && search_thread != NULL) {
        t_state->should_free = 1;
        pthread_cancel(*search_thread); //not finished! cancel thread
        pthread_join(*search_thread, NULL);
    }
    free(t_state->state);
    free(t_state);
    pthread_mutex_unlock(&thread_state_lock);

    //clean up
    free(search_thread);

    printf("generating took %dms\n", (int)(get_micros() - start_time) / 1000);
    if (state->player == 1) {
        time_spent_white += (double)(get_micros() - start_time) / 1000000.0;
        printf("total time spent generating: %d.%ds\n", (int)(time_spent_white), (int)(time_spent_white * 10) % 10);
    } else {
        time_spent_black += (double)(get_micros() - start_time) / 1000000.0;
        printf("total time spent generating: %d.%ds\n", (int)(time_spent_black), (int)(time_spent_black * 10) % 10);

    }
    return best_move;
}

__uint64_t get_micros() {
    struct timeval time;
    gettimeofday(&time,NULL);
    return time.tv_sec*(__uint64_t)1000000+time.tv_usec;
}

float min(float x1, float x2) {
    if (x1 < x2)
        return x1;
    return x2;
}

float max(float x1, float x2) {
    if (x1 > x2)
        return x1;
    return x2;
}

int get_time_for_search(board_state* state) {
    float x = state->full_moves;

    //get time for current player
    float time_spent = time_spent_black;
    if (state->player == 1) {
        time_spent = time_spent_white;
    }

    float fx = 1;
    if (x > 0) //can't run log on 0
        fx = min((time_limit - time_spent) / 15, min(1 + pow((x / (5.0 / (time_limit / 300.0))), 2), 2 + max(min(x / 2, 5), (log10(x/7.0) * time_limit * 0.04 * (time_limit - time_spent) / 400 / (time_limit * 0.002))))) * 1000;
    return fx;
}