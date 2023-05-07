#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include "board.h"

extern int network_socket;

/*
 * Connect to the server
 */
int network_connect(char* ip, char* port);

/*
 * Listens for opponents move
 * sock: socket to listen on
 */
board_move* network_listen_for_move(board_state* state, int sock);
