#include "unittest.h"

#define TEST_POS_COUNT 4
test_position_state* test_positions[TEST_POS_COUNT];

int main(int argc, char **argv) {
/*     long unsigned int count = 1000000000;
    __uint64_t start_time = get_micros();

    for (__uint64_t i = 0; i < count; i++) {
        log2(i);
    }

    __uint64_t end_time = get_micros();

    printf("%lu log2 took %ld micro seconds\n", count, (long)((end_time - start_time)));


    start_time = get_micros();

    for (__uint64_t i = 0; i < count; i++) {
        log2_64(i);
    }

    end_time = get_micros();

    printf("%lu log2_64 took %ld micro seconds\n", count, (long)((end_time - start_time)));

    for (long long int i = 1000000; i < 100000000; i += 1) {
        if ((int)log2(i) != log2_64(i)) {
            printf("different at %lld\n", i);
        }
    } */

    test_positions[0] = new_test_pos_state("r3k2r/pp6/2p3Pb/2N1pP2/Q2p4/4P3/PP1K4/7R w q e6 0 34", "a3,b3,b4,e6,f6,g7", "", "d7,e6,e4,d3,b3,a6,b7", "h2,h3,h4,h5,h6,g1,f1,e1,d1,c1,b1,a1", "a5,a6,a7,b5,c6,b4,c4,d4,b3,c2,d1,a3", "d3,e2,e1,d1,c1,c2"); //Stellung 1 Gruppe J
    test_positions[1] = new_test_pos_state("r3k2r/pp2qppp/1np2n2/2bPp1B1/B2P2Q1/2N2N2/PPP2PPP/2KR3R b kq - 0 10", "a6,a5,d4,e4,g6,h6,h7", "d6,d4,b4,a3", "c8,d7,d5,c4,a4,g8,h5,g4,e4,d5,d7", "b8,c8,d8,g8,f8", "f8,e6,d6,d7,c7,d8", "f8,d8"); //Stellung 2 Gruppe J
    test_positions[2] = new_test_pos_state("8/8/4kpp1/3p4/p6P/2B4b/6P1/6K1 w - - 1 48", "h3,h5,g3,g4", "f6,e5,a5,d4,b4,d2,b2,e1,a1", "", "", "", "h2,f2,h1,f1"); //Stellung 1 - Shirov's Bishop Sacrifice - Gruppe AF
    test_positions[3] = new_test_pos_state("5rk1/pp4pp/4p3/2R3Q1/3n4/6qr/P1P2PPP/5RK1 w - - 2 24", "g3,g3,f3,c3,a3,f4,c4,a4", "", "", "c8,c7,c6,f5,e5,d5,b5,a5,c4,c3,e1,d1,c1,b1,a1", "d8,g7,e7,h6,g6,f6,h5,f5,e5,d5,h4,g4,f4,g3,e3,d2,c1", "h1"); //Stellung 2 - Marshall's Qg3 - Gruppe AF
    //test_positions[4] = new_test_pos_state("", "", "", "", "", "", "");

    if (argc >= 2) {
        if (strcmp(argv[1], "-s") == 0) {
            int test_number = atoi(argv[2]);
            if (test_number > TEST_POS_COUNT || test_position(test_number) != 0) {
                //test failed
                return 1;
            }
        } else if (strcmp(argv[1], "-benchmark") == 0) {
            for (int i = 0; i < TEST_POS_COUNT; i++) {
                benchmark_position(i);
            }
        }
    }

    //clean up
    for (int i = 0; i < TEST_POS_COUNT; i++) {
        if (test_positions[i] != NULL) {
            if (test_positions[i]->state != NULL)
                free(test_positions[i]->state);
            free(test_positions[i]);
        }
    }

    return 0; //pass test for now
}

test_position_state* new_test_pos_state(char* fen, char* pawn_moves, char* bishop_moves, char* knight_moves, char* rook_moves, char* queen_moves, char* king_moves) {
    //save all moves in an array to iterate trough them
    char* moves[] = {pawn_moves, bishop_moves, knight_moves, rook_moves, queen_moves, king_moves};
    test_position_state* state = calloc(1, sizeof(test_position_state));

    //generate board state
    state->state = fen_to_board(fen);

    int j;
    for (int i = 0; i < 6; i++) {
        //copy moves to a writable array
        char* curr_moves_str = calloc(strlen(moves[i]) + 1, sizeof(char));
        strcpy(curr_moves_str, moves[i]);
        char delim[] = ",";
        char* split_state = curr_moves_str;

        //extract every move from string
        char* curr_move = strtok_r(split_state, delim, &split_state);
        for (j = 0; curr_move != NULL && j < 32; j++) {
            //save move in state
            state->moves[i][j] = fen_pos_to_uint(curr_move, 0);
            curr_move = strtok_r(split_state, delim, &split_state);
        }

        //set how many moves are possible
        state->move_counts[i] = j;

        //free writable array
        free(curr_moves_str);
    }
    return state;
}

