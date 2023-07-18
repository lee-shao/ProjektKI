#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>

#include "board.h"
#include "game.h"
#include "network.h"
#include "transposition_table.h"

int network_enabled = 0; //set to one if you want to connect to the server
int self_player = 1; //player you want to be. CHANGE ME should be somewhere else!

int main(int argc, char **argv) {

    //get_rand_64();
    //return 0;
    
    board_state *np = fen_to_board("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"); //"rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1" //"r1b1k1nr/p2p1pNp/n2B4/1p1NP2P/6P1/3P1Q2/P1P1K3/q5b1"

    game_init();
    //alpha_move = malloc(sizeof(board_move));

    if (network_enabled) {
        //connect to server if networking is enabled
        if (network_connect("127.0.0.1", "4466") > 0) {
            if (self_player == 1) {
                send(network_socket, "w\n", 2, 0); //white
            } else {
                send(network_socket, "b\n", 2, 0); //black
            }
        } else {
            printf("connection failed!\n");
            network_enabled = 0;
        }
    }

    //simple move interface for testing
    while (1) {
        int board_value = evaluate_board_state(np);
        printf("%s\n", board_to_fen(np));
        print_board(np);
        printf("zobrist_key: %llu\n", generate_zobrist_key(np));
        printf("player: %d, half-moves: %d, full-moves: %d, value: %d\n", np->player, np->half_moves, np->full_moves, board_value);
        board_move* sugg_move = get_best_known_move(np, get_time_for_search(np)); //alpha_beta_recursive(np, -99999, 99999, 0, 4);
        //get_best_known_move(clone_board_state(np), 2000);
        //print_moves_of_piece(np, ROOK, fen_pos_to_uint("h1", 0));
        printf("hash_sets: %d, hash_dels: %d, hash_collisions: %d, hash_hits: %d, states: %d\n", hash_sets, hash_deletes, hash_collisions, hash_hits, state_count);
        if (sugg_move->to != 0 && board_value < 9000 && board_value > -9000) {
            printf("suggested move %s %s - %d\n", uint_pos_to_fen(sugg_move->from), uint_pos_to_fen(sugg_move->to), sugg_move->score);
            //perform_move(np, sugg_move);
        } else {
            printf("game end!\n");
            return 0;
        }
        //sleep(1);
        //continue;
        
        //handle network move
        if (network_enabled) {
            if (np->player != self_player) {
                //receive move
                board_move* b_move = network_listen_for_move(np, network_socket);

                if (b_move != NULL) {
                    //perform move
                    int move_code = 0;
                    if ((move_code = perform_move(np, b_move)) != 0) {
                        printf("invalid move!%d\n", move_code);
                    }
                    free(b_move);
                }
                continue; //skip console read for this loop execution
            }
        }

        //read move accepts move in format: XY*XY examples: A2-A3, B2 B3
        char *move = NULL;
        size_t len = 0;

        if (getline(&move, &len, stdin) != -1) {
            if (strlen(move) >= 6) {
                board_move* b_move = malloc(sizeof(board_move));
                memset(b_move, 0, sizeof(board_move));
                b_move->piece = -1; //unknown piece type
                //convert move coordinates
                //note: does not check for invalid input
                b_move->from =   (__uint64_t)1 << ((toupper(move[0]) - 'A') + (move[1] - '1') * 8);
                b_move->to =     (__uint64_t)1 << ((toupper(move[3]) - 'A') + (move[4] - '1') * 8);

                int move_code = 0;
                if ((move_code = perform_move(np, b_move)) != 0) {
                    printf("invalid move!%d\n", move_code);
                } else {
                    tt_clear(trans_t);
                }
                free(b_move);
            } else if (strlen(move) >= 3) {
                board_move* b_move = fen_to_move(move, np);

                int move_code = 0;
                if (b_move == NULL || (move_code = perform_move(np, b_move)) != 0) {
                    printf("invalid move!%d\n", move_code);
                } else if (network_enabled) {
                    //send move to opponent
                    //note: this doesn't make sure all bytes are sent.
                    send(network_socket, move, strlen(move), 0);
                }
                free(b_move);
            } else if (strlen(move) >= 1) {
                if (move[0] == 'q') {
                    //tt_clear(trans_t);
                    free(move);
                    break;
                }
            }
        }

        free(move);
    }


    free(np);

    return 0;
}