// htab_init.c 
// Riesenie IJC-DU2, příklad 2), 17.4.2021 
// Autor: Miroslav Harag, FIT VUT 
// Prelozene: gcc 10.2.0 

#include "htab.h"
#include "htab_private.h"

size_t htab_bucket_count(const htab_t * t){
    return t->arr_size;
}
