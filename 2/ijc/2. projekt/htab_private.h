// htab_private.h 
// Riesenie IJC-DU2, příklad 2), 18.4.2021 
// Autor: Miroslav Harag, FIT VUT 
// Prelozene: gcc 10.2.0 

#ifndef _HTAB_PRIVATE_H
#define _HTAB_PRIVATE_H

//typedef struct record_s record;
struct htab_item {
    htab_pair_t value;
    struct htab_item *next;
};

struct htab {
    size_t size;
    size_t arr_size;
    struct htab_item **ptr_array;
};

#endif