int test_position(int index) {
    int move_count_all = 0;
    int move_count_expected = 0;
    int failed = 0;
    //loop through figures
    for (int piece = 0; piece < 6; piece++) {
        //combine with curr player
        __uint64_t combined_board;
        if (test_positions[index]->state->player == 1) {
            combined_board = test_positions[index]->state->pieces[piece] & test_positions[index]->state->white;
        } else {
            combined_board = test_positions[index]->state->pieces[piece] & test_positions[index]->state->black;
        }
        //loop through bits
        __uint64_t moves = 0;
        int move_count = 0;
        int contains = 0;
        char tmp_move_str[255] = ""; 
        char* tmp_str_pointer = tmp_move_str;
        for (int bit = 0; bit < 64; bit++) {
            if ((combined_board >> bit) & 1) {
                //TODO: run get_moves
                moves = get_all_possible_moves(test_positions[index]->state, piece, (__uint64_t)1 << bit);
                //moves = 0b0000000011011111001000000000000000000000000000001111111100000000; //CHANGE ME! hardcoded value for testing
                for (int m_bit = 0; m_bit < 64; m_bit++) {
                    if ((moves >> m_bit) & 1) {
                        //compare moves
                        tmp_str_pointer = tmp_move_str + strlen(tmp_move_str);
                        sprintf(tmp_str_pointer, "%s,", uint_pos_to_fen((__uint64_t)1 << m_bit));
                        for (int comp_i = 0; comp_i < test_positions[index]->move_counts[piece]; comp_i++) {
                            if (test_positions[index]->moves[piece][comp_i] == (__uint64_t)1 << m_bit) {
                                contains += 1;
                                break;
                            }
                        }
                        move_count++;
                    }
                }
            }
        }
        if (contains != test_positions[index]->move_counts[piece] || move_count != test_positions[index]->move_counts[piece]) {
            if (!failed) {
                //print board only once
                print_board(test_positions[index]->state);
            }
            failed = 1;
            if (move_count != test_positions[index]->move_counts[piece]) {
                printf("piece: %c expected %d moves, but got %d instead\n", PIECE_CHARS[piece], test_positions[index]->move_counts[piece], move_count);
            }
            printf("piece: %c expected ", PIECE_CHARS[piece]);
            for (int i = 0; i < test_positions[index]->move_counts[piece]; i++) {
                printf("%s,", uint_pos_to_fen(test_positions[index]->moves[piece][i]));
            }
            printf(" but got %s instead\n", tmp_move_str);
        }
        move_count_all += move_count;
        move_count_expected += test_positions[index]->move_counts[piece];
    }
    if (failed) {
        //test failed
        return -1;
    }
    //test passed
    return 0;
}

void benchmark_position(int index) {
    int iteration_count = 1000;
    __uint64_t start_time = get_micros();

    for (int i = 0; i < iteration_count; i++) {
        //CHANGE ME: dummy implementation. call actual pick move function in game.c if it's ready
        //loop through figures
        get_best_known_move(test_positions[index]->state, 1);
/*         int moves = 0;
        if (moves == 0) {
            //get rid of compiler warning
        }
        for (int piece = 0; piece < 6; piece++) {
            //combine with curr player
            __uint64_t combined_board;
            if (test_positions[index]->state->player == 1) {
                combined_board = test_positions[index]->state->pieces[piece] & test_positions[index]->state->white;
            } else {
                combined_board = test_positions[index]->state->pieces[piece] & test_positions[index]->state->black;
            }
            //loop through bits
            __uint64_t moves = 0;
            for (int bit = 0; bit < 64; bit++) {
                if ((combined_board >> bit) & 1) {
                    //TODO: run get_moves
                    moves = get_possible_moves_in_state(test_positions[index]->state, piece, (__uint64_t)1 << bit);
                    //moves = 0b0000000011011111001000000000000000000000000000001111111100000000; //CHANGE ME! hardcoded value for testing
                    for (int m_bit = 0; m_bit < 64; m_bit++) {
                        if ((moves >> m_bit) & 1) {
                            moves++;
                        }
                    }
                }
            }
        } */
    } 

    __uint64_t end_time = get_micros();

    print_benchmark_result("position", (unsigned long)((end_time - start_time) / iteration_count));
}

void print_benchmark_result(char* name, unsigned long time) {
    printf("%s: took %lu micro seconds\n", name, time);
}