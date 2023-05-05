#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "board.h"

int main(int argc, char **argv) {
    // board_state *np = malloc(sizeof(board_state));
    // memset(np, 0, sizeof(board_state));
    // np->pieces[PAWN] =  0b0000000011011111001000000000000000000000000000001111111100000000;
    // np->pieces[KING] =  0b0100000000000000000000000000000000000000000000000000000000000000;
    // np->black =         0b1111111111111111000000000000000000000000000000000000000000000000;
    // np->white =         0b0000000000000000000000000000000000000000000000001111111111111111;
    board_state *np = fen_to_board("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR"); //"r1b1k1nr/p2p1pNp/n2B4/1p1NP2P/6P1/3P1Q2/P1P1K3/q5b1"
    np->player = 1;

    //simple move interface for testing
    while (1) {
        print_board(np);
        printf("player: %d\n", np->player);

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
                b_move->from =   (__uint64_t)1 << (7 - (toupper(move[0]) - 'A') + (move[1] - '1') * 8);
                b_move->to =     (__uint64_t)1 << (7 - (toupper(move[3]) - 'A') + (move[4] - '1') * 8);

                int move_code = 0;
                if ((move_code = perform_move(np, b_move)) != 0) {
                    printf("invalid move!%d\n", move_code);
                }
                free(b_move);
            } else if (strlen(move) >= 3) {
                board_move* b_move = fen_to_move(move, np);

                int move_code = 0;
                if (b_move == NULL || (move_code = perform_move(np, b_move)) != 0) {
                    printf("invalid move!%d\n", move_code);
                }
                free(b_move);
            } else if (strlen(move) >= 1) {
                if (move[0] == 'q') {
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