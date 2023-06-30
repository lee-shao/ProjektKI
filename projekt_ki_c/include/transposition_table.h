#include "uthash.h"
#include "board.h"

typedef struct _transposition_table {
    __uint64_t key;
    //board_move* move;
    __int32_t score;
    __uint16_t depth;
    __uint8_t type;
    __int8_t player;
    //UT_hash_handle hh;
} transposition_table;

extern transposition_table **trans_t;

extern int hash_sets;
extern int hash_deletes;
extern int hash_collisions;
extern int hash_hits;

void tt_set_position(transposition_table **tt, const __uint64_t key, int player, int score, __uint16_t depth, __uint8_t type);

transposition_table* tt_get_position(transposition_table **tt, const __uint64_t key, int player);

void tt_delete_position(transposition_table **tt, const __uint64_t key);

void tt_clear(transposition_table **tt);