#include "game.h"

int game_phase = PRE_GAME;

__uint64_t get_micros() {
    struct timeval time;
    gettimeofday(&time,NULL);
    return time.tv_sec*(__uint64_t)1000000+time.tv_usec;
}