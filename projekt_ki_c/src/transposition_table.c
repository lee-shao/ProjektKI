#include "transposition_table.h"
#include <stdio.h>

#define HASH_TABLE_SIZE 0x800000
//8000000

transposition_table **trans_t = NULL;

transposition_table t_table[HASH_TABLE_SIZE];

int hash_sets = 0;
int hash_deletes = 0;
int hash_collisions = 0;
int hash_hits = 0;

void tt_set_position(transposition_table **tt, const __uint64_t key, int player, int score, __uint16_t depth, __uint8_t type) {
/*     transposition_table *old = tt_get_position(tt, key);

    if (old != NULL) {
        if (depth > old->depth) {
            tt_delete_position(tt, key);
        } else {
            return;
        }
    }

    //board_move *new_move = calloc(1, sizeof(board_move));
    //new_move->from = move->from;
    //new_move->to = move->to;
    //new_move->piece = move->piece;
    //new_move->score = move->score;

    transposition_table *new = calloc(1, sizeof(transposition_table));
    new->depth = depth;
    new->score = score;
    //new->move = new_move;
    new->key = key;

    hash_sets++;


    HASH_ADD_KEYPTR(hh, *tt, &key, sizeof(__uint64_t), new); */
    int index = key % HASH_TABLE_SIZE;
    transposition_table *t = &t_table[index];
    if (t->key != 0) {
        if (key != t->key) {
            hash_collisions++;
            //printf("COLLISION!\n");
        } else if (depth < t->depth) {
            return;
        }
    }
    hash_sets++;
    t->depth = depth;
    t->score = score;
    t->type = type;
    t->key = key;
    t->player = player;
}

transposition_table* tt_get_position(transposition_table **tt, const __uint64_t key, int player) {
    transposition_table *ret = NULL;

    //HASH_FIND(hh, *tt, &key, sizeof(__uint64_t), ret);

    int index = key % HASH_TABLE_SIZE;
    if (t_table[index].key == key) { // && t_table[index].player == player
        ret = &t_table[index];
    }

    return ret;
}

void tt_delete_position(transposition_table **tt, const __uint64_t key) {
/*     transposition_table *del = tt_get_position(tt, key);

    if (del != NULL) {
        HASH_DEL(*tt, del);
        hash_deletes++;
        free(del);
    } */

    int index = key % HASH_TABLE_SIZE;
    if (t_table[index].key == key) {
        t_table[index].key = 0;
    }

    //free(del->move);
}

void tt_clear(transposition_table **tt) {
/*     transposition_table *current;
    transposition_table *tmp;

    HASH_ITER(hh, *tt, current, tmp) {
        HASH_DEL(*tt, current);
        //printf("del: %llu\n", current->key);
        //free(current->move);
        free(current);
    } */

    for (int i = 0; i < HASH_TABLE_SIZE; i++) {
        t_table[i].key = 0;
    }
}