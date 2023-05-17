#include <stdlib.h>
#include <time.h>
#include <sys/time.h>

//TODO all game related stuff here (winning etc.)

enum game_phases {PRE_GAME = 0, OPENING, MIDGAME, WIN = 20, LOOSE = 21};
extern int game_phase;

__uint64_t get_micros();

