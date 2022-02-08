// htab_init.c 
// Riesenie IJC-DU2, příklad 2), 17.4.2021 
// Autor: Miroslav Harag, FIT VUT 
// Prelozene: gcc 10.2.0 

#include "htab.h"
#include "htab_private.h"

htab_pair_t * htab_find(htab_t * t, htab_key_t key){
    size_t hash = htab_hash_function(key);
    size_t index = hash % t->arr_size;

    struct htab_item **pos = &(t->ptr_array[index]);
    while(*pos != NULL){
        if(!strcmp((*pos)->value.key,key))
            return &(*pos)->value;
        pos = &((*pos)->next);
    }
    return NULL;
}
