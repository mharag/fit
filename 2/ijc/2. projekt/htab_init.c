// htab_init.c
// Riesenie IJC-DU2, příklad 2), 17.4.2021
// Autor: Miroslav Harag, FIT VUT
// Prelozene: gcc 10.2.0

#include <stdio.h>
#include <stdlib.h>
#include "htab.h"
#include "htab_private.h"

htab_t *htab_init(size_t n){
    struct htab *table = NULL;
    table = malloc(sizeof(struct htab));
    if(table == NULL){
        fprintf(stderr, "ERROR: Nepodarilo sa alokovat dostatok priestoru pre htab!\n");
        return NULL;
    }
    
    table->size = 0;
    table->arr_size = n;

    table->ptr_array = malloc(table->arr_size*sizeof(struct htab_pair*));
    if(table->ptr_array == NULL){
        fprintf(stderr, "ERROR: Nepodarilo sa alokovat dostatok priestoru pre htab!\n");
        return NULL;
    }

    for(size_t i = 0; i < table->arr_size; i++){
        table->ptr_array[i] = NULL;
    }


    return table;
}
