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

    print_board(np);

    free(np);

    return 0;
}