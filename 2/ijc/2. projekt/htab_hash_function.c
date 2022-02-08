// htab_init.c 
// Riesenie IJC-DU2, příklad 2), 17.4.2021 
// Autor: Miroslav Harag, FIT VUT 
// Prelozene: gcc 10.2.0 

#include <stdint.h>

#include "htab.h"

size_t htab_hash_function(const char *str) {
    uint32_t h=0;     // musí mít 32 bitů
    const unsigned char *p;
    for(p=(const unsigned char*)str; *p!='\0'; p++)
        h = 65599*h + *p;
    return h;
}
