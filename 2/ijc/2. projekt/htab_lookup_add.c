// htab_init.c 
// Riesenie IJC-DU2, příklad 2), 17.4.2021 
// Autor: Miroslav Harag, FIT VUT 
// Prelozene: gcc 10.2.0 

#include <stdlib.h>
#include "htab.h"
#include "htab_private.h"

htab_pair_t * htab_lookup_add(htab_t * t, htab_key_t key){
    size_t hash = htab_hash_function(key);
    size_t index = hash % t->arr_size;
    
    // zisti ci uz polozka v tabulke existuje
    struct htab_item **pos = &(t->ptr_array[index]);
    while(*pos != NULL){
        if(!strcmp((*pos)->value.key,key))
            return &(*pos)->value;
        pos = &((*pos)->next);
    }

    // vytvori novy polozku
    struct htab_item *new_record;
    new_record = malloc(sizeof(struct htab_item)); 
    if(new_record == NULL)
        return NULL;

    // prekopiruje key
    int length = strlen(key);
    new_record->value.key = malloc(length+1);
    strcpy((char*)new_record->value.key, key);

    new_record->value.value = 0;
    new_record->next = NULL;

    // pos ukazuje na koniec prislusneho listu
    *pos = new_record;

    t->size++;
      
    return &(new_record->value);
}
