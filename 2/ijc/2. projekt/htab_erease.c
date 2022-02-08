// htab_init.c 
// Riesenie IJC-DU2, příklad 2), 17.4.2021 
// Autor: Miroslav Harag, FIT VUT 
// Prelozene: gcc 10.2.0 

#include <stdlib.h>
#include "htab.h"
#include "htab_private.h"

bool htab_erase(htab_t * t, htab_key_t key){
    size_t hash = htab_hash_function(key);
    size_t index = hash % t->arr_size;

    struct htab_item **pos = &(t->ptr_array[index]);
    while(*pos != NULL){
        if(!strcmp((*pos)->value.key,key)){
            struct htab_item *temp = *pos;
            *pos = (*pos)->next;

            free((void*)temp->value.key);
            free(temp);

            t->size--;
            return true;
        }
        pos = &((*pos)->next);
    }
    return false;
}
