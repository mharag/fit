// htab_init.c 
// Riesenie IJC-DU2, příklad 2), 17.4.2021 
// Autor: Miroslav Harag, FIT VUT 
// Prelozene: gcc 10.2.0 

#include <stdlib.h>
#include "htab.h"
#include "htab_private.h"

void htab_clear(htab_t * t){
    for(size_t i = 0; i < t->arr_size; i++){
        while(t->ptr_array[i] != NULL){
            // vymaze prvu polozku listu a ako zaciatok listu nastavi dalsiu polozku v poradi
            struct htab_item *temp = t->ptr_array[i];
            t->ptr_array[i] = t->ptr_array[i]->next;

            free((void*)temp->value.key);
            free(temp);
        }
    }

    t->size = 0;
}
