#include <time.h>
#include <sys/time.h>
#include <unistd.h>

#include "game.h"
#include "transposition_table.h"
#include <sys/random.h>

int game_phase = PRE_GAME;
//board_move* alpha_move = NULL;
int state_count = 0;
int disable_cutoff = 0;
int disable_transposition = 0;
pthread_mutex_t thread_state_lock;
__uint64_t zobrist_key_rands[64][12];
__uint64_t zobrist_white = 0;

__uint64_t get_rand_64() {
    __uint64_t random = 0;
    //for (int i = 0; i < 8; i++) {
    //    random = (random << 8) + (rand() % 255);
    //}
    getrandom(&random, sizeof(__uint64_t), 0);

    //printf("%llu\n", random);
    //print_binary(random);

    return random;
}

void game_init() {
    //init lock
    pthread_mutex_init(&thread_state_lock, NULL);

    //init zobrist_key_rands
    for (int i = 0; i < 64; i++) {
        for (int j = 0; j < 12; j++) {
            zobrist_key_rands[i][j] = get_rand_64();
        }
    }

    zobrist_white = get_rand_64();

    trans_t = (transposition_table **)calloc(1, sizeof(transposition_table *));
    *trans_t = NULL;
}

__uint64_t generate_zobrist_key(board_state* state) {
    __uint64_t key = 0;
    for (int bit = 0; bit < 64; bit++) {
        if (((state->black | state->white) >> bit) & 1) {
            for (int piece = 0; piece < 6; piece++) {
                if ((state->pieces[piece] >> bit) & 1) {
                    if ((state->black >> bit) & 1) {
                        key = key ^ zobrist_key_rands[bit][piece]; //black
                    } else {
                        key = key ^ zobrist_key_rands[bit][6 + piece]; //white
                    }
                    break;
                }
            }
        }
    }

    if (state->player == 1) {
        key ^= zobrist_white;
    }

    return key;
}

int alpha_beta_neg(board_state* state, int alpha, int beta, __uint8_t depth, __uint8_t max_depth) {
    state_count++; //NOTE: NOT thread safe

    //board_move* best_move = calloc(1, sizeof(board_move));
    __uint64_t zobrist_key = 0;
    if (!disable_transposition) {
        zobrist_key = generate_zobrist_key(state);
        //read transposition_table
        transposition_table *pos = tt_get_position(trans_t, zobrist_key, state->player);
        if (pos != NULL && pos->depth >= max_depth - depth) {
            //printf("HASH\n");
            if (pos->type == 0) {
                hash_hits++;
                return pos->score;
            } else if (pos->type == 1 && pos->score <= alpha) {
                return alpha;
            } else if (pos->type == 2 && pos->score >= beta) {
                return beta;
            }
        }
    }
    
    if (depth >= max_depth) {
        int eval = evaluate_board_state(state);
        if (state->player == -1) {
            eval = -eval;
        }
        return eval;

    }

    int score = -999999;
    int score_increased = 0;
    
    __uint64_t combined_board;
    if (state->player == 1) {
        combined_board =  state->white;
    } else {
        combined_board = state->black;
    }


    __uint64_t moves = 0;
    //loop through bits
    for (int bit = 0; bit < 64; bit++) {
        //combine with curr player
        if ((combined_board >> bit) & 1) {
            //get and loop trough all valid moves
            //loop through figures
            for (int piece = 0; piece < 6; piece++) {
                if (((combined_board & state->pieces[piece]) >> bit) & 1) {
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
                            score = -alpha_beta_neg(clone, -beta, -alpha, depth + 1, max_depth);

                            //if (best_move->to == 0 || (score > best_move->score && state->player == 1) || (score < best_move->score && state->player == -1)) {
                            //    best_move->from = (__uint64_t)1 << bit;
                            //    best_move->to = (__uint64_t)1 << m_bit;
                            //    best_move->piece = piece;
                            //    best_move->score = score;
                            //}
                            free(clone); //we don't need the clone any more

                            if (disable_cutoff) //NOTE: NOT thread safe
                                continue;

                            if (score >= beta) {
                                if (!disable_transposition)
                                    tt_set_position(trans_t, zobrist_key, state->player, beta, max_depth - depth, 2);
                                return beta;
                            }
                            if (score > alpha) {
                                score_increased = 1;
                                alpha = score;
                            }
                        }
                    }
                    //free(best_move);
                    break;
                }

            }
        }
    }

    if (!disable_transposition) {
        if (score_increased)
            tt_set_position(trans_t, zobrist_key, state->player, alpha, max_depth - depth, 0);
        else
            tt_set_position(trans_t, zobrist_key, state->player, alpha, max_depth - depth, 1);
    }

    //free(best_move);
    return alpha;

}

