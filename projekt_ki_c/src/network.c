#include "network.h"

int network_socket = -1;

int network_connect(char* ip, char* port) {
    struct addrinfo hints, *res;
    int sock;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    getaddrinfo(ip, port, &hints, &res);

    //create socket
    if ((sock = socket(res->ai_family, res->ai_socktype, res->ai_protocol)) == -1)
        return -1;


    //connect to server
    if (connect(sock, res->ai_addr, res->ai_addrlen) == -1)
        return -1;

    network_socket = sock;
    return sock;
}

board_move* network_listen_for_move(board_state* state, int sock) {
    char move[1024]; //buffer to store the move.
    char buffer[1]; //recv buffer. read one char at a time

    memset(move, 0, 1024);

    //receive line
    //note: incredibly inefficient way to read lines. but we only receive max 5 chars per move, so should be fine
    do {
        recv(sock, buffer, 1, 0);
        printf("%c", buffer[0]);
        if (strlen(move) < 1023) { //prevent buffer overflow
            sprintf(move + strlen(move), "%c", buffer[0]);
        }
    } while (buffer[0] != '\n' && buffer[0] != '\n');

    if (strlen(move) < 3)
        return NULL; //line to short to be a move
    
    //convert to move and return
    board_move* b_move = fen_to_move(move, state);
    return b_move;
}