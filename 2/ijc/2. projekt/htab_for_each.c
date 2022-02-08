// htab_init.c 
// Riesenie IJC-DU2, příklad 2), 17.4.2021 
// Autor: Miroslav Harag, FIT VUT 
// Prelozene: gcc 10.2.0 

#include "htab.h"
#include "htab_private.h"

void htab_for_each(const htab_t * t, void (*f)(htab_pair_t *data)){
    // pre kazdy "bucket"
    for(size_t i = 0; i < t->arr_size; i++){
        // pre kazdy record v bucket
        struct htab_item *ptr = t->ptr_array[i];
        while(ptr != NULL){
            f(&(ptr->value));
            ptr = ptr->next;
        }
    }
}