int alpha_beta_recursive(board_state* state, int alpha, int beta, __uint8_t depth, __uint8_t max_depth) {
    state_count++; //NOTE: NOT thread safe

    //board_move* best_move = calloc(1, sizeof(board_move));
    __uint64_t zobrist_key = generate_zobrist_key(state);

    //read transposition_table
    transposition_table *pos = tt_get_position(trans_t, zobrist_key, state->player);
    if (pos != NULL && pos->depth >= max_depth - depth) {
        //printf("HASH\n");
        if (pos->type == 0) {
            hash_hits++;
            return pos->score;
        } else if (pos->type == 1) {
            //printf("ALPHA\n");
            if (state->player == 1) {
                if (pos->score <= alpha) {
                    hash_hits++;
                    return alpha;
                }
            } else {
                if (pos->score >= beta) {
                    hash_hits++;
                    return beta;
                }
            }
        } else if (pos->type == 2) {
            //printf("BETA\n");
            if (state->player == 1) {
                if (pos->score >= beta) {
                    hash_hits++;
                    return beta;
                }
            } else {
                if (pos->score <= alpha) {
                    hash_hits++;
                    return alpha;
                }
            }         
        }
    }



    if (depth >= max_depth) {
        int eval = evaluate_board_state(state);
        return eval;

    }

    int score = 0;
    int score_increased = 0;
    
    __uint64_t combined_board;
    if (state->player == 1) {
        combined_board =  state->white;
    } else {
        combined_board = state->black;
    }


    __uint64_t moves = 0;
    //loop through bits
    for (int bit = 0; bit < 64; bit++) {
        //combine with curr player
        if ((combined_board >> bit) & 1) {
            //get and loop trough all valid moves
            //loop through figures
            for (int piece = 0; piece < 6; piece++) {
                if (((combined_board & state->pieces[piece]) >> bit) & 1) {
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

                            //if (best_move->to == 0 || (score > best_move->score && state->player == 1) || (score < best_move->score && state->player == -1)) {
                            //    best_move->from = (__uint64_t)1 << bit;
                            //    best_move->to = (__uint64_t)1 << m_bit;
                            //    best_move->piece = piece;
                            //    best_move->score = score;
                            //}
                            free(clone); //we don't need the clone any more

                            if (disable_cutoff) //NOTE: NOT thread safe
                                continue;
                                
                            if (state->player == 1) {
                                if (score > alpha) {
                                    alpha = score;
                                    if (alpha >= beta) {
                                        //free(best_move);
                                        tt_set_position(trans_t, zobrist_key, state->player, score, max_depth - depth, 2);
                                        return score;
                                    }
                                    //tt_set_position(trans_t, zobrist_key, score, max_depth - depth, 0);
                                    score_increased = 1;
                                }
                            } else {
                                if (score < beta) {
                                    beta = score;
                                    if (beta <= alpha) {
                                        //free(best_move);
                                        tt_set_position(trans_t, zobrist_key, state->player, score, max_depth - depth, 2);
                                        return score;
                                    }
                                    score_increased = 1;
                                    //tt_set_position(trans_t, zobrist_key, score, max_depth - depth, 0);
                                }
                            }
                        }
                    }
                    //free(best_move);
                    break;
                }

            }
        }
    }


    //free(best_move);
    if (state->player == 1) {
        if (score_increased)
            tt_set_position(trans_t, zobrist_key, state->player, alpha, max_depth - depth, 0);
        else
            tt_set_position(trans_t, zobrist_key, state->player, alpha, max_depth - depth, 1);
        return alpha;
    }

    if (score_increased)
        tt_set_position(trans_t, zobrist_key, state->player, beta, max_depth - depth, 0);
    else
        tt_set_position(trans_t, zobrist_key, state->player, beta, max_depth - depth, 1);
    return beta;
}

board_move* get_best_known_move_in_depth(board_state* state, int depth) {
    board_move* move = calloc(1, sizeof(board_move));
    int highest_score = 0;
    int score = 0;

    __uint64_t combined_board;
    if (state->player == 1) {
        combined_board =  state->white;
    } else {
        combined_board = state->black;
    }

    __uint64_t moves = 0;
    //loop through bits
    for (int bit = 0; bit < 64; bit++) {
        //combine with curr player
        if ((combined_board >> bit) & 1) {
            //get and loop trough all valid moves
            //loop through figures
            for (int piece = 0; piece < 6; piece++) {
                if (((combined_board & state->pieces[piece]) >> bit) & 1) {
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
                            score = -alpha_beta_neg(clone, -999999, 999999, 0, depth); //alpha_beta_recursive(clone, -999999, 999999, 0, depth);
                            //invert score for black
                            //if (state->player == -1) {
                            //    score = -score;
                            //}
                            if (score > highest_score || move->to == 0) { // || (score >= highest_score && rand() % 100 > 95)
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
                    break;
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
    return best_move;
}

__uint64_t get_micros() {
    struct timeval time;
    gettimeofday(&time,NULL);
    return time.tv_sec*(__uint64_t)1000000+time.tv_usec;
}